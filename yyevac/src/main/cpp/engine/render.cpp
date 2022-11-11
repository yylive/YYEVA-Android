//
// Created by zengjiale on 2022/4/18.
//

#include "render.h"

#define LOG_TAG "Render"
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

Render::Render(){
    initRender();
}

Render::~Render() {
    vertexArray = nullptr;
    alphaArray = nullptr;
    rgbArray = nullptr;
}

void Render::initRender() {
    ELOGV("initRender");
    char VERTEX_SHADER[] =
            "#version 300 es\n"
            "in vec4 vPosition;\n"
            "in vec4 vTexCoordinateAlpha;\n"
            "in vec4 vTexCoordinateRgb;\n"
            "out vec2 v_TexCoordinateAlpha;\n"
            "out vec2 v_TexCoordinateRgb;\n"
            "\n"
            "void main() {\n"
            "   v_TexCoordinateAlpha = vec2(vTexCoordinateAlpha.x, vTexCoordinateAlpha.y);\n"
            "   v_TexCoordinateRgb = vec2(vTexCoordinateRgb.x, vTexCoordinateRgb.y);\n"
            "   gl_Position = vPosition;\n"
            "}";

    char FRAGMENT_SHADER[] = "#version 300 es\n"
                             "#extension GL_OES_EGL_image_external_essl3 : require\n"
                             "precision mediump float;\n"
                             "uniform samplerExternalOES texture;\n"
                             "in vec2 v_TexCoordinateAlpha;\n"
                             "in vec2 v_TexCoordinateRgb;\n"
                             "out vec4 gl_FragColor;\n"
                             "\n"
                             "void main () {\n"
                             "    vec4 alphaColor = texture(texture, v_TexCoordinateAlpha);\n"
                             "    vec4 rgbColor = texture(texture, v_TexCoordinateRgb);\n"
                             "    gl_FragColor = vec4(rgbColor.r, rgbColor.g, rgbColor.b, alphaColor.r);\n"
                             "}";

    shaderProgram = ShaderUtil::createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    uTextureLocation = glGetUniformLocation(shaderProgram, "texture");
    aPositionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    aTextureAlphaLocation = glGetAttribLocation(shaderProgram, "vTexCoordinateAlpha");
    aTextureRgbLocation = glGetAttribLocation(shaderProgram, "vTexCoordinateRgb");

    glGenTextures(1,&textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Render::renderFrame(int frameIndex) {
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void Render::clearFrame() {

}

void Render::destroyRender() {
    releaseTexture();
}

void Render::setAnimeConfig(EvaAnimeConfig* config) {
    int scaleType = config->params->scaleType;
    if (scaleType == 3) {
        float sizeRatio = 1.0f;
        if (config->videoWidth < config->videoHeight) {
            ratio = (float) config->videoHeight / config->videoWidth;
            if (config->videoHeight < surfaceHeight) {
                sizeRatio = (float) config->videoHeight / surfaceHeight;
            } else {
                sizeRatio = (float) surfaceHeight / config->videoHeight;
            }
        } else {
            ratio = (float) config->videoWidth / config->videoHeight;
            if (config->videoWidth < surfaceWidth) {
                sizeRatio = (float) config->videoWidth / surfaceWidth;
            } else {
                sizeRatio = (float) surfaceWidth / config->videoWidth;
            }
        }
        vertexArray->setArray(VertexUtil::create(config->width, config->height,
                                                 new PointRect(0, 0, config->width,
                                                               config->height),
                                                 vertexArray->array,
                                                 ratio, sizeRatio));
    } else if (scaleType == 2) {
        vertexArray->setArray(VertexUtil::create(config->width, config->height,
                                                 new PointRect(0, 0, config->width,
                                                               config->height),
                                                 vertexArray->array));
    } else {  //加入ratio参数，计算图像正常比例
        if (config->videoWidth < config->videoHeight) {
            ratio = (float) config->videoHeight / config->videoWidth;
        } else {
            ratio = (float) config->videoWidth / config->videoHeight;
        }
        vertexArray->setArray(VertexUtil::create(config->width, config->height,
                                                 new PointRect(0, 0, config->width,
                                                               config->height),
                                                 vertexArray->array,
                                                 ratio));
    }
    //alpha顶点归一化
    float* alpha = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->alphaPointRect, alphaArray->array);
    //rgb区域顶点归一化
    float* rgb = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->rgbPointRect, rgbArray->array);
    alphaArray->setArray(alpha);
    rgbArray->setArray(rgb);

    GLfloat vbo[24] = {
            vertexArray->array[0], vertexArray->array[1],
            alpha[0], alpha[1],
            rgb[0], rgb[1],
            vertexArray->array[2], vertexArray->array[3],
            alpha[2], alpha[3],
            rgb[2], rgb[3],
            vertexArray->array[4], vertexArray->array[5],
            alpha[4], alpha[5],
            rgb[4], rgb[5],
            vertexArray->array[6], vertexArray->array[7],
            alpha[6], alpha[7],
            rgb[6], rgb[7]
    };

    //创建vbo
    glGenBuffers(2, vboIds);
    //绑定定点数组到显存
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vbo), vbo, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    GLushort indices[4] = { 0, 1, 2, 3};

    //拷贝图元素索引数据到显存
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    //使用vbo绘制
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
//        vertexArray->setVertexAttribPointer(positionLocation);
    glVertexAttribPointer(aPositionLocation, 2, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat) , (const void *)0);
    glEnableVertexAttribArray(aPositionLocation);

//        rgbaArray->setVertexAttribPointer(textureLocation);
    glVertexAttribPointer(aTextureAlphaLocation, 2, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat) , (const void *) (2*sizeof(GLfloat)));
    glEnableVertexAttribArray(aTextureAlphaLocation);
    glVertexAttribPointer(aTextureRgbLocation, 2, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat) , (const void *) (4*sizeof(GLfloat)));
    glEnableVertexAttribArray(aTextureRgbLocation);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    glBindVertexArray(GL_NONE);
}

GLuint Render::getExternalTexture() {
    return textureId;
}

void Render::releaseTexture() {
    glDeleteTextures(1, &textureId);
}

void Render::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
}

void Render::swapBuffers() {
}

void Render::draw() {
    if (textureId != -1) {
//        glUseProgram(shaderProgram);
//        vertexArray->setVertexAttribPointer(aPositionLocation);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
//        //加载纹理
//        glUniform1i(uTextureLocation, 0);
//
//        alphaArray->setVertexAttribPointer(aTextureAlphaLocation);
//        rgbArray->setVertexAttribPointer(aTextureRgbLocation);
//        //启动混合
//        glEnable(GL_BLEND);
//        //基于alpha通道的半透明混合函数
//        //void glBlendFuncSeparate(GLenum srcRGB,
//        //     GLenum dstRGB,
//        //     GLenum srcAlpha,
//        //     GLenum dstAlpha);
//        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//        //关闭混合
//        glDisable(GL_BLEND);

        glUseProgram(shaderProgram);
        vertexArray->setVertexAttribPointer(aPositionLocation);
        glBindVertexArray(vaoId);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
        //加载纹理
        glUniform1i(uTextureLocation, 0);

        alphaArray->setVertexAttribPointer(aTextureAlphaLocation);
        rgbArray->setVertexAttribPointer(aTextureRgbLocation);
        //启动混合
        glEnable(GL_BLEND);
        //基于alpha通道的半透明混合函数
        //void glBlendFuncSeparate(GLenum srcRGB,
        //     GLenum dstRGB,
        //     GLenum srcAlpha,
        //     GLenum dstAlpha);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLE_STRIP,4, GL_UNSIGNED_SHORT, (const void *)0);
        //关闭混合
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, GL_NONE);
        glBindVertexArray(GL_NONE);
    }
}

