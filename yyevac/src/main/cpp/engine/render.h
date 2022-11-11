//
// Created by zengjiale on 2022/4/18.
//

#ifndef YYEVA_RENDER_H
#define YYEVA_RENDER_H

#include "irender.h"
#include "src/main/cpp/egl/eglcore.h"
#include "src/main/cpp/util/shaderutil.h"
#include "src/main/cpp/util/glfloatarray.h"
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include "src/main/cpp/util/vertexutil.h"
#include "src/main/cpp/util/texcoordsutil.h"

class Render: public IRender {
public:
    Render();
    ~Render();
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
    GlFloatArray *vertexArray = new GlFloatArray();
    GlFloatArray *alphaArray = new GlFloatArray();
    GlFloatArray *rgbArray = new GlFloatArray();

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
    float ratio;
    GLuint vboIds[2];
    GLuint vaoId;
};


#endif //YYEVA_RENDER_H
