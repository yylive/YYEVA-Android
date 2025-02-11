//
// Created by zengjiale on 2022/10/11.
//

#include "mp4render.h"

yyeva::Mp4Render::Mp4Render(): vertexArray(make_shared<GlFloatArray>()), rgbaArray(make_shared<GlFloatArray>()) {
    initRender();
}

yyeva::Mp4Render::~Mp4Render() {
}

void yyeva::Mp4Render::setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo) {
//    textureId = TextureLoadUtil::loadTexture(bitmap, bitmapInfo);
}


void yyeva::Mp4Render::initRender() {
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
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        uniform samplerExternalOES texture;
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

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void yyeva::Mp4Render::renderFrame() {
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void yyeva::Mp4Render::clearFrame() {
    glClearColor(0, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT);
}

void yyeva::Mp4Render::destroyRender() {
    releaseTexture();
}

void yyeva::Mp4Render::setAnimeConfig(shared_ptr<EvaAnimeConfig> config) {
    vertexArray->setArray(VertexUtil::create(config->width, config->height, make_shared<PointRect>(0, 0, config->width, config->height), vertexArray->array));
    float* rgba = TexCoordsUtil::create(config->width, config->height, make_shared<PointRect>(0, 0, config->width, config->height), rgbaArray->array);
    rgbaArray->setArray(rgba);
}

GLuint yyeva::Mp4Render::getExternalTexture() {
    return textureId;
}

void yyeva::Mp4Render::releaseTexture() {
    glDeleteTextures(1, &textureId);
    glDeleteProgram(shaderProgram);
}

void yyeva::Mp4Render::swapBuffers() {

}

void yyeva::Mp4Render::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
}

void yyeva::Mp4Render::draw() {
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
