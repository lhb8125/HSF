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

    par_std_out("n_cell: %d, n_face: %d\n",n_cell,n_face);
    ScalarEle<scalar> *U_ele     = new ScalarEle<scalar>[n_cell];
    ScalarEle<scalar> *U_ele_spe = new ScalarEle<scalar>[n_cell];
    ScalarEle<scalar> *flux_ele  = new ScalarEle<scalar>[n_face];
    for (int i = 0; i < n_cell; ++i)
    {
        U_ele[i].setVal(0.0);
        U_ele_spe[i] = U_ele[i];
    }
    for (int i = 0; i < n_face; ++i)
    {
        flux_ele[i].setVal(1.0);
    }

    Communicator &commcator = reg.getCommunicator();


    Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();

    Field<Cell, ScalarEle<scalar> > *fnU
        = new Field<Cell, ScalarEle<scalar> >(n_cell, U_ele, patchTab, commcator);
    Field<Cell, ScalarEle<scalar> > *fnU_spe
        = new Field<Cell, ScalarEle<scalar> >(n_cell, U_ele_spe, patchTab, commcator);
    Field<Face, ScalarEle<scalar> > *fnf
        = new Field<Face, ScalarEle<scalar> >(n_face, flux_ele, patchTab, commcator);
    reg.addField<Cell, ScalarEle<scalar> >("U", fnU);
    reg.addField<Cell, ScalarEle<scalar> >("U_spe", fnU_spe);
    reg.addField<Face, ScalarEle<scalar> >("flux", fnf);

}

void spMV_data(Region& reg, label n_face, label n_cell,
    label pi, StructS s, label32* arr)
{
    srand((int)time(0));
    ScalarEle<scalar> *A_ele     = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *x_ele     = new ScalarEle<scalar>[n_cell];
    ScalarEle<scalar> *b_ele_spe = new ScalarEle<scalar>[n_cell];
    ScalarEle<scalar> *b_ele     = new ScalarEle<scalar>[n_cell];
    for (int i = 0; i < n_cell; ++i)
    {
        b_ele_spe[i] = b_ele[i];
    }

    Communicator &commcator = reg.getCommunicator();

    Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();

    Field<Face, ScalarEle<scalar> > *fieldA
        = new Field<Face, ScalarEle<scalar> >(n_face, A_ele, patchTab, commcator);
    Field<Cell, ScalarEle<scalar> > *fieldx
        = new Field<Cell, ScalarEle<scalar> >(n_cell, x_ele, patchTab, commcator);
    Field<Cell, ScalarEle<scalar> > *fieldb
        = new Field<Cell, ScalarEle<scalar> >(n_cell, b_ele, patchTab, commcator);
    Field<Cell, ScalarEle<scalar> > *fieldb_spe
        = new Field<Cell, ScalarEle<scalar> >(n_cell, b_ele_spe, patchTab, commcator);
    reg.addField<Cell, ScalarEle<scalar> >("x", fieldx);
    reg.addField<Cell, ScalarEle<scalar> >("b", fieldb);
    reg.addField<Cell, ScalarEle<scalar> >("b_spe", fieldb_spe);
    reg.addField<Face, ScalarEle<scalar> >("A", fieldA);

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
    ScalarEle<scalar> *mass_ele       = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *facex_ele      = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *fcc_ele        = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *rface0_ele     = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *rface1_ele     = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *fcc_ele_spe    = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *rface0_ele_spe = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *rface1_ele_spe = new ScalarEle<scalar>[n_face];
    ScalarEle<scalar> *cellx_ele      = new ScalarEle<scalar>[n_cell];
    ScalarEle<scalar> *S_ele          = new ScalarEle<scalar>[n_cell];
    ScalarEle<scalar> *S_ele_spe      = new ScalarEle<scalar>[n_cell];

    Communicator &commcator = reg.getCommunicator();

    for (int i = 0; i < n_face; ++i)
    {
        fcc_ele_spe[i]    = fcc_ele[i];
        rface0_ele_spe[i] = rface0_ele[i];
        rface1_ele_spe[i] = rface1_ele[i];
    }
    for (int i = 0; i < n_cell; ++i)
    {
        S_ele_spe[i] = S_ele[i];
    }

    Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();
    Field<Face, ScalarEle<scalar> > *fmass
        = new Field<Face, ScalarEle<scalar> >(n_face, mass_ele, patchTab,commcator);
    Field<Face, ScalarEle<scalar> > *ffacex
        = new Field<Face, ScalarEle<scalar> >(n_face, facex_ele, patchTab,commcator);
    Field<Face, ScalarEle<scalar> > *ffcc
        = new Field<Face, ScalarEle<scalar> >(n_face, fcc_ele, patchTab,commcator);
    Field<Face, ScalarEle<scalar> > *frface0
        = new Field<Face, ScalarEle<scalar> >(n_face, rface0_ele, patchTab,commcator);
    Field<Face, ScalarEle<scalar> > *frface1
        = new Field<Face, ScalarEle<scalar> >(n_face, rface1_ele, patchTab,commcator);
    Field<Face, ScalarEle<scalar> > *ffcc_s
        = new Field<Face, ScalarEle<scalar> >(n_face, fcc_ele_spe, patchTab,commcator);
    Field<Face, ScalarEle<scalar> > *frface0_s
        = new Field<Face, ScalarEle<scalar> >(n_face, rface0_ele_spe, patchTab,commcator);
    Field<Face, ScalarEle<scalar> > *frface1_s
        = new Field<Face, ScalarEle<scalar> >(n_face, rface1_ele_spe, patchTab,commcator);
    Field<Cell, ScalarEle<scalar> > *fcellx
        = new Field<Cell, ScalarEle<scalar> >(n_cell, cellx_ele, patchTab,commcator);
    Field<Cell, ScalarEle<scalar> > *fS
        = new Field<Cell, ScalarEle<scalar> >(n_cell, S_ele, patchTab,commcator);
    Field<Cell, ScalarEle<scalar> > *fS_s
        = new Field<Cell, ScalarEle<scalar> >(n_cell, S_ele_spe, patchTab,commcator);

    reg.addField<Face, ScalarEle<scalar> >("massFlux", fmass);
    reg.addField<Face, ScalarEle<scalar> >("facex",    ffacex);
    reg.addField<Cell, ScalarEle<scalar> >("cellx",    fcellx);
    reg.addField<Face, ScalarEle<scalar> >("fcc",      ffcc);
    reg.addField<Face, ScalarEle<scalar> >("rface0",   frface0);
    reg.addField<Face, ScalarEle<scalar> >("rface1",   frface1);
    reg.addField<Face, ScalarEle<scalar> >("fcc_s",    ffcc_s);
    reg.addField<Face, ScalarEle<scalar> >("rface0_s", frface0_s);
    reg.addField<Face, ScalarEle<scalar> >("rface1_s", frface1_s);
    reg.addField<Cell, ScalarEle<scalar> >("S",        fS);
    reg.addField<Cell, ScalarEle<scalar> >("S_s",      fS_s);
}
