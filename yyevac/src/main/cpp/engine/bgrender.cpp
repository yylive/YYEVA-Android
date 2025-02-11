//
// Created by zengjiale on 2022/10/11.
//

#include "bgrender.h"

yyeva::BgRender::BgRender(): vertexArray(make_shared<GlFloatArray>()), rgbaArray(make_shared<GlFloatArray>()) {
    initRender();
}

yyeva::BgRender::~BgRender() {
}

void yyeva::BgRender::setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo) {
    textureId = TextureLoadUtil::loadTexture(bitmap, bitmapInfo);
}

void yyeva::BgRender::initRender() {
    char VERTEX_SHADER[] = R"(in vec4 vPosition;
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

void yyeva::BgRender::renderFrame() {
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void yyeva::BgRender::clearFrame() {
    glClearColor(0, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT);
}

void yyeva::BgRender::destroyRender() {
    releaseTexture();
}

void yyeva::BgRender::setAnimeConfig(shared_ptr<EvaAnimeConfig> config) {
    vertexArray->setArray(VertexUtil::create(config->width, config->height,make_shared<PointRect>(0, 0, config->width, config->height), vertexArray->array));
    float* rgba = TexCoordsUtil::create(config->width, config->height, make_shared<PointRect>(0, 0, config->width, config->height), rgbaArray->array);
    rgbaArray->setArray(rgba);
}

GLuint yyeva::BgRender::getExternalTexture() {
    return textureId;
}

void yyeva::BgRender::releaseTexture() {
    glDeleteTextures(1, &textureId);
    glDeleteProgram(shaderProgram);
}

void yyeva::BgRender::swapBuffers() {

}

void yyeva::BgRender::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
}

void yyeva::BgRender::draw() {
    if (textureId != -1) {
        glUseProgram(shaderProgram);
        vertexArray->setVertexAttribPointer(positionLocation);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        //加载纹理
        glUniform1i(uTextureLocation, 0);
        rgbaArray->setVertexAttribPointer(textureLocation);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
