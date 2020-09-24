#ifndef HSF_RESOURCE_HPP
#define HSF_RESOURCE_HPP

#include "region.hpp"
// #include "funPtr_slave.hpp"
// #include "unat/container.hpp"
#include "unat/iterator.h"
#include "unat/topology.hpp"
#include "unat/rowSubsectionIterator.hpp"

ParaSet paraData;

#define prepareField(reg, funcPtr, \
        faceInoutList, cellInoutList, \
        faceFieldList, cellFieldList) \
{ \
    DataSet dataSet_vertex, dataSet_edge; \
    \
    initDataSet(&dataSet_edge, faceFieldList[0]->getSize()); \
    initDataSet(&dataSet_vertex, cellFieldList[0]->getSize()); \
    for (int i = 0; i < faceFieldList.size(); ++i) \
    { \
        pushArrayToDataSet(&dataSet_edge, \
            faceFieldList[i]->getDim(), \
            faceFieldList[i]->getLocalElement()->length(), \
            faceInoutList[i], \
            faceFieldList[i]->getLocalData()); \
    } \
    \
    for (int i = 0; i < cellFieldList.size(); ++i) \
    {\
        pushArrayToDataSet(&dataSet_vertex, \
            cellFieldList[i]->getDim(), \
            cellFieldList[i]->getLocalElement()->length(), \
            cellInoutList[i], \
            cellFieldList[i]->getLocalData()); \
    } \
    \
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
    opt->fun_slave = funcPtr; \
    /* 生成UNAT迭代器 */ \
    static UNAT::RowSubsectionIterator *rssIter = new UNAT::RowSubsectionIterator(opt, \
     *UNATTopo, E2V); \
    /* 调用UNAT迭代器计算，UNAT函数指针需要进行改造 */ \
    rssIter->edge2VertexIteration(&paraData, opt, 1);  \
}

#endif
