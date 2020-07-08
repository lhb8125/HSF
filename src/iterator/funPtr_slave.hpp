#ifndef FUNPTR_SLAVE_CPP
#define FUNPTR_SLAVE_CPP


#include "mpi.h"
#include "utilities.hpp"
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "cgnslib.h"



void loop_pre(Region& reg, Word A, Word x, Word b);
void prepareField(label32* inoutList, char* funcName, int nPara, ...);
// void spMV_run_slave(Region& reg, ArrayArray<label>& face_2_cell,
//     label n_face_i, label n_face_b, label n_face, label n_cell);

#endif