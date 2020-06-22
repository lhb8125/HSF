/**
* @file: funPtr_host.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 11:04:42
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-23 15:42:43
*/

#include "funPtr_host.hpp"
void spMV_test(Region& reg, ArrayArray<label>& face_2_cell,
    label n_face_i, label n_face_b, label n_face, label n_cell)
{
    Field<scalar> &A = reg.getField<scalar>("face", "A");
    Field<scalar> &x = reg.getField<scalar>("cell", "x");
    Field<scalar> &b = reg.getField<scalar>("cell", "b");
    // printf("n_face: %d, n_face_i: %d, n_face_b: %d, n_cell: %d\n",
        // n_face, n_face_i, n_face_b, n_cell);

    for (int i = 0; i < n_face_i; ++i)
    {
        label row = face_2_cell[i][0];
        label col = face_2_cell[i][1];
        b[col][0] += A[i][0]*x[row][0];
        b[row][0] = A[i][0]*x[col][0];
    }
}

// void integration_test(Region& reg, ArrayArray<label>& face_2_cell,
//     label n_face_i, label n_face_b, label n_face, label n_cell)
// {
//     Field<scalar> &flux = reg.getField<scalar>("face", "flux");
//     Field<scalar> &U    = reg.getField<scalar>("cell", "U");
//     // printf("n_face: %d, n_face_i: %d, n_face_b: %d, n_cell: %d\n",
//         // n_face, n_face_i, n_face_b, n_cell);

//     for (int i = 0; i < n_face_i; ++i)
//     {
//         label row = face_2_cell[i][0];
//         label col = face_2_cell[i][1];
//         U[row][0] += flux[i][0];
//         U[row][1] += flux[i][1];
//         U[row][2] += flux[i][2];
//         U[col][0] -= flux[i][0];
//         U[col][1] -= flux[i][1];
//         U[col][2] -= flux[i][2];
//     }
// }
