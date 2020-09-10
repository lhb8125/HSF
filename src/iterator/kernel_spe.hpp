#ifndef KERNEL_SPE_H
#define KERNEL_SPE_H
#include "region.hpp"
#include "field.hpp"
#include "utilities.h"

using namespace HSF;


void spMV_spe(Region& reg, Word A, Word x, Word b, long pi, StructS s, const int * arr);

void integration_spe(Region& reg, Word flux, Word U);

void calcLudsFcc_spe(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex, Word rface0, Word rface1, Word S);
;

#endif
