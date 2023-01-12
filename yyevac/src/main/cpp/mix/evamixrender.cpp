//
// Created by zengjiale on 2022/4/18.
//

#include "evamixrender.h"

#define LOG_TAG "EvaMixRender"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

EvaMixRender::EvaMixRender() {

}

EvaMixRender::~EvaMixRender() {

}

void EvaMixRender::init(EvaSrcMap* evaSrcMap) {
    shader = new MixShader();
    glDisable(GL_DEPTH_TEST);  //关闭深度测试

    if (!evaSrcMap->map.empty()) {
        map<string, EvaSrc>::iterator it;
        for (it = evaSrcMap->map.begin(); it != evaSrcMap->map.end(); it++) {
//            cout << (*it).first << " " << (*it).second << endl;
            ELOGV("init srcId = %s", it->second.srcId.c_str());
            TextureLoadUtil::loadTexture(&it->second);
            if (shader != nullptr && shader->program != 0) {
                ELOGV("textureProgram=%d, textureId=%d", shader->program, it->second.srcTextureId);
            } else {
                ELOGE("shader program error");
            }
        }
    }
}

void EvaMixRender::rendFrame(GLuint videoTextureId, EvaAnimeConfig* config, EvaFrame* frame, EvaSrc* src) {
    if (videoTextureId <= 0) {
        ELOGE("rendFrame videoTextureId = 0");
        return;
    }
    if (shader == nullptr) {
        ELOGE("rendFrame shader is null");
        return;
    }
    if (config == nullptr) {
        ELOGE("rendFrame config is null");
        return;
    }

    if (frame == nullptr) {
        ELOGE("rendFrame frame is null");
        return;
    }

    if (src == nullptr) {
        ELOGE("rendFrame src is null");
        return;
    }
    shader->useProgram();
    //定点坐标 坐标归一
    vertexArray->setArray(VertexUtil::create(config->width, config->height, frame->frame, vertexArray->array));
    vertexArray->setVertexAttribPointer(shader->aPositionLocation);

    //src纹理坐标 坐标归一
    float* array = genSrcCoordsArray(srcArray->array, frame->frame->w, frame->frame->h, src->bitmapWidth, src->bitmapHeight, src->fitType);
    srcArray->setArray(array);
    srcArray->setVertexAttribPointer(shader->aTextureSrcCoordinatesLocation);

    //绑定src纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src->srcTextureId);
    glUniform1i(shader->uTextureSrcUnitLocation, 0);

    //mask纹理 只有r通道为透明值
    maskArray->setArray(TexCoordsUtil::create(config->videoWidth, config->videoHeight, frame->mFrame, maskArray->array));
    if (frame->mt == 90) {
        maskArray->setArray(TexCoordsUtil::rotate90(maskArray->array));
    }

    maskArray->setVertexAttribPointer(shader->aTextureMaskCoordinatesLocation);
    //绑定mask纹理
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, videoTextureId);
    glUniform1i(shader->uTextureMaskUnitLocation, 1);

    //属性处理
    glUniform1i(shader->uIsFillLocation, 0);
    glUniform4f(shader->uColorLocation, 0, 0, 0, 0);
    //启动混合
    glEnable(GL_BLEND);
    //基于alpha通道的半透明混合函数
    //void glBlendFuncSeparate(GLenum srcRGB,
    //     GLenum dstRGB,
    //     GLenum srcAlpha,
    //     GLenum dstAlpha);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    //绘制
    glDrawArrays(GL_TRIANGLE_STRIP,0, 4);
    //关闭混合
    glDisable(GL_BLEND);
}

void EvaMixRender::release(GLuint textureId) {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
    }
}

/**
 *
 * @param array 顶点数据
 * @param fw 帧宽
 * @param fh 帧高
 * @param sw 元素宽
 * @param sh 元素高
 * @param fitType 缩放模型
 * @return
 */
float *EvaMixRender::genSrcCoordsArray(float *array, int fw, int fh, int sw, int sh,
                                       EvaSrc::FitType fitType) {
    float* srcArray = nullptr;
    if (fitType == EvaSrc::FitType::CENTER_FULL) { //aspectFill
        if (fw ==0 || fh == 0) {
            //中心对齐，不拉伸
            int gw = (sw - fw) / 2;
            int gh = (sh - fh) / 2;
            srcArray = TexCoordsUtil::create(sw, sh, new PointRect(gw, gh, fw, fh), array);
        } else { //centerCrop
            float fScale = float(fw) / fh;
            float sScale = float(sw) / sh;
            PointRect *srcRect;
            if (fScale > sScale) {
                int w = sw;
                int x = 0;
                int h = int(sw / fScale);
                int y = (sh - h) / 2;
                srcRect = new PointRect(x, y, w, h);
            } else {
                int h = sh;
                int y = 0;
                int w = int(sh * fScale);
                int x = (sw - w) / 2;
                srcRect = new PointRect(x, y, w, h);
            }
            srcArray = TexCoordsUtil::create(sw, sh, srcRect, array);
        }
    } else if (fitType == EvaSrc::FitType::CENTER_FIT) {  //aspectFit
        if (fw ==0 || fh == 0) {
            //中心对齐，不拉伸
            int gw = (sw - fw) / 2;
            int gh = (sh - fh) / 2;
            srcArray = TexCoordsUtil::create(sw, sh, new PointRect(gw, gh, fw, fh), array);
        } else {
            float fScale = float(fw) / fh;
            float sScale = float(sw) / sh;
            PointRect *srcRect;
            if (fScale < sScale) {
                int w = sw;
                int x = 0;
                int h = int(sw / fScale);
                int y = (sh - h) / 2;
                srcRect = new PointRect(x, y, w, h);
            } else {
                int h = sh;
                int y = 0;
                int w = int(sh * fScale);
                int x = (sw - w) / 2;
                srcRect = new PointRect(x, y, w, h);
            }
            srcArray = TexCoordsUtil::create(sw, sh, srcRect, array);
        }
    } else { //scaleFill
        srcArray = TexCoordsUtil::create(fw, fh, new PointRect(0, 0, fw, fh), array);
    }
    return srcArray;
}

float *EvaMixRender::transColor(int color) {
    auto* argb = new float[4];
    unsigned int c = color;
    argb[0] = float((c >> 24) & 0x000000ff) / 255.0;
    argb[1] = float((c >> 16) & 0x000000ff) / 255.0;
    argb[2] = float((c >> 8) & 0x000000ff) / 255.0;
    argb[3] = float(c & 0x000000ff) / 255.0;
    return argb;
}

//转换RGB颜色 color string转为color RGB色值
float *EvaMixRender::transColor(std::string color) {
    if (color == "") {
        auto* argb = new float[4]{1.0f};
        return argb;
    }
//    int num = std::stoi(color, NULL, 16);
//    int num = std::stoi(color, nullptr, 16);
//    auto* argb = new float[4];
//    argb[0] = float((num >> 24) & 0xff) / 255.0;
//    argb[0] = float((num >> 16) & 0xff) / 255.0;
//    argb[0] = float((num >> 8) & 0xff) / 255.0;
//    argb[0] = float(num & 0xff) / 255.0;

    if (color[0] == '#') {
        color.erase(0, 1);
    }
    unsigned long num = stoul(color, nullptr,16);
    auto* argb = new float[4];
    argb[0] = float((num >> 24) & 0xff) / 255.0;
    argb[1] = float((num >> 16) & 0xff) / 255.0;
    argb[2] = float((num >> 8) & 0xff) / 255.0;
    argb[3] = float(num & 0xff) / 255.0;
    return argb;
}
