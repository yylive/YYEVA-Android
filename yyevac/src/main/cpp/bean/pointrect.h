//
// Created by zengjiale on 2022/4/15.
//

#ifndef YYEVA_POINTRECT_H
#define YYEVA_POINTRECT_H


class PointRect {
public:
    int x;
    int y;
    int w;
    int h;
    PointRect(int x, int y, int w, int h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
};


#endif //YYEVA_POINTRECT_H
