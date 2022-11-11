#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android/native_window.h>

#ifndef YYEVA_EGLCORE_H
#define YYEVA_EGLCORE_H

/**
 * cangwang 2018.12.1
 */
class EGLCore {
public:
    EGLCore();

    ~EGLCore();

    void start(ANativeWindow *window);

    EGLConfig chooseConfig();

    EGLint* getAttributes();

    EGLContext createContext(EGLDisplay eglDisplay, EGLConfig eglConfig);

    GLboolean buildContext(ANativeWindow *window);

    void swapBuffer();

    void release();

protected:

private:
    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mSurface = EGL_NO_SURFACE;
    EGLContext mContext = EGL_NO_CONTEXT;
};
#endif