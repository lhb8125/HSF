#ifndef HSF_HSFMACRO_H
#define HSF_HSFMACRO_H

#include "utilities.h"

#define SOL_DIM 3

#define CHAR_DIM 200

typedef struct
{
    label32 a;
    scalar b;
    label c;
} StructS;


int test = 1;

#define HSF_EPS 1e-12

#define HSF_compareScalar(a, b) \
{\
    if(fabs((a)-(b))>1e-10) \
    { \
        if((a)==0) \
        { \
            if(fabs((b))>1e-10) \
            { \
                printf("Error, %.8f, %.8f\n", \
                            (a), (b)); \
                return false;\
            } \
        } \
        else if(fabs(((a)-(b))/(a))>1e-10) \
        { \
            printf("Error, %.8f, %.8f\n", \
                        (a), (b)); \
            return false; \
        } \
    } \
    return true; \
}


#endif
