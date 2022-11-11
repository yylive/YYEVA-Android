//
// Created by zengjiale on 2022/4/21.
//

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <bean/evasrc.h>
#include <EGL/egl.h>
#include <GLES2/gl2ext.h>

#include "util/stb_image_write.h"

#ifndef YYEVA_TEXTURELOADUTIL_H
#define YYEVA_TEXTURELOADUTIL_H


class TextureLoadUtil {
public:
    static GLuint loadTexture(unsigned char* bitmap, AndroidBitmapInfo* info);
    static GLuint loadTexture(EvaSrc* src);
    static void releaseTexture(GLuint textureId);
};


#endif //YYEVA_TEXTURELOADUTIL_H
