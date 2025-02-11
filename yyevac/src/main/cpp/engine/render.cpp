//
// Created by zengjiale on 2022/4/18.
//

#include "render.h"

#define LOG_TAG "Render"
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)

yyeva::Render::Render(): vertexArray(make_shared<GlFloatArray>()), alphaArray(make_shared<GlFloatArray>()), rgbArray(make_shared<GlFloatArray>()) {
    initRender();
}

yyeva::Render::~Render() {
}

void yyeva::Render::initRender() {
    char VERTEX_SHADER[] = R"(#version 310 es
            in vec4 vPosition;
            in vec4 vTexCoordinateAlpha;
            in vec4 vTexCoordinateRgb;
            out vec2 v_TexCoordinateAlpha;
            out vec2 v_TexCoordinateRgb;
            void main() {
                v_TexCoordinateAlpha = vec2(vTexCoordinateAlpha.x, vTexCoordinateAlpha.y);
                v_TexCoordinateRgb = vec2(vTexCoordinateRgb.x, vTexCoordinateRgb.y);
                gl_Position = vPosition;
            }
    )";

    char FRAGMENT_SHADER[] = R"(#version 310 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        uniform samplerExternalOES u_Texture;
        in vec2 v_TexCoordinateAlpha;
        in vec2 v_TexCoordinateRgb;
        out vec4 fragColor;

        void main () {
            vec4 alphaColor = texture(u_Texture, v_TexCoordinateAlpha);
            vec4 rgbColor = texture(u_Texture, v_TexCoordinateRgb);
            fragColor = vec4(rgbColor.r, rgbColor.g, rgbColor.b, alphaColor.r);
        }
    )";

    shaderProgram = ShaderUtil::createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    uTextureLocation = glGetUniformLocation(shaderProgram, "u_Texture");
    aPositionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    aTextureAlphaLocation = glGetAttribLocation(shaderProgram, "vTexCoordinateAlpha");
    aTextureRgbLocation = glGetAttribLocation(shaderProgram, "vTexCoordinateRgb");

    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void yyeva::Render::renderFrame() {
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void yyeva::Render::clearFrame() {
    glClearColor(0, 0, 0, 0);
//    glClearColor(1.0, 0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void yyeva::Render::destroyRender() {
    releaseTexture();
}

void yyeva::Render::setAnimeConfig(shared_ptr<EvaAnimeConfig> config) {
    vertexArray->setArray(VertexUtil::create(config->width, config->height, make_shared<PointRect>(0, 0, config->width, config->height), vertexArray->array));
    float* alpha = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->alphaPointRect, alphaArray->array);
    float* rgb = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->rgbPointRect, rgbArray->array);
    alphaArray->setArray(alpha);
    rgbArray->setArray(rgb);
}

GLuint yyeva::Render::getExternalTexture() {
    return textureId;
}

void yyeva::Render::releaseTexture() {
    glDeleteTextures(1, &textureId);
    glDeleteProgram(shaderProgram);
}

void yyeva::Render::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
}

void yyeva::Render::swapBuffers() {

}

void yyeva::Render::draw() {
    if (textureId != -1) {
        glUseProgram(shaderProgram);
        vertexArray->setVertexAttribPointer(aPositionLocation);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
        //加载纹理
        glUniform1i(uTextureLocation, 0);

        alphaArray->setVertexAttribPointer(aTextureAlphaLocation);
        rgbArray->setVertexAttribPointer(aTextureRgbLocation);
        //启动混合
        glEnable(GL_BLEND);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //基于alpha通道的半透明混合函数
        //void glBlendFuncSeparate(GLenum srcRGB,
        //     GLenum dstRGB,
        //     GLenum srcAlpha,
        //     GLenum dstAlpha);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                            GL_ONE_MINUS_SRC_ALPHA);
//        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_BLEND);
    }
}

//如果有背景需要开启混合
void yyeva::Render::setHasBg(bool hasBg) {
    this->hasBg = hasBg;
}
