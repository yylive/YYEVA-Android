//
// Created by zengjiale on 2022/4/18.
//
#include <bean/evaframe.h>
#include <bean/evasrc.h>
#include "bean/evasrcmap.h"
#include "util/texcoordsutil.h"
#include "util/textureloadutil.h"
#include "util/vertexutil.h"
#include <util/glfloatarray.h>
#include "src/main/cpp/bean/evaanimeconfig.h"
#include "mixshader.h"


#ifndef YYEVA_EVAMIXRENDER_H
#define YYEVA_EVAMIXRENDER_H

using namespace std;
class EvaMixRender {
public:
    MixShader* shader;
    GlFloatArray* vertexArray = new GlFloatArray();
    GlFloatArray* srcArray = new GlFloatArray();
    GlFloatArray* maskArray = new GlFloatArray();

    EvaMixRender();
    ~EvaMixRender();
    void init(EvaSrcMap* evaSrcMap);
    void rendFrame(GLuint videoTextureId, EvaAnimeConfig* config, EvaFrame* frame, EvaSrc* src);
    void release(GLuint textureId);

private:
    float* genSrcCoordsArray(float* array, int fw, int fh, int sw, int sh, EvaSrc::FitType fitType);
    float* transColor(int color);
    float* transColor(string color);
};


#endif //YYEVA_EVAMIXRENDER_H
