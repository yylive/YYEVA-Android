//
// Created by zengjiale on 2022/4/15.
//

#include "yuvrender.h"

#define LOG_TAG "YUVRender"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

std::string VERTEX_SHADER = std::string("attribute vec4 v_Position;\n") +
                          "attribute vec2 vTexCoordinateAlpha;\n" +
                          "attribute vec2 vTexCoordinateRgb;\n" +
                          "varying vec2 v_TexCoordinateAlpha;\n" +
                          "varying vec2 v_TexCoordinateRgb;\n" +
                          "\n" +
                          "void main() {\n" +
                          "    v_TexCoordinateAlpha = vTexCoordinateAlpha;\n" +
                          "    v_TexCoordinateRgb = vTexCoordinateRgb;\n" +
                          "    gl_Position = v_Position;\n" +
                          "}";

std::string FRAGMENT_SHADER = std::string("precision mediump float;\n") +
                            "uniform sampler2D sampler_y;\n" +
                            "uniform sampler2D sampler_u;\n" +
                            "uniform sampler2D sampler_v;\n" +
                            "varying vec2 v_TexCoordinateAlpha;\n" +
                            "varying vec2 v_TexCoordinateRgb;\n" +
                            "uniform mat3 convertMatrix;\n" +
                            "uniform vec3 offset;\n" +
                            "\n" +
                            "void main() {\n" +
                            "   highp vec3 yuvColorAlpha;\n" +
                            "   highp vec3 yuvColorRGB;\n" +
                            "   highp vec3 rgbColorAlpha;\n" +
                            "   highp vec3 rgbColorRGB;\n" +
                            "   yuvColorAlpha.x = texture2D(sampler_y,v_TexCoordinateAlpha).r;\n" +
                            "   yuvColorRGB.x = texture2D(sampler_y,v_TexCoordinateRgb).r;\n" +
                            "   yuvColorAlpha.y = texture2D(sampler_u,v_TexCoordinateAlpha).r;\n" +
                            "   yuvColorAlpha.z = texture2D(sampler_v,v_TexCoordinateAlpha).r;\n" +
                            "   yuvColorRGB.y = texture2D(sampler_u,v_TexCoordinateRgb).r;\n" +
                            "   yuvColorRGB.z = texture2D(sampler_v,v_TexCoordinateRgb).r;\n" +
                            "   yuvColorAlpha += offset;\n" +
                            "   yuvColorRGB += offset;\n" +
                            "   rgbColorAlpha = convertMatrix * yuvColorAlpha; \n" +
                            "   rgbColorRGB = convertMatrix * yuvColorRGB; \n" +
                            "   gl_FragColor=vec4(rgbColorRGB, rgbColorAlpha.r);\n" +
                            "}";


YUVRender::YUVRender() {
    initRender();
}

YUVRender::~YUVRender() {
    y = nullptr;
    u = nullptr;
    v = nullptr;
    vertexArray = nullptr;
    alphaArray = nullptr;
    rgbArray = nullptr;
}

void YUVRender::initRender() {
    shaderProgram = ShaderUtil::createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    //获取定点坐标字段
    avPosition = glGetAttribLocation(shaderProgram, "v_Position");
    //获取纹理坐标字段
    rgbPosition = glGetAttribLocation(shaderProgram, "vTexCoordinateRgb");
    alphaPosition = glGetAttribLocation(shaderProgram, "vTexCoordinateAlpha");

    //获取yuv字段
    samplerY = glGetUniformLocation(shaderProgram, "sampler_y");
    samplerU = glGetUniformLocation(shaderProgram, "sampler_u");
    samplerV = glGetUniformLocation(shaderProgram, "sampler_v");
    convertMatrixUniform = glGetUniformLocation(shaderProgram, "convertMatrix");
    convertOffsetUniform = glGetUniformLocation(shaderProgram, "offset");
    int count = sizeof(textureId)/sizeof(int);
    glGenTextures(count, textureId);

    for (int i = 0; i < count; ++i) {
        glBindTexture(GL_TEXTURE_2D, textureId[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void YUVRender::renderFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    draw();
}

void YUVRender::draw() {
    if (widthYUV > 0 && heightYUV > 0  && y != nullptr & u != nullptr && v != nullptr) {
        glUseProgram(shaderProgram);
        vertexArray->setVertexAttribPointer(avPosition);
        alphaArray->setVertexAttribPointer(alphaPosition);
        rgbArray->setVertexAttribPointer(rgbPosition);

        //设置对齐
        glPixelStorei(GL_UNPACK_ALIGNMENT, unpackAlign);

        //激活纹理0来绑定y数据
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthYUV,heightYUV, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);

        //激活纹理1来绑定u数据
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureId[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthYUV/2,heightYUV/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);

        //激活纹理0来绑定y数据
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureId[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widthYUV/2,heightYUV/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);

        //给fragment_shader里面yuv变量设置 0, 1 识别纹理x
        glUniform1f(samplerY, 0);
        glUniform1f(samplerU, 1);
        glUniform1f(samplerV, 2);

        glUniform3fv(convertOffsetUniform, 1, YUV_OFFSET);
        glUniformMatrix3fv(convertMatrixUniform, 1, false, YUV_MATRIX);

        //绘制
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        y = nullptr;
        u = nullptr;
        v = nullptr;
        glDisableVertexAttribArray(avPosition);
        glDisableVertexAttribArray(rgbPosition);
        glDisableVertexAttribArray(alphaPosition);
    }
}

void YUVRender::clearFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();
}

void YUVRender::destroyRender() {
    releaseTexture();
}

void YUVRender::setAnimeConfig(EvaAnimeConfig* config) {
//    vertexArray->setArray();
    vertexArray->setArray(VertexUtil::create(config->width, config->height, new PointRect(0, 0, config->width, config->height), vertexArray->array));
    float* alpha = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->alphaPointRect, alphaArray->array);
    float* rgb = TexCoordsUtil::create(config->videoWidth, config->videoHeight, config->rgbPointRect, rgbArray->array);
    alphaArray->setArray(alpha);
    rgbArray->setArray(rgb);
}


GLuint YUVRender::getExternalTexture() {
    return textureId[0];
}

void YUVRender::releaseTexture() {
    glDeleteTextures(sizeof(textureId)/sizeof(GLuint), textureId);
}

void YUVRender::swapBuffers() {

}

void YUVRender::setYUVData(int width, int height, char *y, char *u, char *v) {
    widthYUV = width;
    heightYUV = height;
    this->y = y;
    this->u = u;
    this->v = v;

    //当帧数的u或者v分享量的宽度不能被4争取，用默认的4字节对齐会导致存取最后一行越界，所以在想GPU传输数据前指定对齐方式
    if ((widthYUV/2) % 4 != 0) {
        if ((widthYUV/2) %2 == 0) {
            unpackAlign = 2;
        } else {
            unpackAlign = 1;
        }
    }
}
