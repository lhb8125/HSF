#ifndef FUNPTR_HOST_CPP
#define FUNPTR_HOST_CPP


#include "mpi.h"
#include "utilities.h"
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "cgnslib.h"

typedef struct
{
    int a;
    double b;
    long c;
}S;

void spMV_test(Region& reg, ArrayArray<label>& face_2_cell,
    label n_face_i, label n_face_b, label n_face, label n_cell);

#endif