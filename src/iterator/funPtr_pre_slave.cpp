/**
* @file: funPtr_pre_slave.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 11:04:42
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-23 15:42:43
*/

// #include "unat/iterator.h"
// #include "unat/rowSubsectionIterator.hpp"

// #undef biSearch
// #define label unat_label
// #define scalar unat_scalar

// using namespace HSF;



#include "funPtr_slave.hpp"

void spMV_pre_slave(Region& reg)
{
    Field<scalar>& field1 = reg.getField<scalar>("setType1", "fieldName1");
    Field<scalar>& field2 = reg.getField<scalar>("setType2", "fieldName2");

    char* type1 = "setType1";
    char* type2 = "setType2";

    scalar *var1 = field1.getLocalData();
    scalar *var2 = field2.getLocalData();

    label dim1 = field1.getDim();
    label dim2 = field2.getDim();

    label size1 = field1.getSize();
    label size2 = field2.getSize();

    // DataSet dataSet_edge, dataSet_vertex;
    // constructSingleArray(dataSet_edge, dim1, size1, COPYIN, var1);
    // constructSingleArray(dataSet_vertex, dim2, size2, COPYOUT, var2);

    // ArrayArray& topo = chooseTopology(type1, type2);

    // Topology *topo = constructTopology(topo, size1);

    // UNAT::RSSIterator rssIter(topo, LDU);


    // CoupledOperator opt;
    // opt.data.edgeData = &dataSet1;
    // opt.data.vertexData = &dataSet2;
    // opt.fun_slave = slave_spMV;

    // rssIter.iteration(NULL, &opt, 1);
}