//
// Created by zengjiale on 2022/10/11.
//
#pragma once

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

#define LOG_TAG "Mp4Render"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)


using namespace std;
namespace yyeva {
    class Mp4Render : public IRender {
    public:
        Mp4Render();

        ~Mp4Render();

        void initRender();

        void setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo);

        void renderFrame();

        void clearFrame();

        void destroyRender();

        void setAnimeConfig(shared_ptr<EvaAnimeConfig> config);

        GLuint getExternalTexture();

        void releaseTexture();

        void swapBuffers();

        void updateViewPort(int width, int height);

        void setHasBg(bool hasBg) {};

        void draw();

    private:
        shared_ptr<GlFloatArray> vertexArray;
        shared_ptr<GlFloatArray> rgbaArray;

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
}