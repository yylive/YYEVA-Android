//
// Created by zengjiale on 2022/4/15.
//
#include "src/main/cpp/egl/glbase.h"
#include "src/main/cpp/egl/eglcore.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
//#include <GLES3/gl3.h>
#include <android/native_window.h>


class EvaEngine: public GLBase {
public:
    EvaEngine(ANativeWindow *window);
    ~EvaEngine();
    int create();
    void draw();
    void stop();

private:
    ANativeWindow * mWindow;
    GLuint mTextureId;
    GLint  mTextureLoc;
    GLint mMatrixLoc;
    GLfloat mMatrix[16];
    EGLCore *mEGLCore;
};
