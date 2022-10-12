//
// Created by zengjiale on 2022/10/11.
//

#include <android/log.h>
#include <engine/irender.h>
#include <egl/eglcore.h>
#include <util/shaderutil.h>
#include <util/glfloatarray.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <util/vertexutil.h>
#include <util/texcoordsutil.h>
#include <util/textureloadutil.h>
#include <android/bitmap.h>

#ifndef YYEVA_BGRENDER_H
#define YYEVA_BGRENDER_H

#define LOG_TAG "BgRender"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)


using namespace std;
class BgRender: public IRender {
public:
    BgRender();
    ~BgRender();
    void initRender();
    void setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo);
    void renderFrame();
    void clearFrame();
    void destroyRender();
    void setAnimeConfig(EvaAnimeConfig* config);
    GLuint getExternalTexture();
    void releaseTexture();
    void swapBuffers();
    void updateViewPort(int width, int height);
    void draw();

private:
    GlFloatArray *vertexArray = new GlFloatArray();
    GlFloatArray *rgbaArray = new GlFloatArray();

    GLuint shaderProgram;
    //shader
    GLuint textureId;
    //顶点位置
    GLint uTextureLocation;
    //纹理位置
    GLint positionLocation;
    //纹理位置
    GLint textureLocation;

    int surfaceWidth = 0;
    int surfaceHeight = 0;
    bool surfaceSizeChanged = false;
};


#endif //YYEVA_BGRENDER_H
