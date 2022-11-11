//
// Created by zengjiale on 2022/4/18.
//
#include <bean/evaframe.h>
#include <bean/evasrc.h>
#include <bean/evasrcmap.h>
#include <util/texcoordsutil.h>
#include <util/textureloadutil.h>
#include <util/vertexutil.h>
#include <util/glfloatarray.h>
#include <bean/evaanimeconfig.h>
#include <engine/irender.h>
#include "mixshader.h"


#ifndef YYEVA_MIXRENDER_H
#define YYEVA_MIXRENDER_H

using namespace std;
class MixRender: public IRender {
public:
    MixRender();
    ~MixRender();

    void initRender();
    void renderFrame(int frameIndex);
    void clearFrame();
    void destroyRender();
    void setAnimeConfig(EvaAnimeConfig* config);
    void updateViewPort(int width, int height);
    GLuint getExternalTexture();
    void releaseTexture();
    void swapBuffers();

    void init(EvaSrcMap* evaSrcMap);
    void rendFrame(GLuint videoTextureId, EvaAnimeConfig* config, EvaFrame* frame, EvaSrc* src);
    void release(GLuint textureId);

private:
    float* genSrcCoordsArray(float* array, int fw, int fh, int sw, int sh, EvaSrc::FitType fitType);
    float* transColor(int color);
    float* transColor(string color);
    float ratio = 1.0f;
    int surfaceWidth = 0;
    int surfaceHeight = 0;

    MixShader* shader;
    GlFloatArray* vertexArray = new GlFloatArray();
    GlFloatArray* srcArray = new GlFloatArray();
    GlFloatArray* maskArray = new GlFloatArray();
    EvaAnimeConfig *config;
    int curFrameIndex;
};


#endif //YYEVA_MIXRENDER_H
