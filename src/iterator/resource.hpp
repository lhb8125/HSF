#ifndef HSF_RESOURCE_HPP
#define HSF_RESOURCE_HPP

#include "region.hpp"
// #include "funPtr_slave.hpp"
// #include "unat/container.hpp"
#include "unat/iterator.h"
#include "unat/topology.hpp"
#include "unat/rowSubsectionIterator.hpp"

ParaSet paraData;

#define prepareField(reg, inoutList, funcPtr, fieldList) \
{ \
    DataSet dataSet_vertex, dataSet_edge; \
    \
    Array<Word> setTypeList; \
    Array<scalar*> varList; \
    Array<label32> dimList; \
    label edgeNum, cellNum; \
    for (int i = 0; i < fieldList.size(); ++i) \
    { \
        scalar* var          = fieldList[i]->getLocalData(); \
        Word setType         = fieldList[i]->getType(); \
        label32 dim          = fieldList[i]->getDim(); \
        label64 size         = fieldList[i]->getSize(); \
        setTypeList.push_back(setType); \
        varList.push_back(var); \
        dimList.push_back(dim); \
        if(setType=="cell") \
            cellNum = size; \
        else if(setType=="face") \
            edgeNum = size; \
        else \
          std::cout<<"error!!!"<<endl; \
    } \
    initDataSet(&dataSet_edge, edgeNum); \
    initDataSet(&dataSet_vertex, cellNum); \
    for (int i = 0; i < fieldList.size(); ++i) \
    { \
        if(setTypeList[i]=="cell") \
        { \
            pushArrayToDataSet(&dataSet_vertex, dimList[i], sizeof(scalar), \
                inoutList[i], varList[i]); \
        } else \
        { \
            pushArrayToDataSet(&dataSet_edge, dimList[i], sizeof(scalar), \
                inoutList[i], varList[i]); \
        } \
    } \
    /* 根据数据集类型选择拓扑 */ \
    UTILITY::ArrayArray<label> topo = reg.getTopology<label>(setTypeList); \
    label n = topo.size(); \
    /* 创建UNAT拓扑 */ \
    UNAT::Topology* UNATTopo = UNAT::Topology::constructTopology(topo, LDU); \
    /* 封装耦合算子 */ \
    coupledOperator_new opt; \
    opt.dataSet_edge = &dataSet_edge; \
    opt.dataSet_vertex = &dataSet_vertex; \
    opt.fun_slave = funcPtr; \
    /* 生成UNAT迭代器 */ \
    static UNAT::RowSubsectionIterator *rssIter = new UNAT::RowSubsectionIterator(&opt, \
     *UNATTopo, E2V); \
    /* 调用UNAT迭代器计算，UNAT函数指针需要进行改造 */ \
    rssIter->edge2VertexIteration(&paraData, &opt, 1); \
}

#endif
