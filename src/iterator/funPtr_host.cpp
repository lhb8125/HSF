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
    Field<Cell, ScalarEle<scalar> > &fieldx = reg.getField<Cell, ScalarEle<scalar> >(x);
    Field<Cell, ScalarEle<scalar> > &fieldb = reg.getField<Cell, ScalarEle<scalar> >(b);
    Field<Face, ScalarEle<scalar> > &fieldA = reg.getField<Face, ScalarEle<scalar> >(A);
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
    Field<Cell, ScalarEle<scalar> > &fnU = reg.getField<Cell, ScalarEle<scalar> >(U);
    Field<Face, ScalarEle<scalar> > &fnFlux = reg.getField<Face, ScalarEle<scalar> >(flux);
#pragma message("getTopology")
    ArrayArray<label> tp = reg.getTopology<label>(2, &flux, &U);
    label nn = reg.getSize(2, &flux, &U);
    // label nn = reg.getMesh().getTopology().getFacesNum();

#pragma message("compute")
    int i;
    for (i = 0; i < nn; ++i)
    {
        label row = tp[i][0];
        label col = tp[i][1];
        fnU[row][0] += fnFlux[i][0];
        fnU[col][0] += fnFlux[i][0];
    }
}

void calcLudsFcc(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex,
    Word rface0, Word rface1, Word S)
{
#pragma message("getField")
    Field<Face, ScalarEle<scalar> > &fMassFlux = reg.getField<Face, ScalarEle<scalar> >(massFlux);
    Field<Cell, ScalarEle<scalar> > &fCellx    = reg.getField<Cell, ScalarEle<scalar> >(cellx);
    Field<Face, ScalarEle<scalar> > &fFcc      = reg.getField<Face, ScalarEle<scalar> >(fcc);
    Field<Face, ScalarEle<scalar> > &fFacex    = reg.getField<Face, ScalarEle<scalar> >(facex);
    Field<Face, ScalarEle<scalar> > &fRface0   = reg.getField<Face, ScalarEle<scalar> >(rface0);
    Field<Face, ScalarEle<scalar> > &fRface1   = reg.getField<Face, ScalarEle<scalar> >(rface1);
    Field<Cell, ScalarEle<scalar> > &fS        = reg.getField<Cell, ScalarEle<scalar> >(S);
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
