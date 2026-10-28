//
// Created by Cangwang on 2024/2/23.
//
#pragma once

#include <egl/eglcore.h>
#include <engine/recordrender.h>
#include <engine/fbrender.h>

using namespace std;
namespace yyeva {
    class RecorderController {
    public:
        RecorderController();

        ~RecorderController();

        void init(EGLContext context, ANativeWindow *window, int textureId);

        void setRenderConfig(shared_ptr<EvaAnimeConfig> config);

        void renderFrame(int textureId, uint64_t time);

        void destroy();

    private:
        shared_ptr<EGLCore> eglCore;
        shared_ptr<FbRender> render;
        int width = 0;
        int height = 0;
    };
}
