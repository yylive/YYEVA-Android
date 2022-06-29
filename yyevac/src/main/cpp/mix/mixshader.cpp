//
// Created by zengjiale on 2022/4/20.
//

#include "mixshader.h"

MixShader::MixShader() {
    char VERTEX[] =  "attribute vec4 a_Position;  \n"
                     "attribute vec2 a_TextureSrcCoordinates;\n"
                     "attribute vec2 a_TextureMaskCoordinates;\n"
                     "varying vec2 v_TextureSrcCoordinates;\n"
                     "varying vec2 v_TextureMaskCoordinates;\n"
                     "void main()\n"
                     "{\n"
                     "    v_TextureSrcCoordinates = a_TextureSrcCoordinates;\n"
                     "    v_TextureMaskCoordinates = a_TextureMaskCoordinates;\n"
                     "    gl_Position = a_Position;\n"
                     "}";

    char FRAGMENT[] = "#extension GL_OES_EGL_image_external : require\n"
                    "precision mediump float; \n"
                    "uniform sampler2D u_TextureSrcUnit;\n"
                    "uniform samplerExternalOES u_TextureMaskUnit;\n"
                    "uniform int u_isFill;\n"
                    "uniform vec4 u_Color;\n"
                    "varying vec2 v_TextureSrcCoordinates;\n"
                    "varying vec2 v_TextureMaskCoordinates;\n"
                    "void main()\n"
                    "{\n"
                    "    vec4 srcRgba = texture2D(u_TextureSrcUnit, v_TextureSrcCoordinates);\n"
                    "    vec4 maskRgba = texture2D(u_TextureMaskUnit, v_TextureMaskCoordinates);\n"
                    "    float isFill = step(0.5, float(u_isFill));\n"
                    "    vec4 srcRgbaCal = isFill * vec4(u_Color.r, u_Color.g, u_Color.b, srcRgba.a) + (1.0 - isFill) * srcRgba;\n"
                    "    gl_FragColor = vec4(srcRgbaCal.r, srcRgbaCal.g, srcRgbaCal.b, srcRgba.a * maskRgba.r);\n"
                    "}";

    program = ShaderUtil::createProgram(VERTEX, FRAGMENT);
    uTextureSrcUnitLocation = glGetUniformLocation(program, U_TEXTURE_SRC_UNIT);
    uTextureMaskUnitLocation = glGetUniformLocation(program, U_TEXTURE_MASK_UNIT);
    //是否填充字体颜色
    uIsFillLocation = glGetUniformLocation(program, U_IS_FILL);
    //填充颜色
    uColorLocation = glGetUniformLocation(program, U_COLOR);

    aPositionLocation = glGetAttribLocation(program, A_POSITION);
    aTextureSrcCoordinatesLocation = glGetAttribLocation(program, A_TEXTURE_SRC_COORDINATES);
    aTextureMaskCoordinatesLocation = glGetAttribLocation(program, A_TEXTURE_MASK_COORDINATES);
}

MixShader::~MixShader() {

}

void MixShader::useProgram() {
    glUseProgram(program);
}
