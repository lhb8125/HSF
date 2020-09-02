#ifndef HSF_KERNEL_SPE_HPP
#define HSF_KERNEL_SPE_HPP

#include "region.hpp"
#include "field.hpp"
#include "utilities.h"
// #include "funPtr_host.hpp"

using namespace HSF;

// void spMV_spe(Region& reg, Word A, Word x, Word b);
void spMV_spe(Region& reg, Word A, Word x, Word b, label pi, StructS s, label32 * arr);

void integration_spe(Region& reg, Word flux, Word U);

void calcLudsFcc_spe(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex, Word rface0, Word rface1, Word S);



#endif
