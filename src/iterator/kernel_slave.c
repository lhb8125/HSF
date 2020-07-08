#include "kernel_slave.h"
label** topo;
void spMV_kernel(scalar** fieldA, scalar** fieldx, scalar** fieldb, label n, 
	label dim_fieldA, label dim_fieldx, label dim_fieldb, 
	long pi, S s, const int * arr, 
	label *owner, label *neighbor)
{
	for (int i = 0; i < n; ++i)
    {
        label row = topo[i][0];
        label col = topo[i][1];
        fieldb[col][0] += pi*fieldA[i][0]*fieldx[row][0]+s.b/arr[3];
        fieldb[row][0] -= pi*fieldA[i][0]*fieldx[col][0]+s.c/arr[2];
    }
}
label** tp;
void integration_kernel(scalar** fieldFlux, scalar** fieldU, label nn, 
	label dim_fieldFlux, label dim_fieldU, 
	
	label *owner, label *neighbor)
{
	for (int i = 0; i < nn; ++i)
    {
        label row = tp[i][0];
        fieldU[row][0] += fieldFlux[i][0];
        fieldU[row][1] += fieldFlux[i][1];
        fieldU[row][2] += fieldFlux[i][2];
    }
}



void spMV_skeleton(DataSet *dataSet_parm, DataSet *dataSet_edge, DataSet *dataSet_vertex, label *row, label *col)
{
	scalar *arr_fieldA = getArrayFromDataSet("A");
	scalar *arr_fieldx = getArrayFromDataSet("x");
	scalar *arr_fieldb = getArrayFromDataSet("b");

	label dim_fieldA = getDimFromDataSet("A");
	label dim_fieldx = getDimFromDataSet("x");
	label dim_fieldb = getDimFromDataSet("b");

	long const_pi = *(long*)getConstFromDataSet(0);
	S const_s = *(S*)getConstFromDataSet(1);
	const int * const_arr = (const int *)getConstFromDataSet(2);

	label n = getSizeFromDataSet(dataSet_edge, dataSet_vertex);

	spMV_kernel(arr_fieldA, arr_fieldx, arr_fieldb, n, 
		dim_fieldA, dim_fieldx, dim_fieldb, 
		const_pi, const_s, const_arr, 
		row, col);
}

void integration_skeleton(DataSet *dataSet_parm, DataSet *dataSet_edge, DataSet *dataSet_vertex, label *row, label *col)
{
	scalar *arr_fieldFlux = getArrayFromDataSet("flux");
	scalar *arr_fieldU = getArrayFromDataSet("U");

	label dim_fieldFlux = getDimFromDataSet("flux");
	label dim_fieldU = getDimFromDataSet("U");


	label n = getSizeFromDataSet(dataSet_edge, dataSet_vertex);

	integration_kernel(arr_fieldFlux, arr_fieldU, n, 
		dim_fieldFlux, dim_fieldU, 
		
		row, col);
}


