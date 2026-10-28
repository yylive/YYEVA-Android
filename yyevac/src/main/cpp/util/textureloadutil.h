//
// Created by zengjiale on 2022/4/21.
//
#pragma once
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <android/bitmap.h>
#include <util/stb_image_write.h>
#include <bean/evasrc.h>
#include <EGL/egl.h>
#include <util/elog.h>

class TextureLoadUtil {
public:
    static GLuint loadTexture(unsigned char* bitmap, AndroidBitmapInfo* info);
    static GLuint loadTexture(shared_ptr<EvaSrc> src);
    static void releaseTexture(GLuint textureId);
};

