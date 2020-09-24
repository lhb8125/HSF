/**
* @file: test.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 11:04:42
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-23 15:42:43
*/
#include <iostream>
#include <fstream>
#include <string>
#include <yaml-cpp/yaml.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include "cstdlib"
#include "mpi.h"
#include "utilities.h"
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "pcgnslib.h"
#include "region.hpp"
#include "funPtr_host.hpp"
#include "topoInterfaces.hpp"
#include "element.hpp"
#include "setType.hpp"
#include "scalar.hpp"

#ifdef sw
#include "kernel_spe.hpp"
#include "unat/athread_switch.h"
#endif
// #include "fieldInterfaces.hpp"
#define OUT std::cout
#define IN std::cin
#define ENDL std::endl
#define String std::string

using namespace HSF;

template<class SetType, class Element>
void checkResult_new(Region& reg, Word x, Word x_spe);

// void checkResult(Region& reg, Word x, Word x_spe);

int main(int argc, char** argv)
{
#ifdef sw
	CG_init();
#endif
	initUtility();
	srand((int)time(0));

	Parameter para("./test/system/config.yaml");
	ControlPara newPara("./test/system/config.yaml");
	// Word path = newPara["domain"]["region"]["0"]["path"].to<Word>();
	// // printf("%s\n", path.c_str());
	// scalar deltaT;
	// newPara["domain"]["solve"]["deltaT"].read(deltaT);
	// // printf("%f\n", deltaT);
	// label32 equNum;
	// equNum = newPara["domain"]["equation"].size();

	/// initialize MPI environment
	printf("initialize MPI environment ......\n");
	// int numproces, rank;
	// MPI_Init(&argc, &argv);
	// MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// MPI_Comm_size(MPI_COMM_WORLD, &numproces);
	// printf("This is process %d, %d processes are launched\n", rank, numproces);
    Communicator &global_comm = COMM::getGlobalComm();
    LoadBalancer *lb = new LoadBalancer(global_comm);

	Element<label> *ele = new Element<label>(5);
	Field<SetType, Element<label> > *f = new Field<SetType, Element<label> >(1, ele, global_comm);
	(*f)[0].show();

	int nPara = 4;
	// char meshFile[100];
	Array<Word> mesh_files;
	Word meshFile;
	newPara["domain"]["region"]["0"]["path"].read(meshFile);
	mesh_files.push_back(meshFile);
	
	printf("reading CGNS file: ");
	for (int i = 0; i < mesh_files.size(); ++i)
	{
		printf("%s, ", mesh_files[i].c_str());
	}
	printf("\n");

	Word resultFile;
	newPara["domain"]["region"]["0"]["resPath"].read(resultFile);
	printf("writing CGNS file: %s\n", resultFile.c_str());
	/// read CGNS file
	Array<Region> regs;
	Region reg(global_comm);
	regs.push_back(reg);

	regs[0].initBeforeBalance(mesh_files);

	
	/// load balance in region
	lb->LoadBalancer_3(regs);

	regs[0].initAfterBalance();

	/// test spmv
	ArrayArray<label> face_2_cell = regs[0].getMesh().getTopology().getFace2Cell();
	label n_face_i = regs[0].getMesh().getTopology().getInnFacesNum();
	label n_face_b = regs[0].getBoundary().getTopology().getFacesNum();
	label n_face   = regs[0].getMesh().getTopology().getFacesNum();
	label n_cell   = regs[0].getMesh().getTopology().getCellsNum();

#ifdef sw
	// integration
	integration_data(regs[0], n_face, n_cell);
	integration(regs[0], "flux","U");
    integration_spe(regs[0], "flux","U_spe");
    checkResult_new<Cell,ScalarEle<scalar> >(regs[0], "U", "U_spe");

    // spMV
    label pi;
    StructS s;
    label32* arr = new label32[10];
	spMV_data(regs[0], n_face, n_cell, pi, s, arr);
	spMV(regs[0], "A", "x", "b", pi, s, arr);
	spMV_spe(regs[0], "A", "x", "b_spe", pi, s, arr);
    checkResult_new<Cell,ScalarEle<scalar> >(regs[0], "b", "b_spe");

 //    // calcLudsFcc
    calcLudsFcc_data(regs[0], n_face, n_cell);
    calcLudsFcc(regs[0], "massFlux", "cellx", "fcc", "facex", "rface0", "rface1", "S");
    calcLudsFcc_spe(regs[0], "massFlux", "cellx", "fcc_s", "facex", "rface0_s", "rface1_s", "S_s");
    checkResult_new<Face,ScalarEle<scalar> >(regs[0], "fcc", "fcc_s");
    checkResult_new<Face,ScalarEle<scalar> >(regs[0], "rface0", "rface0_s");
    checkResult_new<Face,ScalarEle<scalar> >(regs[0], "rface1", "rface1_s");
    checkResult_new<Cell,ScalarEle<scalar> >(regs[0], "S", "S_s");
#endif

    // test MPI
    // integration_data(regs[0], n_face, n_cell);
    // Field<Cell, ScalarEle<scalar> > &fnU = regs[0].getField<Cell, ScalarEle<scalar> >("U");
    // fnU.initSend();
    // fnU.checkSendStatus();
    // integration(regs[0], "flux","U");
	// face_2_cell = regs[0].getBoundary().getTopology().getFace2Cell();
	// spMV_bnd(regs[0], face_2_cell, n_face_b);

	regs[0].writeMesh(resultFile);
	regs[0].writeField<Cell, ScalarEle<scalar> >(resultFile.c_str(), "U");

	// MPI_Finalize();
	
	return 0;
}

// void checkResult(Region& reg, Word x, Word x_spe)
// {
// 	printf("checking result for %s ...\n", x.c_str());
// 	Field<scalar> &fieldX     = reg.getField<scalar>(x);
//     Field<scalar> &fieldX_spe = reg.getField<scalar>(x_spe);

//     label size = fieldX.getSize();
//     label dim  = fieldX.getDim();
//     scalar* x_arr    = fieldX.getLocalData();
//     scalar* xspe_arr = fieldX_spe.getLocalData();
//     for (int i = 0; i < size*dim; ++i)
//     {
// 		if(fabs(x_arr[i]-xspe_arr[i])>1e-10)
// 		{ 
// 			if(x_arr[i]==0) 
// 			{ 
// 				if(fabs(xspe_arr[i])>1e-10) 
// 				{ 
// 					printf("Error on index[%d], %.8f, %.8f\n", 
// 								i, x_arr[i], xspe_arr[i]); 
// 					exit(-1); 
// 				} 
// 			} 
// 			else if(fabs((x_arr[i]-xspe_arr[i])/x_arr[i])>1e-10) 
// 			{ 
// 				printf("Error on index[%d], %.8f, %.8f\n", 
// 							i, x_arr[i], xspe_arr[i]); 
// 				exit(-1); 
// 			} 
// 		} 
//     }
// 	std::cout<<"The result is correct!"<<std::endl; \
// }

template<class SetType, class Element>
void checkResult_new(Region& reg, Word x, Word x_spe)
{
	printf("checking result for %s ...\n", x.c_str());
	Field<SetType, Element> &fieldX     = reg.getField<SetType, Element>(x);
	Field<SetType, Element> &fieldX_spe = reg.getField<SetType, Element>(x_spe);

    label size = fieldX.getSize();
    for (int i = 0; i < size; ++i)
    {
    	if(fieldX[i]!=fieldX_spe[i]) {
    		std::cout<<"The result is wrong!"<<std::endl;
    		return;
    	}
    }
	std::cout<<"The result is correct!"<<std::endl;
}
