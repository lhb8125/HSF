#ifndef KERNEL_SLAVE_H
#define KERNEL_SLAVE_H
#include "unat/iterator.h"
#include "hsfMacro.h"

#ifdef __cplusplus
extern "C"{
#endif


void slave_spMV_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex,
	ParaSet* dataSet_parm, label *row, label *col);
;

void slave_integration_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex,
	ParaSet* dataSet_parm, label *row, label *col);
;

void slave_calcLudsFcc_skeleton(DataSet *dataSet_edge, DataSet *dataSet_vertex,
	ParaSet* dataSet_parm, label *row, label *col);
;
;
#ifdef __cplusplus
}
#endif

#endif
