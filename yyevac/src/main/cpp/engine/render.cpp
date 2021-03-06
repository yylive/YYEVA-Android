//
// Created by zengjiale on 2022/4/18.
//

#include "render.h"

#define LOG_TAG "Render"
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

Render::Render(ANativeWindow *window) : eglCore(new EGLCore()){
    eglCore->start(window);
    initRender();
}

Render::~Render() {
    vertexArray = nullptr;
    alphaArray = nullptr;
    rgbArray = nullptr;
    eglCore = nullptr;
}

void Render::initRender() {
    char VERTEX_SHADER[] = "attribute vec4 vPosition;\n"
                           "attribute vec4 vTexCoordinateAlpha;\n"
                           "attribute vec4 vTexCoordinateRgb;\n"
                           "varying vec2 v_TexCoordinateAlpha;\n"
                           "varying vec2 v_TexCoordinateRgb;\n"
                           "\n"
                           "void main() {\n"
                           "    v_TexCoordinateAlpha = vec2(vTexCoordinateAlpha.x, vTexCoordinateAlpha.y);\n"
                           "    v_TexCoordinateRgb = vec2(vTexCoordinateRgb.x, vTexCoordinateRgb.y);\n"
                           "    gl_Position = vPosition;\n"
                           "}";

    char FRAGMENT_SHADER[] = "#extension GL_OES_EGL_image_external : require\n"
                             "precision mediump float;\n"
                             "uniform samplerExternalOES texture;\n"
                             "varying vec2 v_TexCoordinateAlpha;\n"
                             "varying vec2 v_TexCoordinateRgb;\n"
                             "\n"
                             "void main () {\n"
                             "    vec4 alphaColor = texture2D(texture, v_TexCoordinateAlpha);\n"
                             "    vec4 rgbColor = texture2D(texture, v_TexCoordinateRgb);\n"
                             "    gl_FragColor = vec4(rgbColor.r, rgbColor.g, rgbColor.b, alphaColor.r);\n"
//                             "    gl_FragColor = vec4(1.0,0.2,0.5,1.0);\n"
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

void Render::renderFrame() {
    glClearColor(0, 0, 0, 0);
//    glClearColor(1.0, 0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void Render::clearFrame() {
    glClearColor(0, 0, 0, 0);
//    glClearColor(1.0, 0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    eglCore->swapBuffer();
}

void Render::destroyRender() {
    releaseTexture();
    eglCore->release();
}

void Render::setAnimeConfig(EvaAnimeConfig* config) {
    vertexArray->setArray(VertexUtil::create(config->width, config->height, new PointRect(0, 0, config->width, config->height), vertexArray->array));
    float* alpha = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->alphaPointRect, alphaArray->array);
    float* rgb = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->rgbPointRect, rgbArray->array);
    alphaArray->setArray(alpha);
    rgbArray->setArray(rgb);
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
    eglCore->swapBuffer();
}

void Render::draw() {
    if (textureId != -1) {
        glUseProgram(shaderProgram);
        vertexArray->setVertexAttribPointer(aPositionLocation);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
        //????????????
        glUniform1i(uTextureLocation, 0);

        alphaArray->setVertexAttribPointer(aTextureAlphaLocation);
        rgbArray->setVertexAttribPointer(aTextureRgbLocation);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
