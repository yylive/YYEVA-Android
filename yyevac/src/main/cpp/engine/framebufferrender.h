//
// Created by asus on 2022/10/24.
//

#ifndef YYEVA_FRAMEBUFFERRENDER_H
#define YYEVA_FRAMEBUFFERRENDER_H
#include <engine/irender.h>
#include <util/shaderutil.h>
#include <util/glfloatarray.h>
#include <util/vertexutil.h>
#include <util/texcoordsutil.h>
#include <util/textureloadutil.h>

#define LOG_TAG "FramebufferRender"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
class FramebufferRender : public IRender {
public:
    FramebufferRender();
    ~FramebufferRender();

    void initRender() override;

    void renderFrame(int frameIndex) override;

    void clearFrame() override;

    void destroyRender() override;

    void setAnimeConfig(EvaAnimeConfig *config) override;

    void updateViewPort(int width, int height) override;

    GLuint getExternalTexture() override;

    void enableFramebuffer();

    void disableFrameBuffer();

    void releaseTexture() override;

    void swapBuffers() override;

private:
    shared_ptr<GlFloatArray> vertexArray;
    shared_ptr<GlFloatArray> rgbaArray;

    GLuint shaderProgram;
    //顶点位置
    GLint uTextureLocation;
    //纹理位置
    GLint positionLocation;
    //纹理位置
    GLint textureLocation;

    int surfaceWidth = 0;
    int surfaceHeight = 0;

    GLuint fboId = -1;
    GLuint fboTextureId = -1;

    GLint wLocation;
    GLint hLocation;

    string filterType;
    bool needWh = false;

    string VERTEX_SHADER;
    string FRAGMENT_SHADER;
};


#endif //YYEVA_FRAMEBUFFERRENDER_H
