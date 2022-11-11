//
// Created by zengjiale on 2022/4/22.
//

#include "rendercontroller.h"


RenderController::RenderController() : surface(nullptr), config(nullptr),
                                       frameAll(nullptr), srcMap(nullptr), eglCore(new EGLCore()) {
    renderList.clear();
}

RenderController::~RenderController() {
    surface = nullptr;
    config = nullptr;
    frameAll = nullptr;
    srcMap = nullptr;
    eglCore->release();
    eglCore = nullptr;
    if (!renderList.empty()) {
        for (IRender *render : renderList) {
            render->releaseTexture();
        }
        renderList.clear();
    }
}

GLuint RenderController::initRender(ANativeWindow *window, bool isNeedYUV) {
    if (eglCore != nullptr && window != nullptr) {
//        eglCore->release();
        eglCore->start(window);
    }
    surface = window;
    if (isNeedYUV) {
        ELOGV("use yuv render");
        IRender *render = new YUVRender();
        renderList.push_back(render);
        textureId = render->getExternalTexture();
    } else {
        ELOGV("use normal render");
        IRender *render = new Render();
        renderList.push_back(render);
        textureId = render->getExternalTexture();
    }

    return textureId;
}

void RenderController::setRenderConfig(EvaAnimeConfig *config) {
    if (config == nullptr) {
        return;
    } else {
        this->config = config;
        config->videoTextureId = textureId;
        config->bgTextureId = bgTextureId;
    }
    if (!renderList.empty()) {
        for (IRender *render : renderList) {
            render->setAnimeConfig(config);
        }
    }
}

void RenderController::videoSizeChange(int newWidth, int newHeight) {
    if (config == nullptr) return;
    if (config->videoWidth != newWidth || config->videoHeight != newHeight) {
        ELOGV("videoWidth %d, videoHeight %d, newWidth %d, newHeight %d",
              config->videoWidth, config->videoHeight, newWidth, newHeight);
        config->videoWidth = newWidth;
        config->videoHeight = newHeight;
        if (!renderList.empty()) {
            for (IRender *render : renderList) {
                render->setAnimeConfig(config);
            }
        }
    }
}

void RenderController::destroyRender() {
    if (!renderList.empty()) {
        for (IRender *render : renderList) {
            render->releaseTexture();
            render = nullptr;
        }
        renderList.clear();
    }
    surface = nullptr;

    if (eglCore != nullptr) {
        eglCore->release();
    }
}

void RenderController::updateViewPoint(int width, int height) {
    if (width > 0 && height > 0) {
        surfaceWidth = width;
        surfaceHeight = height;
    }
    if (config != nullptr && config->params != nullptr && config->params->useFbo) {
        framebufferRender = make_shared<FramebufferRender>();
        //先设置宽高
        framebufferRender->updateViewPort(width, height);
        //再初始化数据
        framebufferRender->setAnimeConfig(config);
    }

    if (!renderList.empty()) {
        for (IRender *render : renderList) {
            render->updateViewPort(width, height);
        }
    }
}

int RenderController::getExternalTexture() {
//    if (render != nullptr) {
//        return render->getExternalTexture();
//    } else {
//        ELOGV("render not init");
//        return -1; //未初始化
//    }
    return textureId;
}

void RenderController::renderSwapBuffers() {
    if (!renderList.empty()) {
        for (IRender *render : renderList) {
            render->swapBuffers();
        }
    }

    if (eglCore != nullptr) {
        eglCore->swapBuffer();
    }
}

void RenderController::renderClearFrame() {
    if (!renderList.empty()) {
        for (IRender *render : renderList) {
            render->clearFrame();
        }
    }

    if (eglCore != nullptr) {
        eglCore->swapBuffer();
    }
}

void RenderController::releaseTexture() {
    if (!renderList.empty()) {
        for (IRender *render : renderList) {
            render->releaseTexture();
        }
    }
}

void RenderController::mixConfigCreate(EvaAnimeConfig *config) {
    if (config == nullptr) {
        return;
    } else {
        this->config = config;
    }
    if (!config->isMix) return;
    parseSrc(config);
    parseFrame(config);

    list<EvaSrc> srcList;
    map<string, EvaSrc>::iterator it;
    for (it = srcMap->map.begin(); it != srcMap->map.end(); it++) {
        srcList.push_back(it->second);
    }
    if (config->listener != nullptr) {
        config->listener->onVideoConfigReady(srcList);
    }
}

void RenderController::mixRenderCreate() {
    ELOGV("mixRenderCreate");
    auto* mixRender = new MixRender();
    if (surfaceWidth > 0 && surfaceHeight > 0) {
        mixRender->updateViewPort(surfaceWidth, surfaceHeight);
    }
    renderList.push_back(mixRender);
}

void RenderController::setMixSrc(vector<EvaSrc *> v) {
    if (v.empty()) {
        ELOGV("setMixSrc v empty");
        return;
    }

    if (srcMap->map.empty()) {
        ELOGV("setMixSrc srcMap empty");
        return;
    }

    map<string, EvaSrc>::iterator it;
    for (it = srcMap->map.begin(); it != srcMap->map.end(); it++) {
        for (int i = 0; i < v.size(); ++i) {
            if (it->second.srcTag == BG_TAG) {
                ELOGV("setSrcBitmap set bg_effect");
                if (config != nullptr) {
                    setBgImage(&it->second);
                }
            } else if (it->second.srcTag == v[i]->srcTag) {
                ELOGV("setSrcBitmap setMixSrc = %s", v[i]->srcTag.c_str());
                it->second.bitmapWidth = v[i]->bitmapWidth;
                it->second.bitmapHeight = v[i]->bitmapHeight;
                it->second.bitmapFormat = v[i]->bitmapFormat;
                it->second.fitType = v[i]->fitType;
                int size = v[i]->size;
                it->second.bitmap = new unsigned char[size];
                memset(it->second.bitmap, 0, size);
                memcpy(it->second.bitmap, v[i]->bitmap, size);
            }
        }
    }

    v.clear();
}

void RenderController::renderFrame(int frameIndex) {
    curFrameIndex = frameIndex;
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (config != nullptr && config->params != nullptr && config->params->useFbo) {
        if (!renderList.empty()) {
            if (framebufferRender != nullptr) {
                framebufferRender->enableFramebuffer();
            }
            for (IRender *render : renderList) {
                render->renderFrame(frameIndex);
            }
            if (framebufferRender != nullptr) {
                framebufferRender->disableFrameBuffer();
            }
        }
        if (framebufferRender != nullptr) {
            framebufferRender->renderFrame(frameIndex);
        }
    } else {
        if (!renderList.empty()) {
            for (IRender *render : renderList) {
                render->renderFrame(frameIndex);
            }
        }
    }
}

void RenderController::mixRenderDestroy() {
    if (srcMap != nullptr) {
        srcMap->map.clear();
    }

    if (frameAll != nullptr) {
        frameAll->map.clear();
    }
    srcMap = nullptr;
    frameAll = nullptr;
}

void RenderController::parseFrame(EvaAnimeConfig *config) {
    if (config != nullptr) {
        frameAll = new EvaFrameAll(config->datas);
        config->frameAll = frameAll;
    }
}

void RenderController::parseSrc(EvaAnimeConfig *config) {
    if (config != nullptr) {
        srcMap = new EvaSrcMap(config->effects);
        config->srcMap = srcMap;
    }
}

void RenderController::setSrcBitmap(const char *srcId, unsigned char *bitmap,
                                    AndroidBitmapInfo *bitmapInfo, string addr, string scaleMode) {
    if (srcMap != nullptr && !srcMap->map.empty()) {
        string id = srcId;
        srcMap->map.find(id)->second.bitmapWidth = bitmapInfo->width;
        srcMap->map.find(id)->second.bitmapHeight = bitmapInfo->height;
        srcMap->map.find(id)->second.bitmapFormat = bitmapInfo->format;
        if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGB_565) { //RGB三通道，例如jpg格式
            int size = bitmapInfo->stride * bitmapInfo->height;
            srcMap->map.find(id)->second.bitmap = new unsigned char[size];
            memset(srcMap->map.find(id)->second.bitmap, 0, size);
            memcpy(srcMap->map.find(id)->second.bitmap, bitmap, size);
        } else if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGBA_8888) {  //RGBA四通道，例如png格式 android图片是ARGB排列，所以还是要做图片转换的
            int size = bitmapInfo->stride * bitmapInfo->height;
//            int size = bitmapInfo->width * bitmapInfo->height * sizeof(char);
            srcMap->map.find(id)->second.bitmap = new unsigned char[size];
            memset(srcMap->map.find(id)->second.bitmap, 0, size);
            srcMap->map.find(id)->second.saveAddress = addr;
            memcpy(srcMap->map.find(id)->second.bitmap, bitmap, size);
        }
    }
}

bool RenderController::setSrcBitmap(string srcTag, unsigned char *bitmap,
                                    AndroidBitmapInfo *bitmapInfo, string scaleMode) {
    if (bitmapInfo == nullptr || bitmap == nullptr) {
        ELOGE("setSrcBitmap bitmap is null");
        return false;
    }

    if (srcTag == BG_TAG) {
        setBgImage(bitmap, bitmapInfo);
        return true;
    }

    if (srcMap == nullptr) {
        ELOGE("setSrcBitmap srcMap is null");
        return false;
    }

    if (!srcMap->map.empty()) {
        map<string, EvaSrc>::iterator it;
        for (it = srcMap->map.begin(); it != srcMap->map.end(); it++) {
            if (it->second.srcTag == srcTag) {
                ELOGV("setSrcBitmap tag = %s", srcTag.c_str());
                it->second.bitmapWidth = bitmapInfo->width;
                it->second.bitmapHeight = bitmapInfo->height;
                it->second.bitmapFormat = bitmapInfo->format;
                if (scaleMode == "aspectFit") {
                    it->second.fitType = EvaSrc::CENTER_FIT;
                } else if (scaleMode == "aspectFill") {
                    it->second.fitType = EvaSrc::CENTER_FULL;
                } else {
                    it->second.fitType = EvaSrc::FIX_XY;
                }
                if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGB_565) { //RGB三通道，例如jpg格式
                    int size = bitmapInfo->stride * bitmapInfo->height;
                    it->second.bitmap = new unsigned char[size];
                    memset(it->second.bitmap, 0, size);
                    memcpy(it->second.bitmap, bitmap, size);
                } else if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGBA_8888) {  //RGBA四通道，例如png格式 android图片是ARGB排列，所以还是要做图片转换的
                    int size = bitmapInfo->stride * bitmapInfo->height;
                    it->second.bitmap = new unsigned char[size];
                    memset(it->second.bitmap, 0, size);
//                    it->second.saveAddress = addr;
                    memcpy(it->second.bitmap, bitmap, size);
                }
                return true;
            }
        }
    }
    return false;
}

void RenderController::setSrcTxt(const char *srcId, const char *txt) {
    if (srcMap != nullptr && !srcMap->map.empty()) {
        string id = srcId;
        string t = txt;
        srcMap->map.find(id)->second.txt = t;
    }
}

void RenderController::setBgImage(EvaSrc* src) {
    ELOGV("setBgImage");
    if (config != nullptr) {
        config->bgTextureId = TextureLoadUtil::loadTexture(src);
    }
    IRender* bgRender = new BgRender();
    //背景加在最底层， 最先绘制
    renderList.push_front(bgRender);
}

void RenderController::setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo) {
    ELOGV("setBgImage");
    if (config != nullptr) {
        config->bgTextureId = TextureLoadUtil::loadTexture(bitmap, bitmapInfo);
        bgTextureId = config->bgTextureId;
    } else {
        bgTextureId = TextureLoadUtil::loadTexture(bitmap, bitmapInfo);
    }
    IRender* bgRender = new BgRender();
    if (config != nullptr) {  //防止多进程初始化没有设置
        bgRender->setAnimeConfig(config);
    }
    renderList.push_front(bgRender);
}

void RenderController::touchPoint(int x, int y) {
    if (config != nullptr && config->params != nullptr) {
        int realX = x;
        int realY = y;
        if (config->params->scaleType == 1) {
            if (config->videoWidth / 2 < config->videoHeight) {
                float ratio = (float) config->height / surfaceHeight;
                float distanceX = (config->width / ratio - surfaceWidth) / 2;
                realX = (x + distanceX) * ratio;
                realY = y * ratio;
            } else {
                float ratio = ((float) config->width / surfaceWidth);
                float distanceY = (config->height / ratio - surfaceHeight) / 2;
                realX = x * ratio;
                realY = (y + distanceY) * ratio;
            }
            ELOGV("surfaceWidth %i surfaceHeight %i x %i y %i realX %i realY %i", surfaceWidth,
                  surfaceHeight, x, y, realX, realY);
        } else if (config->params->scaleType == 2) {
            realX = x * (float) (config->videoWidth * 0.5) / surfaceWidth;
            realY = y * (float) config->videoHeight / surfaceHeight;
        } else if (config->params->scaleType == 3) {

        }

        auto list = frameAll->map[curFrameIndex].list;
        for (EvaFrame frame: list) {
            auto src = srcMap->map[frame.srcId];
            if (calClick(realX, realY, frame.frame)) {
                //todo callClick listener
                config->listener->touchCallback(x, y, src);
            }
        }
    }
}

bool RenderController::calClick(int x, int y, PointRect *frame) {
    return x >= frame->x && x <= (frame->x + frame->w)
           && y >= frame->y && y <= (frame->y + frame->h);
}
