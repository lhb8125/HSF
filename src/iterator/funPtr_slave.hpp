#ifndef FUNPTR_SLAVE_CPP
#define FUNPTR_SLAVE_CPP


#include "mpi.h"
#include "utilities.hpp"
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "cgnslib.h"

void spMV_pre_slave(Region& reg);
// void spMV_run_slave(Region& reg, ArrayArray<label>& face_2_cell,
//     label n_face_i, label n_face_b, label n_face, label n_cell);

#endif