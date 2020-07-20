#ifndef HSF_RESOURCE_HPP
#define HSF_RESOURCE_HPP

#include "region.hpp"
// #include "funPtr_slave.hpp"
#include "unat/container.hpp"
#include "unat/iterator.h"
#include "unat/topology.hpp"
#include "unat/rowSubsectionIterator.hpp"

Arrays paraData;

void prepareField(label32* inoutList, e2v_slaveFunPtr funcPtr, int nPara, ...);
void prepareConst(int nPara, ...);


#endif
