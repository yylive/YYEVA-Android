//
// Created by zengjiale on 2022/4/18.
//
#pragma once

#include "irender.h"
#include "src/main/cpp/egl/eglcore.h"
#include "src/main/cpp/util/shaderutil.h"
#include "src/main/cpp/util/glfloatarray.h"
#include <GLES2/gl2.h>
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

        void setHasBg(bool hasBg);

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

        int surfaceWidth = 0;
        int surfaceHeight = 0;
        bool surfaceSizeChanged = false;
        bool hasBg = false;

//    string VERTEX_SHADER = string("attribute vec4 vPosition;\n") +
//                              "attribute vec4 vTexCoordinateAlpha;\n" +
//                              "attribute vec4 vTexCoordinateRgb;\n" +
//                              "varying vec2 v_TexCoordinateAlpha;\n" +
//                              "varying vec2 v_TexCoordinateRgb;\n" +
//                              "\n" +
//                              "void main() {\n" +
//                              "    v_TexCoordinateAlpha = vec2(vTexCoordinateAlpha.x, vTexCoordinateAlpha.y);\n" +
//                              "    v_TexCoordinateRgb = vec2(vTexCoordinateRgb.x, vTexCoordinateRgb.y);\n" +
//                              "    gl_Position = vPosition;\n" +
//                              "}";

//    char VERTEX_SHADER[] = "attribute vec4 vPosition;\n"
//                           "attribute vec4 vTexCoordinateAlpha;\n"
//                           "attribute vec4 vTexCoordinateRgb;\n"
//                           "varying vec2 v_TexCoordinateAlpha;\n"
//                           "varying vec2 v_TexCoordinateRgb;\n"
//                           "\n"
//                           "void main() {\n"
//                           "    v_TexCoordinateAlpha = vec2(vTexCoordinateAlpha.x, vTexCoordinateAlpha.y);\n"
//                           "    v_TexCoordinateRgb = vec2(vTexCoordinateRgb.x, vTexCoordinateRgb.y);\n"
//                           "    gl_Position = vPosition;\n"
//                           "}";


//    string FRAGMENT_SHADER = string("#extension GL_OES_EGL_image_external : require\n") +
//                                "precision mediump float;\n" +
//                                "uniform samplerExternalOES texture;\n" +
//                                "varying vec2 v_TexCoordinateAlpha;\n" +
//                                "varying vec2 v_TexCoordinateRgb;\n" +
//                                "\n" +
//                                "void main () {\n" +
//                                "    vec4 alphaColor = texture2D(texture, v_TexCoordinateAlpha);\n" +
//                                "    vec4 rgbColor = texture2D(texture, v_TexCoordinateRgb);\n" +
//                                "    gl_FragColor = vec4(rgbColor.r, rgbColor.g, rgbColor.b, alphaColor.r);\n" +
//                                "}";
    };
}