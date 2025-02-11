//
// Created by zengjiale on 2022/4/20.
//

#include "mixshader.h"

yyeva::MixShader::MixShader() {
    char VERTEX[] = R"(#version 310 es
            in vec4 a_Position;
            in vec2 a_TextureSrcCoordinates;
            in vec2 a_TextureMaskCoordinates;
            out vec2 v_TextureSrcCoordinates;
            out vec2 v_TextureMaskCoordinates;
            void main() {
                v_TextureSrcCoordinates = a_TextureSrcCoordinates;
                v_TextureMaskCoordinates = a_TextureMaskCoordinates;
                gl_Position = a_Position;
            }
    )";
    char FRAGMENT[] = R"(#version 310 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        uniform sampler2D u_TextureSrcUnit;
        uniform samplerExternalOES u_TextureMaskUnit;
        in vec2 v_TextureSrcCoordinates;
        in vec2 v_TextureMaskCoordinates;
        out vec4 fragColor;

        void main () {
            vec4 srcRgba = texture(u_TextureSrcUnit, v_TextureSrcCoordinates);
            vec4 maskRgba = texture(u_TextureMaskUnit, v_TextureMaskCoordinates);
            fragColor = srcRgba * maskRgba.r;
        }
    )";
    program = ShaderUtil::createProgram(VERTEX, FRAGMENT);
    uTextureSrcUnitLocation = glGetUniformLocation(program, U_TEXTURE_SRC_UNIT);
    uTextureMaskUnitLocation = glGetUniformLocation(program, U_TEXTURE_MASK_UNIT);
//    //是否填充字体颜色
//    uIsFillLocation = glGetUniformLocation(program, U_IS_FILL);
//    //填充颜色
//    uColorLocation = glGetUniformLocation(program, U_COLOR);

    aPositionLocation = glGetAttribLocation(program, A_POSITION);
    aTextureSrcCoordinatesLocation = glGetAttribLocation(program, A_TEXTURE_SRC_COORDINATES);
    aTextureMaskCoordinatesLocation = glGetAttribLocation(program, A_TEXTURE_MASK_COORDINATES);
}

yyeva::MixShader::~MixShader() {
    glDeleteProgram(program);
}

void yyeva::MixShader::useProgram() {
    glUseProgram(program);
}
