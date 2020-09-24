#include "kernel_slave.h"

label** topo;
label** tp;
void spMV_kernel(scalar* fieldA, scalar* fieldx, scalar* fieldb, label n, 
	label dim_fieldA, label dim_fieldx, label dim_fieldb, 
	long pi, StructS s, const int * arr, 
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
        label col = neighbor[i];
        fieldU[row*dim_fieldU+0] += fieldFlux[i*dim_fieldFlux+0];
        fieldU[col*dim_fieldU+0] += fieldFlux[i*dim_fieldFlux+0];
    }
}

void calcLudsFcc_kernel(scalar* fMassFlux, scalar* fCellx, scalar* fFcc, scalar* fFacex, scalar* fRface0, scalar* fRface1, scalar* fS, label nn, 
	label dim_fMassFlux, label dim_fCellx, label dim_fFcc, label dim_fFacex, label dim_fRface0, label dim_fRface1, label dim_fS, 
	
	label *owner, label *neighbor)
{
	double facp;
	double facn;
	int i;
	
	for (i = 0; i < nn; ++i)
    {
        label row = owner[i];
        label col = neighbor[i];
        facp = fMassFlux[i*dim_fMassFlux+0] >= 0.0 ? fMassFlux[i*dim_fMassFlux+0] : 0.0;
        facn = fMassFlux[i*dim_fMassFlux+0] <  0.0 ? fMassFlux[i*dim_fMassFlux+0] : 0.0;
        fFcc[i*dim_fFcc+0] = facn*(fFacex[i*dim_fFacex+0]-fCellx[col*dim_fCellx+0])
                   + facp*(fFacex[i*dim_fFacex+0]-fCellx[row*dim_fCellx+0]);
        fRface0[i*dim_fRface0+0] -= facp;
        fRface1[i*dim_fRface1+0] += facn;
        fS[row*dim_fS+0] += facp;
        fS[col*dim_fS+0] -= facn;
    }
}



void spMV_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex, ParaSet *dataSet_parm, label *row, label *col)
{
	scalar *arr_fieldA = (scalar*)accessDataSet(dataSet_edge, 0);
	scalar *arr_fieldx = (scalar*)accessDataSet(dataSet_vertex, 0);
	scalar *arr_fieldb = (scalar*)accessDataSet(dataSet_vertex, 1);

	label dim_fieldA = accessDataSetDim(dataSet_edge, 0);
	label dim_fieldx = accessDataSetDim(dataSet_vertex, 0);
	label dim_fieldb = accessDataSetDim(dataSet_vertex, 1);

	long const_pi = *(long*)accessParaSet(dataSet_parm, 0);
	StructS const_s = *(StructS*)accessParaSet(dataSet_parm, 1);
	const int * const_arr = (const int *)accessParaSet(dataSet_parm, 2);

	label n = getDataSetSize(dataSet_edge);

	spMV_kernel(arr_fieldA, arr_fieldx, arr_fieldb, n, 
		dim_fieldA, dim_fieldx, dim_fieldb, 
		const_pi, const_s, const_arr, 
		row, col);
}

void integration_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex, ParaSet *dataSet_parm, label *row, label *col)
{
	scalar *arr_fieldFlux = (scalar*)accessDataSet(dataSet_edge, 0);
	scalar *arr_fieldU = (scalar*)accessDataSet(dataSet_vertex, 0);

	label dim_fieldFlux = accessDataSetDim(dataSet_edge, 0);
	label dim_fieldU = accessDataSetDim(dataSet_vertex, 0);


	label n = getDataSetSize(dataSet_edge);

	integration_kernel(arr_fieldFlux, arr_fieldU, n, 
		dim_fieldFlux, dim_fieldU, 
		
		row, col);
}

void calcLudsFcc_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex, ParaSet *dataSet_parm, label *row, label *col)
{
	scalar *arr_fMassFlux = (scalar*)accessDataSet(dataSet_edge, 0);
	scalar *arr_fCellx = (scalar*)accessDataSet(dataSet_vertex, 0);
	scalar *arr_fFcc = (scalar*)accessDataSet(dataSet_edge, 1);
	scalar *arr_fFacex = (scalar*)accessDataSet(dataSet_edge, 2);
	scalar *arr_fRface0 = (scalar*)accessDataSet(dataSet_edge, 3);
	scalar *arr_fRface1 = (scalar*)accessDataSet(dataSet_edge, 4);
	scalar *arr_fS = (scalar*)accessDataSet(dataSet_vertex, 1);

	label dim_fMassFlux = accessDataSetDim(dataSet_edge, 0);
	label dim_fCellx = accessDataSetDim(dataSet_vertex, 0);
	label dim_fFcc = accessDataSetDim(dataSet_edge, 1);
	label dim_fFacex = accessDataSetDim(dataSet_edge, 2);
	label dim_fRface0 = accessDataSetDim(dataSet_edge, 3);
	label dim_fRface1 = accessDataSetDim(dataSet_edge, 4);
	label dim_fS = accessDataSetDim(dataSet_vertex, 1);


	label n = getDataSetSize(dataSet_edge);

	calcLudsFcc_kernel(arr_fMassFlux, arr_fCellx, arr_fFcc, arr_fFacex, arr_fRface0, arr_fRface1, arr_fS, n, 
		dim_fMassFlux, dim_fCellx, dim_fFcc, dim_fFacex, dim_fRface0, dim_fRface1, dim_fS, 
		
		row, col);
}


