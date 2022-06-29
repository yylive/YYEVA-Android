//
// Created by zengjiale on 2022/4/20.
//

#include "string"
#include "src/main/cpp/util/shaderutil.h"

#ifndef YYEVA_MIXSHADER_H
#define YYEVA_MIXSHADER_H

using namespace std;
class MixShader {
private:
    // Uniform constants
    const GLchar *U_TEXTURE_SRC_UNIT = "u_TextureSrcUnit";
    const GLchar *U_TEXTURE_MASK_UNIT = "u_TextureMaskUnit";
    const GLchar *U_IS_FILL = "u_isFill";
    const GLchar *U_COLOR = "u_Color";

    // Attribute constants
    const GLchar *A_POSITION = "a_Position";
    const GLchar *A_TEXTURE_SRC_COORDINATES = "a_TextureSrcCoordinates";
    const GLchar *A_TEXTURE_MASK_COORDINATES = "a_TextureMaskCoordinates";

public:
    GLuint program;
    GLint uTextureSrcUnitLocation;
    GLint uTextureMaskUnitLocation;
    GLint uIsFillLocation;
    GLint uColorLocation;

    GLint aPositionLocation;
    GLint aTextureSrcCoordinatesLocation;
    GLint aTextureMaskCoordinatesLocation;


    MixShader();
    ~MixShader();
    void useProgram();
};


#endif //YYEVA_MIXSHADER_H
