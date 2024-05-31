//
// Created by zengjiale on 2022/4/18.
//
#pragma once
#include <bean/evaframe.h>
#include <bean/evasrc.h>
#include "bean/evasrcmap.h"
#include "util/texcoordsutil.h"
#include "util/textureloadutil.h"
#include "util/vertexutil.h"
#include <util/glfloatarray.h>
#include "src/main/cpp/bean/evaanimeconfig.h"
#include "mixshader.h"

using namespace std;
namespace yyeva {
    class EvaMixRender {
    public:
        shared_ptr<MixShader> shader;
        GlFloatArray *vertexArray = new GlFloatArray();
        GlFloatArray *srcArray = new GlFloatArray();
        GlFloatArray *maskArray = new GlFloatArray();

        EvaMixRender();

        ~EvaMixRender();

        void init(shared_ptr<EvaSrcMap> evaSrcMap);

        void rendFrame(GLuint videoTextureId, shared_ptr<EvaAnimeConfig> config,
                       shared_ptr<EvaFrame> frame, shared_ptr<EvaSrc> src);

        void release(GLuint textureId);

    private:
        float *
        genSrcCoordsArray(float *array, float fw, float fh, float sw, float sh, EvaSrc::FitType fitType);

        float *transColor(int color);

        float *transColor(string color);
    };
}