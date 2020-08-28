#ifndef FUNPTR_HOST_CPP
#define FUNPTR_HOST_CPP

#include "region.hpp"
#include "mpi.h"
#include "utilities.h"
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "cgnslib.h"

using namespace HSF;

typedef struct
{
    int a;
    double b;
    long c;
}S;

void spMV(Region& reg, Word A, Word x, Word b,
    const label pi, const S s, const label32* arr);

void integration(Region& reg, Word flux, Word U);

#endif
