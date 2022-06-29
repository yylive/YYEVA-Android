//
// Created by zengjiale on 2022/4/15.
//

#include "evaengine.h"
//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>



EvaEngine::EvaEngine(ANativeWindow *window): mWindow(window), mEGLCore(new EGLCore()), mTextureId(0), mTextureLoc(0), mMatrixLoc(0){

}

EvaEngine::~EvaEngine() {
    if (mWindow) {
        ANativeWindow_release(mWindow);
        mWindow = nullptr;
    }
    if (mEGLCore) {
        delete mEGLCore;
        mEGLCore = nullptr;
    }
}
int EvaEngine::create() {

    return mTextureId;
}
void EvaEngine::draw() {

}
void EvaEngine::stop() {

}