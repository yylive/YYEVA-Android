//
// Created by asus on 2022/10/24.
//

#include "framebufferrender.h"

FramebufferRender::FramebufferRender() {

}

FramebufferRender::~FramebufferRender() {
    destroyRender();
}

void FramebufferRender::initRender() {
    VERTEX_SHADER.clear();
    FRAGMENT_SHADER.clear();
    VERTEX_SHADER =
            "#version 300 es\n"
            "in vec4 vPosition;\n"
            "in vec4 vTexCoordinate;\n"
            "out vec2 v_TexCoordinate;\n"
            "\n"
            "void main() {\n"
            "v_TexCoordinate = vec2(vTexCoordinate.x, vTexCoordinate.y);\n"
            "gl_Position = vPosition;\n"
            "}";

    if (!filterType.empty()) {
        if (filterType == "hermite") {
            needWh = true;
            FRAGMENT_SHADER =
                    "#version 300 es\n"
                    "precision highp float;\n"
                    "in highp vec2 v_TexCoordinate;\n"
                    "uniform sampler2D uTexture;\n"
                    "out vec4 glFragColor;\n"

                    "uniform float w;\n"
                    "uniform float h;\n"
                    "float c_textureSize = 0.0;\n"

                    "float c_onePixel = 0.0;\n"
                    "float c_twoPixels = 0.0;\n"

                    "float c_x0 = -1.0;\n"
                    "float c_x1 =  0.0;\n"
                    "float c_x2 =  1.0;\n"
                    "float c_x3 =  2.0;\n"

                    //=======================================================================================
                    "vec3 CubicHermite (vec3 A, vec3 B, vec3 C, vec3 D, float t)\n"
                    "{\n"
                    "float t2 = t*t;\n"
                    "float t3 = t*t*t;\n"
                    "vec3 a = -A/2.0 + (3.0*B)/2.0 - (3.0*C)/2.0 + D/2.0;\n"
                    "vec3 b = A - (5.0*B)/2.0 + 2.0*C - D / 2.0;\n"
                    "vec3 c = -A/2.0 + C/2.0;\n"
                    "vec3 d = B;\n"

                    "return a*t3 + b*t2 + c*t + d;\n"
                    "}\n"

                    //=======================================================================================
                    "vec3 BicubicHermiteTextureSample (vec2 P)\n"
                    "{\n"
                    "vec2 pixel = P * c_textureSize + 0.5;\n"

                    "vec2 frac = fract(pixel);\n"
                    "pixel = floor(pixel) / c_textureSize - vec2(c_onePixel/2.0);\n"

                    "vec3 C00 = texture(uTexture, pixel + vec2(-c_onePixel ,-c_onePixel)).rgb;\n"
                    "vec3 C10 = texture(uTexture, pixel + vec2( 0.0        ,-c_onePixel)).rgb;\n"
                    "vec3 C20 = texture(uTexture, pixel + vec2( c_onePixel ,-c_onePixel)).rgb;\n"
                    "vec3 C30 = texture(uTexture, pixel + vec2( c_twoPixels,-c_onePixel)).rgb;\n"

                    "vec3 C01 = texture(uTexture, pixel + vec2(-c_onePixel , 0.0)).rgb;\n"
                    "vec3 C11 = texture(uTexture, pixel + vec2( 0.0        , 0.0)).rgb;\n"
                    "vec3 C21 = texture(uTexture, pixel + vec2( c_onePixel , 0.0)).rgb;\n"
                    "vec3 C31 = texture(uTexture, pixel + vec2( c_twoPixels, 0.0)).rgb;\n"

                    "vec3 C02 = texture(uTexture, pixel + vec2(-c_onePixel , c_onePixel)).rgb;\n"
                    "vec3 C12 = texture(uTexture, pixel + vec2( 0.0        , c_onePixel)).rgb;\n"
                    "vec3 C22 = texture(uTexture, pixel + vec2( c_onePixel , c_onePixel)).rgb;\n"
                    "vec3 C32 = texture(uTexture, pixel + vec2( c_twoPixels, c_onePixel)).rgb;\n"

                    "vec3 C03 = texture(uTexture, pixel + vec2(-c_onePixel , c_twoPixels)).rgb;\n"
                    "vec3 C13 = texture(uTexture, pixel + vec2( 0.0        , c_twoPixels)).rgb;\n"
                    "vec3 C23 = texture(uTexture, pixel + vec2( c_onePixel , c_twoPixels)).rgb;\n"
                    "vec3 C33 = texture(uTexture, pixel + vec2( c_twoPixels, c_twoPixels)).rgb;\n"

                    "vec3 CP0X = CubicHermite(C00, C10, C20, C30, frac.x);\n"
                    "vec3 CP1X = CubicHermite(C01, C11, C21, C31, frac.x);\n"
                    "vec3 CP2X = CubicHermite(C02, C12, C22, C32, frac.x);\n"
                    "vec3 CP3X = CubicHermite(C03, C13, C23, C33, frac.x);\n"

                    "return CubicHermite(CP0X, CP1X, CP2X, CP3X, frac.y);\n"
                    "}\n"

                    "void main() {\n"
                    "c_textureSize = sqrt(w*h);\n"
                    "c_onePixel = (1.0 / c_textureSize);\n"
                    "c_twoPixels = (2.0 / c_textureSize);\n"
                    "glFragColor = vec4(BicubicHermiteTextureSample(v_TexCoordinate), 1.0);\n"
                    "}";
        } else if (filterType == "lagrange") {
            needWh = true;
            FRAGMENT_SHADER =
                    "#version 300 es\n"
                    "precision highp float;\n"
                    "in highp vec2 v_TexCoordinate;\n"
                    "uniform sampler2D uTexture;\n"
                    "out vec4 glFragColor;\n"
                    "uniform float w;\n"
                    "uniform float h;\n"
                    "float c_textureSize = 0.0;\n"

                    "float c_onePixel = 0.0;\n"
                    "float c_twoPixels = 0.0;\n"
                    "float c_x0 = -1.0;\n"
                    "float c_x1 =  0.0;\n"
                    "float c_x2 =  1.0;\n"
                    "float c_x3 =  2.0;\n"

                    "vec3 CubicLagrange (vec3 A, vec3 B, vec3 C, vec3 D, float t) {\n"
                    "return\n"
                    "    A * (\n"
                    "            (t - c_x1) / (c_x0 - c_x1) *\n"
                    "            (t - c_x2) / (c_x0 - c_x2) *\n"
                    "            (t - c_x3) / (c_x0 - c_x3)) +\n"
                    "    B * (\n"
                    "            (t - c_x0) / (c_x1 - c_x0) *\n"
                    "            (t - c_x2) / (c_x1 - c_x2) *\n"
                    "            (t - c_x3) / (c_x1 - c_x3)) +\n"
                    "    C * (\n"
                    "            (t - c_x0) / (c_x2 - c_x0) *\n"
                    "            (t - c_x1) / (c_x2 - c_x1) *\n"
                    "            (t - c_x3) / (c_x2 - c_x3)) +\n"
                    "    D * (\n"
                    "            (t - c_x0) / (c_x3 - c_x0) *\n"
                    "            (t - c_x1) / (c_x3 - c_x1) *\n"
                    "            (t - c_x2) / (c_x3 - c_x2)\n"
                    "    );\n"
                    "}\n"

                    //=======================================================================================
                    "vec3 BicubicLagrangeTextureSample (vec2 P) {\n"
                    "vec2 pixel = P * c_textureSize + 0.5;\n"

                    "vec2 frac = fract(pixel);\n"
                    "pixel = floor(pixel) / c_textureSize - vec2(c_onePixel/2.0);\n"

                    "vec3 C00 = texture(uTexture, pixel + vec2(-c_onePixel ,-c_onePixel)).rgb;\n"
                    "vec3 C10 = texture(uTexture, pixel + vec2( 0.0        ,-c_onePixel)).rgb;\n"
                    "vec3 C20 = texture(uTexture, pixel + vec2( c_onePixel ,-c_onePixel)).rgb;\n"
                    "vec3 C30 = texture(uTexture, pixel + vec2( c_twoPixels,-c_onePixel)).rgb;\n"

                    "vec3 C01 = texture(uTexture, pixel + vec2(-c_onePixel , 0.0)).rgb;\n"
                    "vec3 C11 = texture(uTexture, pixel + vec2( 0.0        , 0.0)).rgb;\n"
                    "vec3 C21 = texture(uTexture, pixel + vec2( c_onePixel , 0.0)).rgb;\n"
                    "vec3 C31 = texture(uTexture, pixel + vec2( c_twoPixels, 0.0)).rgb;\n"

                    "vec3 C02 = texture(uTexture, pixel + vec2(-c_onePixel , c_onePixel)).rgb;\n"
                    "vec3 C12 = texture(uTexture, pixel + vec2( 0.0        , c_onePixel)).rgb;\n"
                    "vec3 C22 = texture(uTexture, pixel + vec2( c_onePixel , c_onePixel)).rgb;\n"
                    "vec3 C32 = texture(uTexture, pixel + vec2( c_twoPixels, c_onePixel)).rgb;\n"

                    "vec3 C03 = texture(uTexture, pixel + vec2(-c_onePixel , c_twoPixels)).rgb;\n"
                    "vec3 C13 = texture(uTexture, pixel + vec2( 0.0        , c_twoPixels)).rgb;\n"
                    "vec3 C23 = texture(uTexture, pixel + vec2( c_onePixel , c_twoPixels)).rgb;\n"
                    "vec3 C33 = texture(uTexture, pixel + vec2( c_twoPixels, c_twoPixels)).rgb;\n"

                    "vec3 CP0X = CubicLagrange(C00, C10, C20, C30, frac.x);\n"
                    "vec3 CP1X = CubicLagrange(C01, C11, C21, C31, frac.x);\n"
                    "vec3 CP2X = CubicLagrange(C02, C12, C22, C32, frac.x);\n"
                    "vec3 CP3X = CubicLagrange(C03, C13, C23, C33, frac.x);\n"

                    "return CubicLagrange(CP0X, CP1X, CP2X, CP3X, frac.y);\n"
                    "}\n"


                    "void main() {\n"
                    "c_textureSize = sqrt(w*h);\n"
                    "c_onePixel = (1.0 / c_textureSize);\n"
                    "c_twoPixels = (2.0 / c_textureSize);\n"
                    "glFragColor = vec4(BicubicLagrangeTextureSample(v_TexCoordinate), 1.0);\n"
                    "}";
        }
    } else {
        FRAGMENT_SHADER = "#version 300 es\n"
                          //                      "#extension GL_OES_EGL_image_external_essl3 : require"
                          "precision mediump float;\n"
                          "uniform sampler2D uTexture;\n"
                          "in vec2 v_TexCoordinate;\n"
                          "out vec4 vFragColor;\n"
                          "\n"
                          "void main () {\n"
                          "vFragColor = texture(uTexture, v_TexCoordinate);\n"
                          //                      "vFragColor = vec4(1.0,0.2,0.5,1.0);\n"
                          "}";
    }

    shaderProgram = ShaderUtil::createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    uTextureLocation = glGetUniformLocation(shaderProgram, "uTexture");
    positionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    textureLocation = glGetAttribLocation(shaderProgram, "vTexCoordinate");
    if (needWh) {
        wLocation = glGetUniformLocation(shaderProgram,"w");
        hLocation = glGetUniformLocation(shaderProgram,"h");
    }

    glGenTextures(1, &fboTextureId);
    glBindTexture(GL_TEXTURE_2D, fboTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    vertexArray = make_shared<GlFloatArray>();
    rgbaArray = make_shared<GlFloatArray>();
}

void FramebufferRender::renderFrame(int frameIndex) {
    if (fboTextureId != -1) {
        glUseProgram(shaderProgram);
        vertexArray->setVertexAttribPointer(positionLocation);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboTextureId);
        //加载纹理
        glUniform1i(uTextureLocation, 0);
        rgbaArray->setVertexAttribPointer(textureLocation);

        if (needWh) {
            glUniform1f(wLocation, surfaceWidth);
            glUniform1f(hLocation, surfaceHeight);
        }

        glEnable(GL_BLEND);
        //基于alpha通道的半透明混合函数
        //void glBlendFuncSeparate(GLenum srcRGB,
        //     GLenum dstRGB,
        //     GLenum srcAlpha,
        //     GLenum dstAlpha);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //关闭混合
        glDisable(GL_BLEND);
    }
}

void FramebufferRender::clearFrame() {

}

void FramebufferRender::destroyRender() {
    VERTEX_SHADER.clear();
    FRAGMENT_SHADER.clear();
}

void FramebufferRender::setAnimeConfig(EvaAnimeConfig *config) {
    if (config != nullptr && config->params != nullptr && config->params->useFbo) {
        filterType = config->params->filterType;
        initRender();
        glGenFramebuffers(1, &fboId);

        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        glBindTexture(GL_TEXTURE_2D, fboTextureId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, fboTextureId, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceWidth, surfaceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            ELOGE("initFrameBuffer framebuffer init fail");
            return;
        }
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
        vertexArray->setArray(VertexUtil::createFlip(config->width, config->height,
                                                     new PointRect(0, 0, config->width,
                                                                   config->height),
                                                     vertexArray->array));

        float* rgba = TexCoordsUtil::create(config->width, config->height, new PointRect(0, 0, config->width, config->height), rgbaArray->array);
        rgbaArray->setArray(rgba);
    }
}

void FramebufferRender::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceWidth = width;
    surfaceHeight = height;
}

GLuint FramebufferRender::getExternalTexture() {
    return fboTextureId;
}

void FramebufferRender::releaseTexture() {
    glDeleteTextures(1, &fboTextureId);
    glDeleteFramebuffers(1, &fboId);
}

void FramebufferRender::swapBuffers() {

}

void FramebufferRender::enableFramebuffer() {
    if (fboId != -1) {
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    }
}

void FramebufferRender::disableFrameBuffer() {
    if (fboId != -1) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
