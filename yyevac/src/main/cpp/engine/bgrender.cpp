//
// Created by zengjiale on 2022/10/11.
//

#include "bgrender.h"

BgRender::BgRender(): vertexArray(new GlFloatArray()), rgbaArray(new GlFloatArray()),textureId(0) {
    initRender();
}

BgRender::~BgRender() {
    vertexArray = nullptr;
    rgbaArray = nullptr;
}

void BgRender::initRender() {
    char VERTEX_SHADER[] =
            "#version 310 es\n"
            "in vec4 vPosition;\n"
            "in vec4 vTexCoordinate;\n"
            "out vec2 v_TexCoordinate;\n"
            "\n"
            "void main() {\n"
            "v_TexCoordinate = vec2(vTexCoordinate.x, vTexCoordinate.y);\n"
            "gl_Position = vPosition;\n"
            "}";

    char FRAGMENT_SHADER[] = "#version 310 es\n"
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
    shaderProgram = ShaderUtil::createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    uTextureLocation = glGetUniformLocation(shaderProgram, "uTexture");
    positionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    textureLocation = glGetAttribLocation(shaderProgram, "vTexCoordinate");
}

void BgRender::renderFrame(int frameIndex) {
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void BgRender::clearFrame() {
    glClearColor(0, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT);
//    glClearColor(1.0, 0, 0.5, 1.0);
}

void BgRender::destroyRender() {
    releaseTexture();
}

void BgRender::setAnimeConfig(EvaAnimeConfig *config) {
    if (config != nullptr && config->bgTextureId != -1) {
        textureId = config->bgTextureId;
    }
    float ratio;
    if (config->params->scaleType == 3) {
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
    } else if (config->params->scaleType == 2) {
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


//    vertexArray->setArray(VertexUtil::create(config->width, config->height, new PointRect(0, 0, config->width, config->height), vertexArray->array));
    float* rgba = TexCoordsUtil::create(config->width, config->height, new PointRect(0, 0, config->width, config->height), rgbaArray->array);
    rgbaArray->setArray(rgba);

    GLfloat vbo[16] = {
            vertexArray->array[0], vertexArray->array[1],
            rgba[0], rgba[1],
            vertexArray->array[2], vertexArray->array[3],
            rgba[2], rgba[3],
            vertexArray->array[4], vertexArray->array[5],
            rgba[4], rgba[5],
            vertexArray->array[6], vertexArray->array[7],
            rgba[6], rgba[7]
    };

    //创建vbo
    glGenBuffers(2, vboIds);
    //绑定定点数组到显存
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
//    checkGLError("glBindBuffer");
    glBufferData(GL_ARRAY_BUFFER, sizeof(vbo), vbo, GL_STATIC_DRAW);
//    checkGLError("glBufferData");
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
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat) , (const void *)0);
    glEnableVertexAttribArray(positionLocation);
//        rgbaArray->setVertexAttribPointer(textureLocation);
    glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat) , (const void *) (2*sizeof(GLfloat)));
    glEnableVertexAttribArray(textureLocation);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    glBindVertexArray(GL_NONE);
}

GLuint BgRender::getExternalTexture() {
    return textureId;
}

void BgRender::releaseTexture() {
    glDeleteTextures(1, &textureId);
}

void BgRender::swapBuffers() {

}

void BgRender::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
}

void BgRender::draw() {
    if (textureId != -1) {
//        glUseProgram(shaderProgram);
//        vertexArray->setVertexAttribPointer(positionLocation);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, textureId);
//        //加载纹理
//        glUniform1i(uTextureLocation, 0);
//        rgbaArray->setVertexAttribPointer(textureLocation);
//
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

//        glUseProgram(shaderProgram);
//        //使用vbo绘制
//        glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
////        vertexArray->setVertexAttribPointer(positionLocation);
//        glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat) , (const void *)0);
//        checkGLError("glVertexAttribPointer");
////        glEnableVertexAttribArray(positionLocation);
//
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, textureId);
//        //加载纹理
//        glUniform1i(uTextureLocation, 0);
////        rgbaArray->setVertexAttribPointer(textureLocation);
//        glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat) , (const void *) (2*sizeof(GLfloat)));
//        checkGLError("glVertexAttribPointer");
//        glEnableVertexAttribArray(textureLocation);
//        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
//
//        glEnable(GL_BLEND);
//        //基于alpha通道的半透明混合函数
//        //void glBlendFuncSeparate(GLenum srcRGB,
//        //     GLenum dstRGB,
//        //     GLenum srcAlpha,
//        //     GLenum dstAlpha);
//        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
////        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
////        //使用ebo绘制
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
//        glDrawElements(GL_TRIANGLE_STRIP,4, GL_UNSIGNED_SHORT, (const void *)0);
//        //关闭混合
//        glDisable(GL_BLEND);

        glUseProgram(shaderProgram);
        glBindVertexArray(vaoId);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        //加载纹理
        glUniform1i(uTextureLocation, 0);
        glDrawElements(GL_TRIANGLE_STRIP,4, GL_UNSIGNED_SHORT, (const void *)0);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glBindVertexArray(GL_NONE);
    }
}
