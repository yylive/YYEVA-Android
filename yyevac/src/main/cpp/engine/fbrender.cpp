//
// Created by Cangwang on 2024/3/6.
//

#include "fbrender.h"

yyeva::FbRender::FbRender() {
    initRender();
}

yyeva::FbRender::~FbRender() {
}

void yyeva::FbRender::setTextureId(GLuint textureId) {
    this->textureId = textureId;
    ELOGV("setTextureId %d", textureId);
}

void yyeva::FbRender::initRender() {
    char VERTEX_SHADER[] = R"(#version 310 es
        in vec4 vPosition;
        in vec4 vTexCoordinate;
        out vec2 v_TexCoordinate;
        void main() {
            v_TexCoordinate = vec2(vTexCoordinate.x, vTexCoordinate.y);
            gl_Position = vPosition;
        }
    )";
    char FRAGMENT_SHADER[] = R"(#version 310 es
        precision mediump float;
        uniform sampler2D texture;
        in vec2 v_TexCoordinate;
        out vec4 fragColor;

        void main () {
            fragColor = texture(texture, v_TexCoordinate);
        }
    )";
    shaderProgram = ShaderUtil::createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    uTextureLocation = glGetUniformLocation(shaderProgram, "texture");
    positionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    textureLocation = glGetAttribLocation(shaderProgram, "vTexCoordinate");
}

void yyeva::FbRender::renderFrame() {
//    ELOGV("renderFrame %d", textureId);
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void yyeva::FbRender::clearFrame() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void yyeva::FbRender::destroyRender() {
    releaseTexture();
}

GLuint yyeva::FbRender::getExternalTexture() {
    return textureId;
}

void yyeva::FbRender::releaseTexture() {
    glDeleteTextures(1,&textureId);
    glDeleteProgram(shaderProgram);
}

void yyeva::FbRender::swapBuffers() {

}

void yyeva::FbRender::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
    ELOGV("width: %d, height: %d", width, height);
}

void yyeva::FbRender::draw() {
    if (textureId != -1) {
        glUseProgram(shaderProgram);
//        vertexArray->setVertexAttribPointer(positionLocation);
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, textureVertices);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        //加载纹理
        glUniform1i(uTextureLocation, 0);
//        rgbaArray->setVertexAttribPointer(textureLocation);
        glEnableVertexAttribArray(textureLocation);
        glVertexAttribPointer(textureLocation, 2, GL_FLOAT, GL_FALSE, 0, texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
}
