//
// Created by Cangwang on 2024/2/23.
//

#include "recordercontroller.h"
#include <util/elog.h>

#define LOG_TAG "RecorderController"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)


yyeva::RecorderController::RecorderController(): eglCore(make_shared<EGLCore>()),
    render(nullptr) {
}

yyeva::RecorderController::~RecorderController() {
    render->destroyRender();
}

void yyeva::RecorderController::init(EGLContext context, ANativeWindow *window, int textureId) {
    ELOGV("init textureId %d, width %d, height %d", textureId, width, height);
//    ELog::get()->i(LOG_TAG,  "init textureId %d, width %d, height %d", textureId, width, height);
    if (textureId == -1) {
        ELOGE("textureId -1");
        return;
    }

    if (eglCore == nullptr) {
        eglCore = make_shared<EGLCore>();
    }
    if (window) {
        eglCore->start(context, window);
    }

    if (window != nullptr && render == nullptr) {
        render = make_shared<FbRender>();
        width = ANativeWindow_getWidth(window);
        height = ANativeWindow_getHeight(window);
        if (width > 0 && height > 0) {
            render->updateViewPort(width, height);
        } else {
            ELOGE("init error width %d height %d", width, height);
        }
        render->setTextureId(textureId);
    }
}

void yyeva::RecorderController::setRenderConfig(shared_ptr<EvaAnimeConfig> config) {
    if (render != nullptr) {
        render->setAnimeConfig(config);
    }
}

void yyeva::RecorderController::renderFrame(int textureId, uint64_t time) {
    ELOGV("renderFrame %d", textureId);
    if (textureId == -1) {
        ELOGE("textureId -1");
        return;
    }
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (render != nullptr) {
        render->renderFrame();
    }
    //缓冲区交换
//    glFlush();
    if (eglCore != nullptr) {
        eglCore->setPresentationTime(time);
        eglCore->swapBuffer();
    }
}

void yyeva::RecorderController::destroy() {
    if (render != nullptr) {
        render->clearFrame();
        render->destroyRender();
        render = nullptr;
    }
    if (eglCore != nullptr) {
        eglCore->release();
        eglCore = nullptr;
    }
}
