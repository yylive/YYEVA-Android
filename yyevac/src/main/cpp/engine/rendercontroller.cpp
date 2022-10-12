//
// Created by zengjiale on 2022/4/22.
//

#include "rendercontroller.h"

#define LOG_TAG "RenderController"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

RenderController::RenderController():render(nullptr),config(nullptr),
                                    frameAll(nullptr), srcMap(nullptr), bgRender(nullptr), eglCore(new EGLCore()) {

}

RenderController::~RenderController() {
    render = nullptr;
    bgRender = nullptr;
    config = nullptr;
    srcMap = nullptr;
    frameAll = nullptr;
    eglCore = nullptr;
}

GLuint RenderController::initRender(ANativeWindow *window, bool isNeedYUV) {
    if (eglCore != nullptr && window != nullptr) {
        eglCore->start(window);
    }
    if (window != nullptr && render == nullptr) {
        if (isNeedYUV) {
            ELOGV("use yuv render");
            render = new YUVRender();
        } else {
            ELOGV("use normal render");
            render = new Render();
        }
    }
    return render->getExternalTexture();
}

void RenderController::setRenderConfig(EvaAnimeConfig *config) {
    if (config == nullptr) {
        return;
    } else {
        this->config = config;
    }
    if (bgRender != nullptr) {
        bgRender->setAnimeConfig(config);
    }
    if (render != nullptr) {
        render->setAnimeConfig(config);
    }
}

void RenderController::videoSizeChange(int newWidth, int newHeight) {
    if (config == nullptr) return;
    if (config->videoWidth != newWidth || config->videoHeight != newHeight) {
        ELOGV("videoWidth $d, videoHeight %d, newWidth %d, newHeight %d",
              config->videoWidth, config->videoHeight, newWidth, newHeight);
        config->videoWidth = newWidth;
        config->videoHeight = newHeight;
        render->setAnimeConfig(config);
    }
}

void RenderController::destroyRender() {
    if (bgRender != nullptr) {
        bgRender->releaseTexture();
        bgRender = nullptr;
    }

    if (render != nullptr) {
        render->releaseTexture();
        render = nullptr;
    }
    if (eglCore != nullptr) {
        eglCore->release();
    }
}

void RenderController::updateViewPoint(int width, int height) {
    if (bgRender != nullptr) {
        bgRender->updateViewPort(width, height);
    }
    if (render != nullptr) {
        render->updateViewPort(width, height);
    }
}

int RenderController::getExternalTexture() {
    if (render != nullptr) {
        return render->getExternalTexture();
    } else {
        ELOGV("render not init");
        return -1; //未初始化
    }
}

void RenderController::renderFrame() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (bgRender != nullptr) {
        bgRender->renderFrame();
    }
    if (render != nullptr) {
        render->renderFrame();
    }
}

void RenderController::renderSwapBuffers() {
    if (bgRender != nullptr) {
        bgRender->swapBuffers();
    }
    if (render != nullptr) {
        render->swapBuffers();
    }
    if (eglCore != nullptr) {
        eglCore->swapBuffer();
    }
}

void RenderController::renderClearFrame() {
    if (bgRender != nullptr) {
        bgRender->clearFrame();
    }
    if (render != nullptr) {
        render->clearFrame();
    }
    if (eglCore != nullptr) {
        eglCore->swapBuffer();
    }
}

void RenderController::releaseTexture() {
    if (bgRender != nullptr) {
        bgRender->releaseTexture();
    }
    if (render != nullptr) {
        render->releaseTexture();
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
}

void RenderController::mixRenderCreate() {
    mixRender = new EvaMixRender();
    if (mixRender != nullptr && config != nullptr && srcMap != nullptr) {
        mixRender->init(srcMap);
    }
}

void RenderController::mixRendering(int frameIndex) {
    if (!config->isMix) return;
    int videoTextureId = getExternalTexture();
    if (videoTextureId == -1) return;

    curFrameIndex = frameIndex;
    if (frameAll!= nullptr && !frameAll->map.empty()) {
        list<EvaFrame> list = frameAll->map.find(frameIndex)->second.list;
//    std::list<EvaFrame>::iterator it;
//    for (it = list.begin(); it != list.end(); ++it) {
//        EvaSrc src = srcMap->map.find(it->srcId)->second;
//        mixRender->rendFrame(videoTextureId, config, *it, src);
//    }

        if (srcMap != nullptr && !srcMap->map.empty()) {
            for (EvaFrame frame: list) {
                EvaSrc src = srcMap->map.find(frame.srcId)->second;
                mixRender->rendFrame(videoTextureId, config, &frame, &src);
            }
        }
    }
}

void RenderController::mixRenderRelease(int textureId) {
    if (mixRender != nullptr) {
        mixRender->release(textureId);
    }
}

void RenderController::mixRenderDestroy() {
//    int textureId = getExternalTexture();
//    if (textureId != -1) {
//        mixRender->release(textureId);
//    }

    if (mixRender != nullptr && srcMap != nullptr && srcMap->map.size() > 0) {
        map<string, EvaSrc>::iterator iter;
        iter = srcMap->map.begin();
        while(iter != srcMap->map.end()) {
            mixRender->release(iter->second.srcTextureId);
            iter++;
        }
        srcMap->map.clear();
    }

    if (frameAll != nullptr) {
        frameAll->map.clear();
    }
    mixRender = nullptr;
    srcMap = nullptr;
    frameAll = nullptr;
}

void RenderController::parseFrame(EvaAnimeConfig* config) {
    if (config != nullptr) {
        frameAll = new EvaFrameAll(config->datas);
    }
}

void RenderController::parseSrc(EvaAnimeConfig* config) {
    if (config != nullptr) {
        srcMap = new EvaSrcMap(config->effects);
    }
}

void RenderController::setSrcBitmap(const char *srcId, unsigned char *bitmap,
                                    AndroidBitmapInfo* bitmapInfo, string addr) {
    if (srcMap != nullptr && !srcMap->map.empty()) {
        string id = srcId;
//        srcMap->map.find(id)->second.bitmap = bitmap;
//        srcMap->map.find(id)->second.bitmapInfo = bitmapInfo;
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
//            for(int i = 0; i < size; i++)
//            {
//                int pixel = bitmap[i];
//
//                int a = (pixel >> 24) & 0xFF;
//                int r = (pixel >> 16) & 0xFF;
//                int g = (pixel >>  8) & 0xFF;
//                int b = (pixel >>  0) & 0xFF;
//
//                bitmap[i] = (int)(a | (r << 24 ) | (g << 16) | (b << 8));
//            }
            memcpy(srcMap->map.find(id)->second.bitmap, bitmap, size);
        }
    }
}

void RenderController::setSrcTxt(const char *srcId, const char *txt) {
    if (srcMap != nullptr && !srcMap->map.empty()) {
        string id = srcId;
        string t = txt;
        srcMap->map.find(id)->second.txt = t;
    }
}

void RenderController::setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo) {
    ELOGV("setBgImage");
    if (bgRender == nullptr) {
        bgRender = new BgRender();
    }

    bgRender->setBgImage(bitmap, bitmapInfo);
}
