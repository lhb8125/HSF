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
    const label pi, const StructS s, const label32* arr)
{
#pragma message("getField")
    Field<scalar> &fieldA = reg.getField<scalar>(A);
    Field<scalar> &fieldx = reg.getField<scalar>(x);
    Field<scalar> &fieldb = reg.getField<scalar>(b);
#pragma message("getTopology")
    ArrayArray<label> topo = reg.getTopology<label>(3, &A, &x, &b);
    label n = reg.getSize(3, &A, &x, &b);

#pragma message("compute")
#pragma message("arr[10]")
    int i;
    for (i = 0; i < n; ++i)
    {
        label row = topo[i][0];
        label col = topo[i][1];
        fieldb[col][0] += pi*fieldA[i][0]*fieldx[row][0]+arr[0]*s.b;
        fieldb[row][0] -= pi*fieldA[i][0]*fieldx[col][0]+arr[1]*s.c;
    }
}

void integration(Region& reg, Word flux, Word U)
{
#pragma message("getField")
    Field<scalar> &fieldFlux = reg.getField<scalar>(flux);
    Field<scalar> &fieldU    = reg.getField<scalar>(U);
#pragma message("getTopology")
    ArrayArray<label> tp = reg.getTopology<label>(2, &flux, &U);
    label nn = reg.getSize(2, &flux, &U);

#pragma message("compute")
    int i;
    for (i = 0; i < nn; ++i)
    {
        label row = tp[i][0];
        fieldU[row][0] += fieldFlux[i][0];
    }
}

void calcLudsFcc(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex,
    Word rface0, Word rface1, Word S)
{
#pragma message("getField")
    Field<scalar> &fMassFlux = reg.getField<scalar>(massFlux);
    Field<scalar> &fCellx    = reg.getField<scalar>(cellx);
    Field<scalar> &fFcc      = reg.getField<scalar>(fcc);
    Field<scalar> &fFacex    = reg.getField<scalar>(facex);
    Field<scalar> &fRface0   = reg.getField<scalar>(rface0);
    Field<scalar> &fRface1   = reg.getField<scalar>(rface1);
    Field<scalar> &fS   = reg.getField<scalar>(S);
#pragma message("getTopology")
    ArrayArray<label> tp = reg.getTopology<label>(7, &massFlux, &cellx, &fcc, &facex, &rface0, &rface1, &S);
    label nn = reg.getSize(7, &massFlux, &cellx, &fcc, &facex, &rface0, &rface1, &S);


#pragma message("compute")
    scalar facp, facn;
    int i;
    for (i = 0; i < nn; ++i)
    {
        label row = tp[i][0];
        label col = tp[i][1];
        facp = fMassFlux[i][0] >= 0.0 ? fMassFlux[i][0] : 0.0;
        facn = fMassFlux[i][0] <  0.0 ? fMassFlux[i][0] : 0.0;
        fFcc[i][0] = facn*(fFacex[i][0]-fCellx[col][0])
                   + facp*(fFacex[i][0]-fCellx[row][0]);
        fRface0[i][0] -= facp;
        fRface1[i][0] += facn;
        fS[row][0] += facp;
        fS[col][0] -= facn;
    }
}
