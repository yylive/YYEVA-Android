//
// Created by asus on 2022/4/17.
//
#include "shaderutil.h"


#define LOG_TAG "ShareUtil"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)

GLuint ShaderUtil::createProgram(string vertexSource, string fragmentSource) {
    GLuint vertexShaderHandle = compileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint fragmentShaderHandle = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
    return createAndLinkProgram(vertexShaderHandle, fragmentShaderHandle);
}

GLuint ShaderUtil::createProgram(char* vertexSource, char* fragmentSource) {
    GLuint vertexShaderHandle = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShaderHandle = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    return createAndLinkProgram(vertexShaderHandle, fragmentShaderHandle);
}

GLuint ShaderUtil::compileShader(GLenum shaderType, const char* shaderSource) {
    GLint shaderHandle = glCreateShader(shaderType);
    if (shaderHandle != 0) {
        GLint compiled;
        glShaderSource(shaderHandle, 1,
                       &shaderSource, nullptr);
        shaderSource = nullptr;
        glCompileShader(shaderHandle);
        glGetShaderiv(shaderHandle,GL_COMPILE_STATUS,&compiled);
        if(!compiled){
            GLint infoLen = 0;
            glGetShaderiv(shaderHandle,GL_INFO_LOG_LENGTH,&infoLen);

            if(infoLen >1){
                char *infoLog= (char*)malloc(sizeof(char*) *infoLen);
                glGetShaderInfoLog(shaderHandle,infoLen,NULL,infoLog);
                ELOGE("Error compiling shader:[%s]",infoLog);
                free(infoLog);
            }
            glDeleteShader(shaderHandle);
            return 0;
        }
        return shaderHandle;
    } else {
        GLint infoLen = 0;
        glGetShaderiv(shaderHandle,GL_INFO_LOG_LENGTH,&infoLen);
        if (infoLen > 1) {
            char *infoLog= (char*)malloc(sizeof(char*) *infoLen);
            glGetShaderInfoLog(shaderHandle,infoLen,NULL,infoLog);
            ELOGE("Error create shader:[%s]", infoLog);
            free(infoLog);
        }
        ELOGE("Error create shader");
    }
    glDeleteShader(shaderHandle);
    return 0;
}

GLuint ShaderUtil::createAndLinkProgram(GLuint vertexShaderHandle, GLuint fragmentShaderHandle) {
    GLuint iProgId = glCreateProgram();
    if (iProgId == 0){
        ELOGE("create program failed");
        return 0;
    }
    glAttachShader(iProgId,vertexShaderHandle);
    glAttachShader(iProgId,fragmentShaderHandle);

    //链接shader到程序
    glLinkProgram(iProgId);

    GLint linked;
    glGetProgramiv(iProgId,GL_LINK_STATUS,&linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(iProgId, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char *infoLog = (char *) malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(iProgId, infoLen, nullptr, infoLog);
            ELOGE("loadProgram failed: %s", infoLog);
            free(infoLog);
        }
        glDetachShader(iProgId, vertexShaderHandle);
        glDetachShader(iProgId, fragmentShaderHandle);
        glDeleteShader(vertexShaderHandle);
        glDeleteShader(fragmentShaderHandle);
        glDeleteProgram(iProgId);
        return 0;
    }
//    glDetachShader(iProgId, vertexShaderHandle);
//    glDetachShader(iProgId, fragmentShaderHandle);
    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);
    return iProgId;
}
