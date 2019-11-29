/**
* @file: utilities.hpp
* @author: Liu Hongbin
* @brief: some useful utilities
* @date:   2019-10-08 15:12:44
* @last Modified by:   lenovo
* @last Modified time: 2019-11-29 16:49:46
*/
#ifndef UTILITIES_HPP
#define UTILITIES_HPP


#include <iostream>
#include <vector>
#include <algorithm>
#include "mpi.h"
#include "base.hpp"
#include "container.hpp"
#include "basicFunction.h"
#include "basicFunction.hpp"
#include "usingCpp.hpp"
#include "OStream.hpp"
#include "multiOStream.hpp"
#include "dummyOStream.hpp"
#include "communicator.hpp"
#include "communicationManager.hpp"
#include "mpiWrapper.hpp"
#include "utilInterfaces.h"

namespace HSF
{

// #define Array std::vector
#define String std::string
//#define Label label
//#define Scalar scalar
typedef label Label;
typedef scalar Scalar;

#define Inner 0
#define Boco 1

#define Terminate(location, content) {printf("Location: %s, error message: %s, file: %s, line: %d\n", location, content, __FILE__, __LINE__); exit(-1);}

#if(Label==long)
#define MPI_LABEL MPI_LONG
#else
#define MPI_LABEL MPI_INT
#endif

// #define DEBUG_METIS
// #define DEBUG



} // end namespace HSF

#endif
