//
// Created by Cangwang on 2024/2/27.
//
#pragma once

#include <engine/irender.h>
#include <egl/eglcore.h>
#include <util/shaderutil.h>
#include <util/glfloatarray.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <util/vertexutil.h>
#include <util/texcoordsutil.h>
#include <util/textureloadutil.h>
#include <android/bitmap.h>

#define LOG_TAG "RecordRender"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)

namespace yyeva {
    class RecordRender : public IRender {
    public:
        RecordRender();
        ~RecordRender();
        void initRender();
        void renderFrame();
        void clearFrame();
        void destroyRender();
        void setAnimeConfig(EvaAnimeConfig* config);
        GLuint getExternalTexture();
        void releaseTexture();
        void swapBuffers();
        void updateViewPort(int width, int height);
        void setHasBg(bool hasBg);
        void setBlendMode(int blendMode) {};
        void draw();
        void setTextureId(GLuint textureId);
    private:
        shared_ptr<GlFloatArray> vertexArray;
        shared_ptr<GlFloatArray> rgbaArray;

        GLuint shaderProgram;
        //shader
        GLuint textureId;
        //顶点位置Tex
        GLint uTextureLocation;
        //纹理位置
        GLint positionLocation;
        //纹理位置
        GLint textureLocation;

        int surfaceWidth = 0;
        int surfaceHeight = 0;
        bool surfaceSizeChanged = false;
    };
}
