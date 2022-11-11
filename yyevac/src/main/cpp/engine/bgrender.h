//
// Created by zengjiale on 2022/10/11.
//

#include <android/log.h>
#include <engine/irender.h>
#include <util/shaderutil.h>
#include <util/glfloatarray.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
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
    void renderFrame(int frameIndex);
    void clearFrame();
    void destroyRender();
    void setAnimeConfig(EvaAnimeConfig* config);
    GLuint getExternalTexture();
    void releaseTexture();
    void swapBuffers();
    void updateViewPort(int width, int height);
    void draw();

private:
    GlFloatArray *vertexArray;
    GlFloatArray *rgbaArray;

    GLuint shaderProgram;
    //shader
    GLuint textureId = -1;
    //顶点位置
    GLint uTextureLocation;
    //纹理位置
    GLint positionLocation;
    //纹理位置
    GLint textureLocation;

    GLuint vboIds[2];
    GLuint vaoId;

    int surfaceWidth = 0;
    int surfaceHeight = 0;
    bool surfaceSizeChanged = false;
};


#endif //YYEVA_BGRENDER_H
