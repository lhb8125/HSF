#include "kernel_slave.h"

label** topo;
label** tp;
// void spMV_kernel(scalar* fieldA, scalar* fieldx, scalar* fieldb, label n, 
// 	label dim_fieldA, label dim_fieldx, label dim_fieldb, 
// 	long pi, S s, const int * arr, 
// 	label *owner, label *neighbor)
void spMV_kernel(scalar* fieldA, scalar* fieldx, scalar* fieldb, label n, 
    label dim_fieldA, label dim_fieldx, label dim_fieldb, 
    label pi, label32* arr, StructS s,
    label *owner, label *neighbor)
{
    int i;
	for (i = 0; i < n; ++i)
    {
        label row = owner[i];
        label col = neighbor[i];
        fieldb[col*dim_fieldb+0] += pi*fieldA[i*dim_fieldA+0]*fieldx[row*dim_fieldx+0]+arr[0]*s.b;
        fieldb[row*dim_fieldb+0] -= pi*fieldA[i*dim_fieldA+0]*fieldx[col*dim_fieldx+0]+arr[1]*s.c;
    }
}

void integration_kernel(scalar* fieldFlux, scalar* fieldU, label nn, 
	label dim_fieldFlux, label dim_fieldU, 
	
	label *owner, label *neighbor)
{
    int i;
	for (i = 0; i < nn; ++i)
    {
        label row = owner[i];
        fieldU[row*dim_fieldU+0] += fieldFlux[i*dim_fieldFlux+0];
        // fieldU[row*dim_fieldU+1] += fieldFlux[i*dim_fieldFlux+1];
        // fieldU[row*dim_fieldU+2] += fieldFlux[i*dim_fieldFlux+2];
    }
}

void calcLudsFcc_kernel(scalar* massFlux, scalar* cellx, scalar* fcc, scalar* facex, scalar* rface0, scalar* rface1, scalar* S, label nn, 
    label dim_massFlux, label dim_cellx, label dim_fcc, label dim_facex, label dim_rface0, label dim_rface1, label dim_S,
    label *owner, label *neighbor)
{
    int i;
    scalar facp, facn;
    for (i = 0; i < nn; ++i)
    {
        label row = owner[i];
        label col = neighbor[i];
        facp = massFlux[i*dim_massFlux+0] >= 0.0 ? massFlux[i*dim_massFlux+0] : 0.0;
        facn = massFlux[i*dim_massFlux+0] <  0.0 ? massFlux[i*dim_massFlux+00] : 0.0;
        fcc[i*dim_fcc+0] = facn*(facex[i*dim_facex+0]-cellx[col*dim_cellx+0])
                         + facp*(facex[i*dim_facex+0]-cellx[row*dim_cellx+0]);
        rface0[i*dim_rface0+0] -= facp;
        rface1[i*dim_rface1+0] += facn;
        S[row*dim_S+0]         += facp;
        S[col*dim_S+0]         -= facn;
    }
}


void spMV_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex,
    ParaSet *dataSet_parm, label *row, label *col)
{
	scalar *arr_fieldA = (scalar*)accessDataSet(dataSet_edge, 0);
	scalar *arr_fieldx = (scalar*)accessDataSet(dataSet_vertex, 0);
	scalar *arr_fieldb = (scalar*)accessDataSet(dataSet_vertex, 1);

	label dim_fieldA = accessDataSetDim(dataSet_edge, 0);
	label dim_fieldx = accessDataSetDim(dataSet_vertex, 0);
	label dim_fieldb = accessDataSetDim(dataSet_vertex, 1);

	label const_pi = *(label*)accessParaSet(dataSet_parm, 0);
	label32 * const_arr = (label32*)accessParaSet(dataSet_parm, 1);
    StructS const_s = *(StructS*)accessParaSet(dataSet_parm, 2);

	label n = getDataSetSize(dataSet_edge);

	spMV_kernel(arr_fieldA, arr_fieldx, arr_fieldb, n, 
		dim_fieldA, dim_fieldx, dim_fieldb, 
        const_pi, const_arr, const_s,
		row, col);
}

void integration_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex,
    ParaSet* dataSet_parm, label *row, label *col)
{
	scalar *arr_fieldFlux = (scalar*)accessDataSet(dataSet_edge, 0);
	scalar *arr_fieldU = (scalar*)accessDataSet(dataSet_vertex, 0);

	label dim_fieldFlux = accessDataSetDim(dataSet_edge, 0);
	label dim_fieldU = accessDataSetDim(dataSet_vertex, 0);


	label n = dataSet_edge->arraySize;

	integration_kernel(arr_fieldFlux, arr_fieldU, n, 
		dim_fieldFlux, dim_fieldU, 
		
		row, col);
}

void calcLudsFcc_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex,
    ParaSet* dataSet_parm, label *row, label *col)
{
    scalar *massFlux = (scalar*)accessDataSet(dataSet_edge, 0);
    scalar *cellx    = (scalar*)accessDataSet(dataSet_vertex, 0);
    scalar *fcc      = (scalar*)accessDataSet(dataSet_edge, 1);
    scalar *facex    = (scalar*)accessDataSet(dataSet_edge, 2);
    scalar *rface0   = (scalar*)accessDataSet(dataSet_edge, 3);
    scalar *rface1   = (scalar*)accessDataSet(dataSet_edge, 4);
    scalar *S        = (scalar*)accessDataSet(dataSet_vertex, 1);

    label dim_massFlux = accessDataSetDim(dataSet_edge, 0);
    label dim_cellx    = accessDataSetDim(dataSet_vertex, 0);
    label dim_fcc      = accessDataSetDim(dataSet_edge, 1);
    label dim_facex    = accessDataSetDim(dataSet_edge, 2);
    label dim_rface0   = accessDataSetDim(dataSet_edge, 3);
    label dim_rface1   = accessDataSetDim(dataSet_edge, 4);
    label dim_S        = accessDataSetDim(dataSet_vertex, 1);

    label n = getDataSetSize(dataSet_edge);

    calcLudsFcc_kernel(massFlux, cellx, fcc, facex, rface0, rface1, S, n, 
        dim_massFlux, dim_cellx, dim_fcc, dim_facex, dim_rface0, dim_rface1, dim_S,
        row, col);
}


