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
#include "cstdlib"
#include "mpi.h"
#include "utilities.hpp"
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "cgnslib.h"
// #include "fieldInterfaces.hpp"
#define OUT std::cout
#define IN std::cin
#define ENDL std::endl
#define String std::string

void spMV_data(Region& reg, label n_face, label n_cell);
void spMV_test(Region& reg, ArrayArray<label>& face_2_cell,
	label n_face_i, label n_face_b, label n_face, label n_cell);
void spMV_bnd(Region& reg, ArrayArray<label>& face_2_cell,
	label n_face_b);

using namespace HSF;

// #define DEBUG_YAML

// void loadRegionTopologyFromYAML(String filePtr, Array<Scalar> &s,
// 	ArrayArray<Label> &nei, Label procNum);
// void operator >> (const YAML::Node& node, Array<Scalar>& s);
// void operator >> (const YAML::Node& node, Array<Array<Label> >& nei);
// void operator >> (const YAML::Node& node, Array<Label>& regionIdx);
// void hdf5ToAdf(char* filePtr, char* desFilePtr);

int main(int argc, char** argv)
{
	LoadBalancer *lb = new LoadBalancer();

	Parameter para("./config.yaml");

	/// initialize MPI environment
	printf("initialize MPI environment ......\n");
	// int numproces, rank;
	// MPI_Init(&argc, &argv);
	// MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// MPI_Comm_size(MPI_COMM_WORLD, &numproces);
	// printf("This is process %d, %d processes are launched\n", rank, numproces);
	init_utility_();

	int nPara = 4;
	// char meshFile[100];
	Array<char*> mesh_file(10);
	for (int i = 0; i < mesh_file.size(); ++i)
	{
		mesh_file[i] = new char[100];
	}
	// para.getPara(&nPara, meshFile, "char*", "domain1", "region", "0", "path");
	para.getPara<char>(mesh_file, nPara, "domain1", "region", "0", "path");
	
	printf("reading CGNS file: ");
	for (int i = 0; i < mesh_file.size(); ++i)
	{
		printf("%s, ", mesh_file[i]);
	}
	printf("\n");

	char resultFile[100];
	nPara = 4;
	para.getPara<char>(resultFile, nPara, "domain1", "region", "0", "resPath");
	printf("writing CGNS file: %s\n", resultFile);
	/// read CGNS file
	Array<Region> regs;
	Region reg;
	regs.push_back(reg);

	regs[0].initBeforeBalance(mesh_file);

	
	/// load balance in region
	lb->LoadBalancer_3(regs);

	regs[0].initAfterBalance();

	ArrayArray<label> face_2_cell = regs[0].getMesh().getTopology().getFace2Cell();
	label n_face_i = regs[0].getMesh().getTopology().getInnFacesNum();
	label n_face_b = regs[0].getBoundary().getTopology().getFacesNum();
	label n_face   = regs[0].getMesh().getTopology().getFacesNum();
	label n_cell   = regs[0].getMesh().getTopology().getCellsNum();

	spMV_data(regs[0], n_face, n_cell);
	spMV_test(regs[0], face_2_cell, n_face_i, n_face_b, n_face, n_cell);

	face_2_cell = regs[0].getBoundary().getTopology().getFace2Cell();
	spMV_bnd(regs[0], face_2_cell, n_face_b);

	regs[0].writeMesh(resultFile);
	regs[0].writeField<scalar>(resultFile, "b", "cell");

	// MPI_Finalize();
	
	return 0;
}

void spMV_test(Region& reg, ArrayArray<label>& face_2_cell,
	label n_face_i, label n_face_b, label n_face, label n_cell)
{
	Field<scalar> &A = reg.getField<scalar>("face", "A");
	Field<scalar> &x = reg.getField<scalar>("cell", "x");
	Field<scalar> &b = reg.getField<scalar>("cell", "b");
    // printf("n_face: %d, n_face_i: %d, n_face_b: %d, n_cell: %d\n",
    	// n_face, n_face_i, n_face_b, n_cell);
    for (int i = 0; i < n_face_i; ++i)
    {
    	label row = face_2_cell[i][0];
    	label col = face_2_cell[i][1];
        b[col][0] += A[i][0]*x[row][0];
        b[row][0] += A[i][0]*x[col][0];
    }
}

void spMV_bnd(Region& reg, ArrayArray<label>& face_2_cell,
	label n_face_b)
{
	Field<scalar> &A = reg.getField<scalar>("face", "A");
	Field<scalar> &b = reg.getField<scalar>("cell", "b");
    // 边界计算部分
    for (int i = 0; i < n_face_b; ++i)
    {
    	label row = face_2_cell[i][0];
        b[row][0] += A[i][0];
    }
}

void spMV_data(Region& reg, label n_face, label n_cell)
{
	scalar *A_arr = new scalar[n_face];
	scalar *x_arr = new scalar[n_cell];
	scalar *b_arr = new scalar[n_cell];

	for (int i = 0; i < n_face; ++i)
	{
		A_arr[i] = 1.0;
	}
	for (int i = 0; i < n_cell; ++i)
	{
		x_arr[i] = 1.0;
		b_arr[i] = 0.0;
	}

	Table<Word, Table<Word, Patch *> *> &patchTab = reg.getPatchTab();
	Field<scalar> *fA = new Field<scalar>("face", 1, n_face, A_arr, patchTab);
	Field<scalar> *fx = new Field<scalar>("cell", 1, n_cell, x_arr, patchTab);
	Field<scalar> *fb = new Field<scalar>("cell", 1, n_cell, b_arr, patchTab);
	reg.addField<scalar>("A", fA);
	reg.addField<scalar>("x", fx);
	reg.addField<scalar>("b", fb);
}