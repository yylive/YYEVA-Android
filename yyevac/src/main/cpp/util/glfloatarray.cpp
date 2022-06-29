//
// Created by asus on 2022/4/17.
//

#include "glfloatarray.h"

GlFloatArray::GlFloatArray() {

}

GlFloatArray::~GlFloatArray() {

}

void GlFloatArray::setArray(GLfloat* a) {
    *array = *a;
}
void GlFloatArray::setVertexAttribPointer(GLuint attributeLocation) {
    glVertexAttribPointer(attributeLocation, 2, GL_FLOAT, GL_FALSE, 0, array);
    glEnableVertexAttribArray(attributeLocation);
}
