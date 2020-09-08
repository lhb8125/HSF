#ifndef HSF_FUNPTR_HOST_CPP
#define HSF_FUNPTR_HOST_CPP


#include "region.hpp"
#include "mpi.h"
#include "utilities.h"
// #include "loadBalancer.hpp"
// #include "parameter.hpp"
// #include "cgnslib.h"
#include "hsfMacro.h"

using namespace HSF;

// void spMV(Region& reg, Word A, Word x, Word b,
    // const label pi, const S s, const label32* arr);
void spMV(Region& reg, Word A, Word x, Word b,
    const label pi, const StructS s, const label32* arr);
void spMV_data(Region& reg, label n_face, label n_cell,
    label pi, StructS s, label32* arr);

void integration_data(Region& reg, label n_face, label n_cell);
void integration(Region& reg, Word flux, Word U);

void calcLudsFcc_data(Region& reg, label n_face, label n_cell);
void calcLudsFcc(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex,
    Word rface0, Word rface1, Word S);

#endif
