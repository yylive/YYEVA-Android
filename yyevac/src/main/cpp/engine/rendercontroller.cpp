//
// Created by zengjiale on 2022/4/22.
//

#include "rendercontroller.h"

#define LOG_TAG "RenderController"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)

yyeva::RenderController::RenderController():render(nullptr),config(nullptr),
                                    frameAll(nullptr), srcMap(nullptr),
                                    bgRender(nullptr), eglCore(make_shared<EGLCore>()),
                                    fbRender(nullptr) {
}

yyeva::RenderController::~RenderController() {
    if (render != nullptr) {
        render->destroyRender();
    }
    if (bgRender != nullptr) {
        bgRender->destroyRender();
    }
    if (fbRender != nullptr) {
        fbRender->destroyRender();
    }
}

GLuint yyeva::RenderController::initRender(ANativeWindow *window, bool isNeedYUV, bool isNormalMp4) {

    if (eglCore != nullptr && window != nullptr) {
        eglCore->start(window);
    }
    if (window != nullptr && render == nullptr) {
        if (isNormalMp4) {
            render = make_shared<Mp4Render>();
        } else if (isNeedYUV) {
            ELOGV("use yuv render");
            render = make_shared<YUVRender>();
        } else {
            ELOGV("use normal render");
            render = make_shared<Render>();
        }
        //如果有背景需要开启混合
        if (bgRender != nullptr) {
            render->setHasBg(true);
        }
        width = ANativeWindow_getWidth(window);
        height = ANativeWindow_getHeight(window);
    }
    return render->getExternalTexture();
}

void yyeva::RenderController::setRenderConfig(shared_ptr<EvaAnimeConfig> config) {
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

void yyeva::RenderController::videoSizeChange(int newWidth, int newHeight) {
    if (config == nullptr) return;
    if (config->videoWidth != newWidth || config->videoHeight != newHeight) {
        ELOGV("videoWidth $d, videoHeight %d, newWidth %d, newHeight %d",
              config->videoWidth, config->videoHeight, newWidth, newHeight);
        config->videoWidth = newWidth;
        config->videoHeight = newHeight;
        render->setAnimeConfig(config);
    }
}

void yyeva::RenderController::destroyRender() {
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

void yyeva::RenderController::updateViewPoint(int width, int height) {
    if (bgRender != nullptr) {
        bgRender->updateViewPort(width, height);
    }
    if (render != nullptr) {
        render->updateViewPort(width, height);
    } else {
        ELOGE("render is null");
    }
}

int yyeva::RenderController::getExternalTexture() {
    if (render != nullptr) {
        return render->getExternalTexture();
    } else {
        ELOGV("render not init");
        return -1; //未初始化
    }
}

void yyeva::RenderController::renderFrame() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    if (videoRecord) { //视频录制使用fbo后的纹理作为传输
        //获取录制使用的FrameBuffer
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
    }
    if (bgRender != nullptr) {
        bgRender->renderFrame();
    }
    if (render != nullptr) {
        render->renderFrame();
    }
    if (videoRecord) {
        if (!config->isMix) {
            glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
            if (fbRender != nullptr) {
                fbRender->renderFrame();
            }
        }
    }
}

void yyeva::RenderController::renderSwapBuffers() {
    if (bgRender != nullptr) {
        bgRender->swapBuffers();
    }
    if (render != nullptr) {
        render->swapBuffers();
    }
    if (fbRender != nullptr) {
        fbRender->swapBuffers();
    }
    if (eglCore != nullptr) {
        eglCore->swapBuffer();
    }
}

void yyeva::RenderController::renderClearFrame() {
    if (bgRender != nullptr) {
        bgRender->clearFrame();
    }
    if (render != nullptr) {
        render->clearFrame();
    }
    if (fbRender != nullptr) {
        fbRender->clearFrame();
    }
    if (eglCore != nullptr) {
        eglCore->swapBuffer();
    }
}

void yyeva::RenderController::releaseTexture() {
    if (bgRender != nullptr) {
        bgRender->releaseTexture();
    }
    if (render != nullptr) {
        render->releaseTexture();
    }
    if (fbRender != nullptr) {
        fbRender->releaseTexture();
    }
    if (videoRecord) {
        destroyRecordFrameBuffer();
        videoRecord = false;
    }
}

void yyeva::RenderController::mixConfigCreate(shared_ptr<EvaAnimeConfig> config) {
    if (config == nullptr) {
        return;
    } else {
        this->config = config;
    }
    if (!config->isMix) return;
    parseSrc(config);
    parseFrame(config);
}

void yyeva::RenderController::mixRenderCreate() {
    mixRender = make_shared<EvaMixRender>();
    if (mixRender != nullptr && config != nullptr && srcMap != nullptr) {
        mixRender->init(srcMap);
    }
}

void yyeva::RenderController::mixRendering(int frameIndex) {
    if (!config->isMix) return;
    int videoTextureId = getExternalTexture();
    if (videoTextureId == -1) return;

    curFrameIndex = frameIndex;
    if (frameAll!= nullptr && !frameAll->map.empty()) {
        if (frameAll->map.size() <= frameIndex) {
            ELOGE("mixRendering fps is error, please set fps");
            return;
        }

        list<shared_ptr<EvaFrame>> list = frameAll->map.find(frameIndex)->second->list;
//    std::list<EvaFrame>::iterator it;
//    for (it = list.begin(); it != list.end(); ++it) {
//        EvaSrc src = srcMap->map.find(it->srcId)->second;
//        mixRender->rendFrame(videoTextureId, config, *it, src);
//    }
        if (!list.empty()) {
            if (srcMap != nullptr && !srcMap->map.empty()) {
                if (mixRender != nullptr) {
                    for (shared_ptr<EvaFrame> frame: list) {
                        if (frame->srcId.empty()) {
                            ELOGE("mixRendering frame is empty");
                            continue;
                        }
                        auto src = srcMap->map.find(frame->srcId)->second;
                        mixRender->rendFrame(videoTextureId, config, frame, src);
                    }
                    if (videoRecord) {
                        //获取录制使用的FrameBuffer
                        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
                        if (fbRender != nullptr) {
                            fbRender->renderFrame();
                        }
                    }
                } else {
                    ELOGE("mixRendering mixRender is null");
                }
            }
        } else {
            ELOGE("mixRendering mixRender list is empty");
        }
    }

}

void yyeva::RenderController::mixRenderRelease(int textureId) {
    if (mixRender != nullptr) {
        mixRender->release(textureId);
    }
}

void yyeva::RenderController::mixRenderDestroy() {
//    int textureId = getExternalTexture();
//    if (textureId != -1) {
//        mixRender->release(textureId);
//    }

    if (mixRender != nullptr && srcMap != nullptr && srcMap->map.size() > 0) {
        map<string, shared_ptr<EvaSrc>>::iterator iter;
        iter = srcMap->map.begin();
        while(iter != srcMap->map.end()) {
            mixRender->release(iter->second->srcTextureId);
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

void yyeva::RenderController::parseFrame(shared_ptr<EvaAnimeConfig> config) {
    if (config != nullptr) {
//        frameAll = new EvaFrameAll(config->datas);
        frameAll = make_shared<EvaFrameAll>(config->datas);
    }
}

void yyeva::RenderController::parseSrc(shared_ptr<EvaAnimeConfig> config) {
    if (config != nullptr) {
        srcMap = make_shared<EvaSrcMap>(config->effects);
    }
}

void yyeva::RenderController::setSrcBitmap(const char *srcId, unsigned char *bitmap,
                                    AndroidBitmapInfo* bitmapInfo, string scaleMode) {
    if (srcMap != nullptr && !srcMap->map.empty()) {
        string id = srcId;
//        srcMap->map.find(id)->second.bitmap = bitmap;
//        srcMap->map.find(id)->second.bitmapInfo = bitmapInfo;
        srcMap->map.find(id)->second->bitmapWidth = bitmapInfo->width;
        srcMap->map.find(id)->second->bitmapHeight = bitmapInfo->height;
        srcMap->map.find(id)->second->bitmapFormat = bitmapInfo->format;
        if (scaleMode == "aspectFit") {
            srcMap->map.find(id)->second->fitType = EvaSrc::CENTER_FIT;
        } else if (scaleMode == "aspectFill") {
            srcMap->map.find(id)->second->fitType = EvaSrc::CENTER_FULL;
        } else {
            srcMap->map.find(id)->second->fitType = EvaSrc::FIX_XY;
        }
        if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGB_565) { //RGB三通道，例如jpg格式
            int size = bitmapInfo->stride * bitmapInfo->height;
            srcMap->map.find(id)->second->bitmap = new unsigned char[size];
            memset(srcMap->map.find(id)->second->bitmap, 0, size);
            memcpy(srcMap->map.find(id)->second->bitmap, bitmap, size);
        } else if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGBA_8888) {  //RGBA四通道，例如png格式 android图片是ARGB排列，所以还是要做图片转换的
            int size = bitmapInfo->stride * bitmapInfo->height;
//            int size = bitmapInfo->width * bitmapInfo->height * sizeof(char);
            srcMap->map.find(id)->second->bitmap = new unsigned char[size];
            memset(srcMap->map.find(id)->second->bitmap, 0, size);
//            srcMap->map.find(id)->second.saveAddress = addr;
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
            memcpy(srcMap->map.find(id)->second->bitmap, bitmap, size);
        }
    }
}

void yyeva::RenderController::setSrcTxt(const char *srcId, const char *txt) {
    if (srcMap != nullptr && !srcMap->map.empty()) {
        string id = srcId;
        string t = txt;
        srcMap->map.find(id)->second->txt = t;
    }
}

void yyeva::RenderController::setBgImage(unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo) {
    ELOGV("setBgImage");
    if (bgRender == nullptr) {
        bgRender = make_shared<BgRender>();
        if (render != nullptr) {
            render->setHasBg(true);
        }
    }

    bgRender->setBgImage(bitmap, bitmapInfo);
}

EGLContext yyeva::RenderController::getEglContext() {
    return eglCore->getEglContext();
}

void yyeva::RenderController::setVideoRecord(bool videoRecord) {
    if (videoRecord && width > 0 && height > 0) { //设置为录制
        initRecordFrameBuffer(width, height);
        fbRender = make_shared<FbRender>();
        fbRender->setTextureId(mFrameBufferTextures);
    } else if (this->videoRecord && !videoRecord) { //之前需要录制，现在设置不录制
        destroyRecordFrameBuffer();
        fbRender = nullptr;
    }
    this->videoRecord = videoRecord;
}

GLuint yyeva::RenderController::getRecordFramebufferId() {
    return mFrameBufferTextures;
}

void yyeva::RenderController::initRecordFrameBuffer(int width, int height) {
    //比对大小
    if (mFrameWidth != width || mFrameHeight !=height){
        destroyRecordFrameBuffer();
    }
    mFrameWidth = width;
    mFrameHeight = height;
    mFrameBuffer = 0;
    mFrameBufferTextures = 0;
    //生成帧缓冲id
    glGenFramebuffers(1,&mFrameBuffer);
    //生成纹理id
    glGenTextures(1,&mFrameBufferTextures);
    //绑定纹理
    glBindTexture(GL_TEXTURE_2D,mFrameBufferTextures);
    //纹理赋值为空，先纹理占位
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE, nullptr);
    //设定纹理参数
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    //绑定帧图
    glBindFramebuffer(GL_FRAMEBUFFER,mFrameBuffer);
    //绑定纹理到帧图
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,mFrameBufferTextures,0);
    //切换回默认纹理
    glBindTexture(GL_TEXTURE_2D,0);
    //切换回默认的帧缓冲
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    ELOGV("framebufferTextures %d", mFrameBufferTextures);
}

void yyeva::RenderController::destroyRecordFrameBuffer() {
    glDeleteTextures(1, &mFrameBufferTextures);
    glDeleteFramebuffers(1, &mFrameBuffer);
    mFrameWidth = -1;
    mFrameHeight = -1;
    ELOGV("destroyRecordFrameBuffer");
}
