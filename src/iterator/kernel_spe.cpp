#include "kernel_spe.hpp"
#include "unat/iterator.hpp"
#include "kernel_slave.h"
#include "resource.h"
#include "unat/topology.hpp"
#include "unat/rowSubsectionIterator.hpp"

void spMV_spe(Region& reg, Word A, Word x, Word b, long pi, StructS s, const int * arr)
{
    Field<Cell, ScalarEle<scalar> > &pre_fieldx
    	= reg.getField<Cell, ScalarEle<scalar> >(x);
    Field<Cell, ScalarEle<scalar> > &pre_fieldb
    	= reg.getField<Cell, ScalarEle<scalar> >(b);
    Field<Face, ScalarEle<scalar> > &pre_fieldA
    	= reg.getField<Face, ScalarEle<scalar> >(A);

    DataSet dataSet_vertex, dataSet_edge;
    initDataSet(&dataSet_edge, pre_fieldA.getSize());
    initDataSet(&dataSet_vertex, pre_fieldx.getSize());

    pushArrayToDataSet(&dataSet_edge,
        pre_fieldA.getDim(),
        pre_fieldA.getLocalElement()->length(),
        0, 
        pre_fieldA.getLocalData()); 
    pushArrayToDataSet(&dataSet_vertex,
        pre_fieldx.getDim(),
        pre_fieldx.getLocalElement()->length(),
        0,
        pre_fieldx.getLocalData()); 
    pushArrayToDataSet(&dataSet_vertex,
        pre_fieldb.getDim(),
        pre_fieldb.getLocalElement()->length(),
        1,
        pre_fieldb.getLocalData()); 

	ParaSet paraData;
	constructParaSet(&paraData, 3, &pi, sizeof(pi), &s, sizeof(s), arr, 10*sizeof(*arr));

    /* 根据数据集类型选择拓扑 */ \
    UTILITY::ArrayArray<label> topo = reg.getMesh().getBlockTopology().getFace2Cell(); \
    topo.num = reg.getMesh().getTopology().getInnFacesNum(); \
    /* 创建UNAT拓扑 */ \
    UNAT::Topology* UNATTopo = UNAT::Topology::constructTopology(topo, LDU); \
    /* 封装耦合算子 */ \
    coupledOperator_new *opt \
        = (coupledOperator_new*)malloc(sizeof(coupledOperator_new)); \
    opt->dataSet_edge = &dataSet_edge; \
    opt->dataSet_vertex = &dataSet_vertex; \
    opt->fun_slave = slave_spMV_skeleton; \
    /* 生成UNAT迭代器 */ \
    static UNAT::RowSubsectionIterator *rssIter = new UNAT::RowSubsectionIterator(opt, \
     *UNATTopo, E2V); \
    /* 调用UNAT迭代器计算，UNAT函数指针需要进行改造 */ \
    rssIter->edge2VertexIteration(&paraData, opt, 1);  \
}

void integration_spe(Region& reg, Word flux, Word U)
{
    Field<Cell, ScalarEle<scalar> > &pre_fnU = reg.getField<Cell, ScalarEle<scalar> >(U);
    Field<Face, ScalarEle<scalar> > &pre_fnFlux = reg.getField<Face, ScalarEle<scalar> >(flux);	

    DataSet dataSet_vertex, dataSet_edge;
    initDataSet(&dataSet_edge, pre_fnFlux.getSize());
    initDataSet(&dataSet_vertex, pre_fnU.getSize());

    pushArrayToDataSet(&dataSet_edge,
        pre_fnFlux.getDim(),
        pre_fnFlux.getLocalElement()->length(),
        0, 
        pre_fnFlux.getLocalData()); 
    pushArrayToDataSet(&dataSet_vertex,
        pre_fnU.getDim(),
        pre_fnU.getLocalElement()->length(),
        1,
        pre_fnU.getLocalData()); 

	ParaSet paraData;
	constructParaSet(&paraData, 0);

    /* 根据数据集类型选择拓扑 */ \
    UTILITY::ArrayArray<label> topo = reg.getMesh().getBlockTopology().getFace2Cell(); \
    topo.num = reg.getMesh().getTopology().getInnFacesNum(); \
    /* 创建UNAT拓扑 */ \
    UNAT::Topology* UNATTopo = UNAT::Topology::constructTopology(topo, LDU); \
    /* 封装耦合算子 */ \
    coupledOperator_new *opt \
        = (coupledOperator_new*)malloc(sizeof(coupledOperator_new)); \
    opt->dataSet_edge = &dataSet_edge; \
    opt->dataSet_vertex = &dataSet_vertex; \
    opt->fun_slave = slave_integration_skeleton; \
    /* 生成UNAT迭代器 */ \
    static UNAT::RowSubsectionIterator *rssIter = new UNAT::RowSubsectionIterator(opt, \
     *UNATTopo, E2V); \
    /* 调用UNAT迭代器计算，UNAT函数指针需要进行改造 */ \
    rssIter->edge2VertexIteration(&paraData, opt, 1);  \
}

void calcLudsFcc_spe(Region& reg, Word massFlux, Word cellx, Word fcc, Word facex, Word rface0, Word rface1, Word S)
{
	Field<Face, ScalarEle<scalar> >& pre_fMassFlux
		= reg.getField<Face, ScalarEle<scalar> >(massFlux);
	Field<Cell, ScalarEle<scalar> >& pre_fCellx
		= reg.getField<Cell, ScalarEle<scalar> >(cellx);
	Field<Face, ScalarEle<scalar> >& pre_fFcc
		= reg.getField<Face, ScalarEle<scalar> >(fcc);
	Field<Face, ScalarEle<scalar> >& pre_fFacex
		= reg.getField<Face, ScalarEle<scalar> >(facex);
	Field<Face, ScalarEle<scalar> >& pre_fRface0
		= reg.getField<Face, ScalarEle<scalar> >(rface0);
	Field<Face, ScalarEle<scalar> >& pre_fRface1
		= reg.getField<Face, ScalarEle<scalar> >(rface1);
	Field<Cell, ScalarEle<scalar> >& pre_fS
		= reg.getField<Cell, ScalarEle<scalar> >(S);

	Array<Field<Cell, ScalarEle<scalar> >*> cellFieldList;
	Array<Field<Face, ScalarEle<scalar> >*> faceFieldList;
	faceFieldList.push_back(&pre_fMassFlux);
	faceFieldList.push_back(&pre_fFcc);
	faceFieldList.push_back(&pre_fFacex);
	faceFieldList.push_back(&pre_fRface0);
	faceFieldList.push_back(&pre_fRface1);
	cellFieldList.push_back(&pre_fCellx);
	cellFieldList.push_back(&pre_fS);

	Array<label32> cellInoutList, faceInoutList;
	faceInoutList.push_back(0);
	faceInoutList.push_back(1);
	faceInoutList.push_back(0);
	faceInoutList.push_back(1);
	faceInoutList.push_back(1);
	cellInoutList.push_back(0);
	cellInoutList.push_back(1);

	constructParaSet(&paraData, 0);
	prepareField(reg, slave_calcLudsFcc_skeleton,
		faceInoutList, cellInoutList,
		faceFieldList, cellFieldList);
}


