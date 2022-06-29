//
// Created by zengjiale on 2022/4/18.
//

#include "src/main/cpp/bean/pointrect.h"

#ifndef YYEVA_TEXCOORDSUTIL_H
#define YYEVA_TEXCOORDSUTIL_H

class TexCoordsUtil {
public:

    static float* create(int width, int height, PointRect* rect, float* array);

    static float* rotate90(float* array);

};

#endif //YYEVA_TEXCOORDSUTIL_H
