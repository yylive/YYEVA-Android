//
// Created by zengjiale on 2023/12/21.
//

#ifndef YYEVA_EASY_H
#define YYEVA_EASY_H

#ifndef SAFE_DELETE
#define SAFE_DELETE(ptr)   \
    do {                   \
        if (ptr != nullptr) { \
            delete ptr;    \
            ptr = nullptr;    \
        }                  \
    } while (0)
#endif

#endif //YYEVA_EASY_H
