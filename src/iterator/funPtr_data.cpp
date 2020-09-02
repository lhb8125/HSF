/**
* @file: funPtr_data.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 11:04:42
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-23 15:42:43
*/

#include "funPtr_host.hpp"

void integration_data(Region& reg, label n_face, label n_cell)
{
    srand((int)time(0));
    scalar *U_arr     = new scalar[n_cell];
    scalar *U_arr_spe = new scalar[n_cell];
    scalar *flux_arr  = new scalar[n_face];

    for (int i = 0; i < n_face; ++i)
    {
        flux_arr[i] = (scalar)(rand()%100)/100;
    }
    for (int i = 0; i < n_cell; ++i)
    {
        U_arr[i] = (scalar)(rand()%100)/100;
        U_arr_spe[i] = U_arr[i];
    }

    Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();
    Field<scalar> *fU     = new Field<scalar>("cell", 1, n_cell, U_arr, patchTab);
    Field<scalar> *fU_spe = new Field<scalar>("cell", 1, n_cell, U_arr_spe, patchTab);
    Field<scalar> *ff     = new Field<scalar>("face", 1, n_face, flux_arr, patchTab);
    reg.addField<scalar>("U", fU);
    reg.addField<scalar>("U_spe", fU_spe);
    reg.addField<scalar>("flux", ff);
}

void spMV_data(Region& reg, label n_face, label n_cell,
    label pi, StructS s, label32* arr)
{
    srand((int)time(0));
    scalar *A_arr     = new scalar[n_face];
    scalar *x_arr     = new scalar[n_cell];
    scalar *b_arr_spe = new scalar[n_cell];
    scalar *b_arr     = new scalar[n_cell];

    for (int i = 0; i < n_face; ++i)
    {
        A_arr[i] = (scalar)(rand()%100)/100;
    }
    for (int i = 0; i < n_cell; ++i)
    {
        x_arr[i]     = (scalar)(rand()%100)/100;
        b_arr[i]     = (scalar)(rand()%100)/100;
        b_arr_spe[i] = b_arr[i];
    }

    Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();
    Field<scalar> *fA     = new Field<scalar>("face", 1, n_face, A_arr,     patchTab);
    Field<scalar> *fx     = new Field<scalar>("cell", 1, n_cell, x_arr,     patchTab);
    Field<scalar> *fb     = new Field<scalar>("cell", 1, n_cell, b_arr,     patchTab);
    Field<scalar> *fb_spe = new Field<scalar>("cell", 1, n_cell, b_arr_spe, patchTab);
    reg.addField<scalar>("A",     fA);
    reg.addField<scalar>("x",     fx);
    reg.addField<scalar>("b",     fb);
    reg.addField<scalar>("b_spe", fb_spe);

    pi = 3;
    s.a = 3;
    s.b = 2.0;
    s.c = 1200;
    arr[0] = 11;
    arr[1] = 111;
}

void calcLudsFcc_data(Region& reg, label n_face, label n_cell)
{
    srand((int)time(0));
    scalar *mass_arr   = new scalar[n_face];
    scalar *cellx_arr  = new scalar[n_cell];
    scalar *facex_arr  = new scalar[n_face];

    scalar *fcc_arr    = new scalar[n_face];
    scalar *rface0_arr = new scalar[n_face];
    scalar *rface1_arr = new scalar[n_face];
    scalar *S_arr      = new scalar[n_cell];

    scalar *fcc_arr_spe    = new scalar[n_face];
    scalar *rface0_arr_spe = new scalar[n_face];
    scalar *rface1_arr_spe = new scalar[n_face];
    scalar *S_arr_spe      = new scalar[n_cell];

    for (int i = 0; i < n_face; ++i)
    {
        mass_arr[i] = (scalar)(rand()%100-50)/100;
        facex_arr[i] = (scalar)(rand()%100)/100;
        fcc_arr[i] = (scalar)(rand()%100)/100;
        rface0_arr[i] = (scalar)(rand()%100)/100;
        rface1_arr[i] = (scalar)(rand()%100)/100;
        fcc_arr_spe[i] = fcc_arr[i];
        rface0_arr_spe[i] = rface0_arr[i];
        rface1_arr_spe[i] = rface1_arr[i];
    }
    for (int i = 0; i < n_cell; ++i)
    {
        cellx_arr[i] = (scalar)(rand()%100)/100;
        S_arr[i]     = (scalar)(rand()%100)/100;
        S_arr_spe[i] = S_arr[i];
    }

    Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();
    Field<scalar> *fmass     = new Field<scalar>("face", 1, n_face, mass_arr,     patchTab);
    Field<scalar> *ffacex    = new Field<scalar>("face", 1, n_face, facex_arr,     patchTab);
    Field<scalar> *ffcc      = new Field<scalar>("face", 1, n_face, fcc_arr,     patchTab);
    Field<scalar> *frface0   = new Field<scalar>("face", 1, n_face, rface0_arr,     patchTab);
    Field<scalar> *frface1   = new Field<scalar>("face", 1, n_face, rface1_arr,     patchTab);
    Field<scalar> *ffcc_s    = new Field<scalar>("face", 1, n_face, fcc_arr_spe,     patchTab);
    Field<scalar> *frface0_s = new Field<scalar>("face", 1, n_face, rface0_arr_spe,     patchTab);
    Field<scalar> *frface1_s = new Field<scalar>("face", 1, n_face, rface1_arr_spe,     patchTab);
    Field<scalar> *fcellx    = new Field<scalar>("cell", 1, n_cell, cellx_arr,     patchTab);
    Field<scalar> *fS        = new Field<scalar>("cell", 1, n_cell, S_arr,     patchTab);
    Field<scalar> *fS_s      = new Field<scalar>("cell", 1, n_cell, S_arr_spe,     patchTab);

    reg.addField<scalar>("massFlux", fmass);
    reg.addField<scalar>("facex",    ffacex);
    reg.addField<scalar>("cellx",    fcellx);
    reg.addField<scalar>("fcc",      ffcc);
    reg.addField<scalar>("rface0",   frface0);
    reg.addField<scalar>("rface1",   frface1);
    reg.addField<scalar>("fcc_s",    ffcc_s);
    reg.addField<scalar>("rface0_s", frface0_s);
    reg.addField<scalar>("rface1_s", frface1_s);
    reg.addField<scalar>("S", fS);
    reg.addField<scalar>("S_s", fS_s);
}
