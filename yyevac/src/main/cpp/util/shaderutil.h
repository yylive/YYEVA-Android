//
// Created by asus on 2022/4/17.
//
#pragma once
#include <string>
//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <util/elog.h>

using namespace std;
class ShaderUtil {
public:
    static GLuint createProgram(string vertexSource, string fragmentSource);

    static GLuint createProgram(char* vertexSource, char* fragmentSource);

    static GLuint compileShader(GLuint shaderType, const char* shaderSource);

    static GLuint createAndLinkProgram(GLuint vertexShaderHandle, GLuint fragmentShaderHandle);
};
