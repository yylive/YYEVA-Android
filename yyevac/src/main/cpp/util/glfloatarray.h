//
// Created by asus on 2022/4/17.
//

//#include <GLES3/gl3.h>
//#include <GLES3/gl3ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#ifndef YYEVA_GLFLOATARRAY_H
#define YYEVA_GLFLOATARRAY_H

class GlFloatArray {
public:
    GlFloatArray();
    ~GlFloatArray();
    void setArray(float array[]);
    void setVertexAttribPointer(GLuint attributeLocation);
    GLfloat array[8];

private:
    char* floatBuffer;
};
#endif