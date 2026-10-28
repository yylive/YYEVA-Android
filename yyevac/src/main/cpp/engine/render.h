//
// Created by zengjiale on 2022/4/18.
//
#pragma once

#include "irender.h"
#include "src/main/cpp/egl/eglcore.h"
#include "src/main/cpp/util/shaderutil.h"
#include "src/main/cpp/util/glfloatarray.h"
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include "src/main/cpp/util/vertexutil.h"
#include "src/main/cpp/util/texcoordsutil.h"

namespace yyeva {
    class Render : public IRender {
    public:
        Render();

        ~Render();

        void initRender();

        void renderFrame();

        void clearFrame();

        void destroyRender();

        void setAnimeConfig(shared_ptr<EvaAnimeConfig> config);

        GLuint getExternalTexture();

        void releaseTexture();

        void swapBuffers();

        void updateViewPort(int width, int height);

        void setExternalTextureTransform(const float* matrix);

        void setHasBg(bool hasBg);

        void setBlendMode(int blendMode) override;

        void draw();

    private:
        shared_ptr<GlFloatArray> vertexArray;
        shared_ptr<GlFloatArray> alphaArray;
        shared_ptr<GlFloatArray> rgbArray;

        GLuint shaderProgram;
        //shader
        GLuint textureId;
        //顶点位置
        GLint uTextureLocation;
        //rgb纹理位置
        GLint aPositionLocation;
        //alpha纹理位置
        GLint aTextureAlphaLocation;
        GLint aTextureRgbLocation;
        GLint uTextureTransformLocation;
        GLfloat textureTransform[16];

        int surfaceWidth = 0;
        int surfaceHeight = 0;
        bool surfaceSizeChanged = false;
        bool hasBg = false;
        int blendMode = 1;
    };
}
