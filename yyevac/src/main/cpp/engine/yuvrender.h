//
// Created by zengjiale on 2022/4/15.
//
#include "irender.h"
#include "src/main/cpp/egl/eglcore.h"
#include "src/main/cpp/util/shaderutil.h"
#include "src/main/cpp/util/glfloatarray.h"
#include <EGL/eglext.h>
//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "src/main/cpp/util/vertexutil.h"
#include "src/main/cpp/util/texcoordsutil.h"


class YUVRender: public IRender {
public:
    YUVRender();
    ~YUVRender();
    void initRender();
    void renderFrame();
    void clearFrame();
    void destroyRender();
    void setAnimeConfig(EvaAnimeConfig* config);
    GLuint getExternalTexture();
    void releaseTexture();
    void swapBuffers();
    void setHasBg(bool hasBg) {}
    void setYUVData(int width, int height, char *y, char *u, char *v);

private:
    GlFloatArray *vertexArray = new GlFloatArray();
    GlFloatArray *alphaArray = new GlFloatArray();
    GlFloatArray *rgbArray = new GlFloatArray();

    GLuint shaderProgram;
    //顶点位置
    GLint avPosition;
    //rgb纹理位置
    GLint rgbPosition;
    //alpha纹理位置
    GLint alphaPosition;
    //shader  yuv变量
    GLint samplerY = 0;
    GLint samplerU = 0;
    GLint samplerV = 0;
    GLuint textureId[3];
    GLint convertMatrixUniform;
    GLint convertOffsetUniform;
    //YUV数据
    int widthYUV = 0;
    int heightYUV = 0;
    char* y;
    char* u;
    char* v;

    // 像素数据向GPU传输时默认以4字节对齐
    int unpackAlign = 4;

    GLfloat YUV_OFFSET[3] = {
            0, -0.501960814, -0.501960814
    };


//    GLint YUV_OFFSET[] = {
//            0,1,2
//    };

    GLfloat YUV_MATRIX[9] = {
            1.0f, 1.0f, 1.0f,
            0.0f, -0.3441f, 1.772f,
            1.402f, -0.7141f, 0.0f
    };

    void draw();
};
