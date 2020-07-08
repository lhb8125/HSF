
#include "funPtr_slave.h"

void loop_kernel()
{
}

void loop_skeleton()
{
}

// void spMV_kernel(scalar *A, scalar *x, scalar *b, label n,
//     label dim_A, label dim_x, label dim_b, label *row, label *col)
// {
//     for (int i = 0; i < n; ++i)
//     {
//         b[col[i]*dim_b+0] += A[i*dim_A+0]*x[row[i]*dim_x+0];
//         b[row[i]*dim_b+0] += A[i*dim_A+0]*x[col[i]*dim_x+0];
//     }
// }

// void spMV_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex,
//     label *row, label *col)
// {
//     scalar *x = accessArray(dataSet_vertex, 0);
//     scalar *b = accessArray(dataSet_vertex, 1);
//     scalar *A = accessArray(dataSet_edge, 0);
//     label dim_A = getArrayDims(dataSet_edge, 0);
//     label dim_x = getArrayDims(dataSet_vertex, 0);
//     label dim_b = getArrayDims(dataSet_vertex, 1);
//     // here dim_U = dim_flux
//     label n = getArraySize(dataSet_edge);

//     spMV_kernel(A, x, b, n, dim_A, dim_x, dim_b, row, col);
// }