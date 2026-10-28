//
// Created by Cangwang on 2024/2/27.
//

#include "recordrender.h"

yyeva::RecordRender::RecordRender(): vertexArray(make_shared<GlFloatArray>()), rgbaArray(make_shared<GlFloatArray>())  {
    initRender();
}

yyeva::RecordRender::~RecordRender() {

}

void yyeva::RecordRender::initRender() {
    char VERTEX_SHADER[] = R"(#version 310 es
        in vec4 vPosition;
        in vec4 vTexCoordinate;
        out vec2 v_TexCoordinate;
        void main() {
            v_TexCoordinate = vec2(vTexCoordinate.x, vTexCoordinate.y);
            gl_Position = vPosition;
        }
    )";
//    char FRAGMENT_SHADER[] =
//            "#extension GL_OES_EGL_image_external : require\n"
//            "precision mediump float;\n"
//            "uniform samplerExternalOES texture;\n"
//            "in vec2 v_TexCoordinate;\n"
//            "\n"
//            "void main () {\n"
//            "    gl_FragColor = texture(texture, v_TexCoordinate);\n"
//            //                             "    gl_FragColor = vec4(1.0,0.2,0.5,1.0);\n"
//            "}";
    char FRAGMENT_SHADER[] = R"(#version 310 es
        precision mediump float;
        uniform sampler2D texture;
        in vec2 v_TexCoordinate;
        out vec4 fragColor;

        void main () {
            fragColor = texture(texture, vec2(v_TexCoordinate.x, 1.0-v_TexCoordinate.y));
        }
    )";
    shaderProgram = ShaderUtil::createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    uTextureLocation = glGetUniformLocation(shaderProgram, "texture");
    positionLocation = glGetAttribLocation(shaderProgram, "vPosition");
    textureLocation = glGetAttribLocation(shaderProgram, "vTexCoordinate");

}

void yyeva::RecordRender::renderFrame() {
    if (surfaceSizeChanged && surfaceWidth > 0 && surfaceHeight > 0) {
        surfaceSizeChanged = false;
        glViewport(0, 0, surfaceWidth, surfaceHeight);
    }
    draw();
}

void yyeva::RecordRender::clearFrame() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void yyeva::RecordRender::destroyRender() {
    releaseTexture();
}

void yyeva::RecordRender::setAnimeConfig(EvaAnimeConfig *config) {
    vertexArray->setArray(VertexUtil::create(config->width, config->height, make_shared<PointRect>(0, 0, config->width, config->height), vertexArray->array));
    float* rgba = TexCoordsUtil::create(config->width, config->height, make_shared<PointRect>(0, 0, config->width, config->height), rgbaArray->array);
    rgbaArray->setArray(rgba);
}

GLuint yyeva::RecordRender::getExternalTexture() {
    return textureId;
}

void yyeva::RecordRender::releaseTexture() {
    if (textureId != -1) {
        glDeleteTextures(1, &textureId);
    }
    glDeleteProgram(shaderProgram);
}

void yyeva::RecordRender::swapBuffers() {

}

void yyeva::RecordRender::updateViewPort(int width, int height) {
    if (width <= 0 || height <= 0) return;
    surfaceSizeChanged = true;
    surfaceWidth = width;
    surfaceHeight = height;
    ELOGV("width: %d, height: %d", width, height);
}

void yyeva::RecordRender::setHasBg(bool hasBg) {

}

void yyeva::RecordRender::draw() {
    if (textureId != -1) {
        glUseProgram(shaderProgram);
        vertexArray->setVertexAttribPointer(positionLocation);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        //加载纹理
        glUniform1i(uTextureLocation, 0);
        rgbaArray->setVertexAttribPointer(textureLocation);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
}

void yyeva::RecordRender::setTextureId(GLuint textureId) {
    ELOGV("setTextureId %d", textureId);
    this->textureId = textureId;
}
