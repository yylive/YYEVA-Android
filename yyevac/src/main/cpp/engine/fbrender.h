//
// Created by Cangwang on 2024/3/6.
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

#define LOG_TAG "FbRender"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)

using namespace std;
namespace yyeva {
    class FbRender : public IRender {
    public:
        FbRender();
        ~FbRender();
        void initRender();
        void renderFrame();
        void clearFrame();
        void destroyRender();
        void setAnimeConfig(shared_ptr<EvaAnimeConfig> config) {};
        GLuint getExternalTexture();
        void releaseTexture();
        void swapBuffers();
        void updateViewPort(int width, int height);
        void draw();
        void setHasBg(bool hasBg){};
        void setBlendMode(int blendMode) {};
        void setTextureId(GLuint textureId);
    private:
//        GlFloatArray *vertexArray = new GlFloatArray();
//        GlFloatArray *rgbaArray = new GlFloatArray();
        GLfloat textureVertices[8] = {
                -1.0f, -1.0f, // bot left
                -1.0f, 1.0f, // top left
                1.0f, -1.0f, // bot right
                1.0f, 1.0f, // top right
        };
        GLfloat texture[8] = {
                0.0f, 0.0f, // bot left
                0.0f, 1.0f, // top left
                1.0f, 0.0f, // bot right
                1.0f, 1.0f, // top right
        };
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
