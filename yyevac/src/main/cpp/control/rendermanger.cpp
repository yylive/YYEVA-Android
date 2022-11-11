//
// Created by zengjiale on 2022/8/25.
//

#include "rendermanger.h"

RenderManager::RenderManager():controller(new RenderController()), optionParams(nullptr), renderThread(nullptr) {
    createThread(renderThread, "render_manager");
}

RenderManager::~RenderManager() {
    if (controller != nullptr) {
        controller->releaseTexture();
        controller->destroyRender();
        controller = nullptr;
    }
    optionParams = nullptr;
    quitSafely(renderThread);
}

void RenderManager::renderData() {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            try {
                //updateTexImage
                if (controller != nullptr) {
                    mixRendering();
                }
                if (controller != nullptr) {
                    controller->renderSwapBuffers();
                }
            } catch (exception e) {
                ELOGE("renderData %s", e.what());
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::onDecoding(int decodeIndex) {
    this->decodeIndex = decodeIndex;
}

void RenderManager::onDecodeEnd(int decodeIndex) {
    ELOGV("onDecodeEnd decodeEndIndex = %i", decodeIndex);
    this->decodeEndIndex = decodeIndex;
}

void RenderManager::onLoopStart() {
    ELOGV("onLoopStart");
    this->decodeIndex = 0;
    this->frameIndex = 0;
    this->frameDiffTimes = 0;
    this->decodeEndIndex = 0;
}

void RenderManager::setReleaseFunc(std::function<void()> releaseCallback) {
//    this->releaseCallback = releaseCallback;
}

void RenderManager::mixRendering() {
    if (decodeIndex > frameIndex + 1 || frameDiffTimes >= DIFF_TIMES) {
        ELOGV("jump frameIndex %i, decodeIndex= %i, frameDiffTimes=%i", frameIndex, decodeIndex, frameDiffTimes);
    }

    if (decodeIndex != frameIndex) {
        frameDiffTimes ++;
    } else  {
        frameDiffTimes = 0;
    }

    ELOGV("onRendering frameIndex = %i, decodeEndIndex = %i",frameIndex, decodeEndIndex);
    if (controller != nullptr) {
        controller->renderFrame(frameIndex);
    }

    if (decodeEndIndex != 0 && decodeEndIndex == frameIndex) {
//        releaseCallback();
        // release
    }

    frameIndex ++;
}

GLuint RenderManager::initRender(ANativeWindow *window, bool isNeedYUV) {
//    if (renderThread != nullptr) {
//        auto renderWork = [&](RenderController* c, ANativeWindow *w, bool yuv) -> GLuint
//        {
//            if (c != nullptr) {
//                GLuint r = c->initRender(w, yuv);
//                return r;
//            } else {
//                return -1;
//            }
//        };
//        renderThread->submit(renderWork, controller, window, isNeedYUV);
//    }
    if (controller != nullptr) {
        if (controller->getExternalTexture() == -1) {
            return controller->initRender(window, isNeedYUV);
        } else {
            return controller->getExternalTexture();
        }
    } else {
        return -1;
    }
}

void RenderManager::setRenderConfig(EvaAnimeConfig *config) {
//    if (renderThread != nullptr) {
//        auto renderWork = [&]() -> void
//        {
//            if (controller != nullptr) {
//                controller->setRenderConfig(config);
//            }
//        };
//        renderThread->submit(renderWork);
//    }
    if (controller != nullptr) {
        if (config == nullptr) {
            ELOGE("setRenderConfig config null");
        }
        if (optionParams == nullptr) {
            ELOGE("setRenderConfig optionParams null");
        }
        config->params = optionParams;
        controller->setRenderConfig(config);

//        controller->mixInitSrc();
    }
}

void RenderManager::videoSizeChange(int newWidth, int newHeight) {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            if (controller != nullptr) {
                controller->videoSizeChange(newWidth, newHeight);
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::destroyRender() {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            if (controller != nullptr) {
                controller->destroyRender();
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::updateViewPoint(int width, int height) {
    if (renderThread != nullptr) {
//        auto renderWork = [&]() -> void
//        {
//            if (controller != nullptr) {
//                controller->updateViewPoint(width, height);
//            }
//        };
//        renderThread->submit(renderWork);
        controller->updateViewPoint(width, height);
    }
}

int RenderManager::getExternalTexture() {
    if (controller != nullptr) {
        return controller->getExternalTexture();
    }
    return -1;
}

void RenderManager::renderFrame() {
//    if (renderThread != nullptr) {
//        auto renderWork = [&]() -> void
//        {
//            if (controller != nullptr) {
//                controller->renderFrame();
//                controller->renderSwapBuffers();
//            }
//        };
//        renderThread->submit(renderWork);
//    }
    if (controller != nullptr) {
        mixRendering();
        controller->renderSwapBuffers();
    }
}

void RenderManager::renderSwapBuffers() {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            if (controller != nullptr) {
                controller->renderSwapBuffers();
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::renderClearFrame() {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            if (controller != nullptr) {
                controller->renderClearFrame();
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::releaseTexture() {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            if (controller != nullptr) {
                controller->releaseTexture();
            }
            if (controller->surface != nullptr) {
                ANativeWindow_release(controller->surface);
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::mixConfigCreate(EvaAnimeConfig *config) {
//    if (renderThread != nullptr) {
//        auto renderWork = [&]() -> void
//        {
//            if (controller != nullptr) {
//                controller->mixConfigCreate(config);
//            }
//        };
//        renderThread->submit(renderWork);
//    }
    if (controller != nullptr) {
        controller->mixConfigCreate(config);
    }
}

void RenderManager::setMixSrc(vector<EvaSrc*> v) {
    if (controller != nullptr) {
        controller->setMixSrc(v);
    }
}

void RenderManager::mixRenderCreate() {
//    if (renderThread != nullptr) {
//        auto renderWork = [&]() -> void
//        {
//            if (controller != nullptr) {
//                controller->mixRenderCreate();
//            }
//        };
//        renderThread->submit(renderWork);
//    }
    if (controller != nullptr) {
        controller->mixRenderCreate();
    }
}

void RenderManager::mixRenderDestroy() {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            if (controller != nullptr) {
                controller->mixRenderDestroy();
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::setParams(OptionParams *params) {
    optionParams = params;
}

bool RenderManager::setSrcBitmap(string name, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode) {
//    if (renderThread != nullptr) {
//        auto renderWork = [&]() -> void
//        {
//            if (controller != nullptr) {
//                controller->setSrcBitmap(name, bitmap, bitmapInfo);
//            }
//        };
//        renderThread->submit(renderWork);
//    }
    if (controller != nullptr) {
        return controller->setSrcBitmap(name, bitmap, bitmapInfo, scaleMode);
    }
    return false;
}

void RenderManager::setSrcTxt(const char *srcId, const char *txt) {
    if (renderThread != nullptr) {
        auto renderWork = [&]() -> void
        {
            if (controller != nullptr) {
                controller->setSrcTxt(srcId, txt);
            }
        };
        renderThread->submit(renderWork);
    }
}

void RenderManager::touchPoint(int x, int y) {
    if (controller != nullptr) {
        controller->touchPoint(x, y);
    }
}


bool RenderManager::createThread(ThreadPool *t, string name) {
    try {
        if (t == nullptr) {
            t = new ThreadPool(1);
            t->init();
            renderThread = t;
        }
        return true;
    } catch (exception e) {
        ELOGE("createThread failed: %s", e.what());
    }
    return false;
}

void RenderManager::quitSafely(ThreadPool *t) {
    if (t != nullptr) {
        t->shutdown();
        t = nullptr;
    }
}

ThreadPool *RenderManager::getRenderThread() {
    return renderThread;
}
