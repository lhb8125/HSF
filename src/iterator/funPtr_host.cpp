/**
* @file: funPtr_host.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 11:04:42
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-23 15:42:43
*/

#include "funPtr_host.hpp"
void spMV(Region& reg, Word A, Word x, Word b,
    const label pi, const S s, const label32* arr)
{
#pragma message("getField")
    Field<scalar> &fieldA = reg.getField<scalar>(A);
    Field<scalar> &fieldx = reg.getField<scalar>(x);
    Field<scalar> &fieldb = reg.getField<scalar>(b);
#pragma message("getTopology")
    ArrayArray<label> &topo = reg.getTopology<label>(3, &A, &x, &b);
    label n = reg.getSize(3, &A, &x, &b);

#pragma message("compute")
#pragma message("arr[10]")
    for (int i = 0; i < n; ++i)
    {
        label row = topo[i][0];
        label col = topo[i][1];
        fieldb[col][0] += pi*fieldA[i][0]*fieldx[row][0]+s.b/arr[3];
        fieldb[row][0] -= pi*fieldA[i][0]*fieldx[col][0]+s.c/arr[2];
    }
}

void integration(Region& reg, Word flux, Word U)
{
#pragma message("getField")
    Field<scalar> &fieldFlux = reg.getField<scalar>(flux);
    Field<scalar> &fieldU    = reg.getField<scalar>(U);
#pragma message("getTopology")
    ArrayArray<label> &tp = reg.getTopology<label>(3, &flux, &U);
    label nn = reg.getSize(3, &flux, &U);

#pragma message("compute")
    for (int i = 0; i < nn; ++i)
    {
        label row = tp[i][0];
        fieldU[row][0] += fieldFlux[i][0];
        fieldU[row][1] += fieldFlux[i][1];
        fieldU[row][2] += fieldFlux[i][2];
    }
}
