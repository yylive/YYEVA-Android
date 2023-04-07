//
// Created by zengjiale on 2022/10/11.
//

#include "mp4render.h"

Mp4Render::Mp4Render() {
    initRender();
}

Mp4Render::~Mp4Render() {
    vertexArray = nullptr;
    rgbaArray = nullptr;
}

void Mp4Render::setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo) {
//    textureId = TextureLoadUtil::loadTexture(bitmap, bitmapInfo);
}


void Mp4Render::initRender() {
    char VERTEX_SHADER[] = "attribute vec4 vPosition;\n"
                           "attribute vec4 vTexCoordinate;\n"
                           "varying vec2 v_TexCoordinate;\n"
                           "\n"
                           "void main() {\n"
                           "    v_TexCoordinate = vec2(vTexCoordinate.x, vTexCoordinate.y);\n"
                           "    gl_Position = vPosition;\n"
                           "}";

    char FRAGMENT_SHADER[] =
                             "#extension GL_OES_EGL_image_external : require\n"
                             "precision mediump float;\n"
                             "uniform samplerExternalOES texture;\n"
                             "varying vec2 v_TexCoordinate;\n"
                             "\n"
                             "void main () {\n"
                             "    gl_FragColor = texture2D(texture, v_TexCoordinate);\n"
//                             "    gl_FragColor = vec4(1.0,0.2,0.5,1.0);\n"
                             "}";
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

void Mp4Render::renderFrame() {
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void Mp4Render::clearFrame() {
    glClearColor(0, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT);
}

void Mp4Render::destroyRender() {
    releaseTexture();
}

void Mp4Render::setAnimeConfig(EvaAnimeConfig *config) {
    vertexArray->setArray(VertexUtil::create(config->width, config->height, new PointRect(0, 0, config->width, config->height), vertexArray->array));
    float* rgba = TexCoordsUtil::create(config->width, config->height, new PointRect(0, 0, config->width, config->height), rgbaArray->array);
    rgbaArray->setArray(rgba);
}

GLuint Mp4Render::getExternalTexture() {
    return textureId;
}

void Mp4Render::releaseTexture() {
    glDeleteTextures(1, &textureId);
}

void Mp4Render::swapBuffers() {

}

void Mp4Render::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
}

void Mp4Render::draw() {
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
