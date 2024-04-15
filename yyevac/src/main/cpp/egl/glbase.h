//#include <GLES3/gl3.h>

#pragma once
#include <GLES2/gl2.h>

/**
 * cangwang 2018.12.1
 */
class GLBase{
public:
    GLBase(): mProgram(0), mWidth(0), mHeight(0){

    }

    void resize(int width,int height){
        mWidth = width;
        mHeight = height;
    }

protected:
    GLuint mProgram;
    GLint mWidth;
    GLint mHeight;

private:

};
