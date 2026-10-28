//
// Created by asus on 2022/4/17.
//
#include <string>
//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef YYEVA_SHADERUTIL_H
#define YYEVA_SHADERUTIL_H

using namespace std;
class ShaderUtil {
public:
    static GLuint createProgram(string vertexSource, string fragmentSource);

    static GLuint createProgram(char* vertexSource, char* fragmentSource);

    static GLuint compileShader(GLuint shaderType, const char* shaderSource);

    static GLuint createAndLinkProgram(GLuint vertexShaderHandle, GLuint fragmentShaderHandle);
};

#endif