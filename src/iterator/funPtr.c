/*
* @Author: liu hongbin
* @Date:   2020-06-01 13:45:26
* @Last Modified by:   lhb8125
* @Last Modified time: 2020-06-01 13:57:11
*/

#include "funPtr.hpp"

// 基于面的循环
void spMV(ArrayArray &topo,
	ArrayArray &dataSet_edge, ArrayArray &dataSet_vertex)
{
	scalar *A = accessArray(dataSet_edge, 0);
	scalar *x = accessArray(dataSet_vertex, 0);
	scalar *b = accessArray(dataSet_vertex, 1);
	for (int i = 0; i < topo.size(); ++i)
	{
		label left  = topo[i][0];
		label right = topo[i][1];
		x[left] += A[i]*b[right];
		x[right] += A[i]*b[left];
	}
}

// 基于体的循环
void integration(ArrayArray &topo,
	ArrayArray &dataSet_edge, ArrayArray &dataSet_vertex)
{
	scalar *U = accessArray(dataSet_vertex, 0);
	scalar *flux = accessArray(dataSet_edge, 1);
	int dim_U = getArrayDim(dataSet_vertex, 0);
	int dim_flux = getArrayDim(dataSet_edge, 1);
	for (int i = 0; i < topo&size(); ++i)
	{
		int size = topo[i].size();
		// 遍历体单元周围所有面
		for (int j = 0; j < size; ++j)
		{
			// x方向
			U[i*dim_U+0] += coef*flux[topo[i][j]*dim_flux+0];
			// y方向
			U[i*dim_U+1] += coef*flux[topo[i][j]*dim_flux+1];
			// z方向
			U[i*dim_U+2] += coef*flux[topo[i][j]*dim_flux+2];
		}
	}
}