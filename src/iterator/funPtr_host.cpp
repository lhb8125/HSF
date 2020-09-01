/**
* @file: funPtr_host.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 11:04:42
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-23 15:42:43
*/

#include "funPtr_host.hpp"

extern void integration_pre(Region& reg, Word flux, Word U);

void spMV(Region& reg, Word A, Word x, Word b,
    const label pi, const S s, const label32* arr)
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
    ArrayArray<label> tp = reg.getTopology<label>(2, &flux, &U);
    label nn = reg.getSize(2, &flux, &U);

#pragma message("compute")
    printf("%f\n", fieldU[55][0]);
    for (int i = 0; i < nn; ++i)
    {
        label row = tp[i][0];
        fieldU[row][0] += fieldFlux[i][0];
    }

    printf("%f\n", fieldU[55][0]);
    integration_pre(reg, flux, U);
    printf("%f\n", fieldU[55][0]);
}

void integration_data(Region& reg, label n_face, label n_cell)
{
    srand((int)time(0));
    scalar *U_arr    = new scalar[n_cell];
    scalar *flux_arr = new scalar[n_face];

    for (int i = 0; i < n_face; ++i)
    {
        flux_arr[i] = (scalar)(rand()%100)/100;
    }
    for (int i = 0; i < n_cell; ++i)
    {
        U_arr[i] = (scalar)(rand()%100)/100;
    }

    Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();
    Field<scalar> *fU = new Field<scalar>("cell", 1, n_cell, U_arr, patchTab);
    Field<scalar> *ff = new Field<scalar>("face", 1, n_face, flux_arr, patchTab);
    reg.addField<scalar>("U", fU);
    reg.addField<scalar>("flux", ff);
}
