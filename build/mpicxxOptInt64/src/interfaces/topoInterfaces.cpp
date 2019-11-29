/**
* @file: interfaces.hpp
* @author: Liu Hongbin
* @brief: fortran function interfaces
* @date:   2019-11-11 10:56:28
* @last Modified by:   lenovo
* @last Modified time: 2019-11-29 14:35:28
*/
#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <unistd.h>
#include "cstdlib"
#include "mpi.h"
#include "parameter.hpp"
#include "topoInterfaces.hpp"
#include "loadBalancer.hpp"

using namespace HSF;

#define REGION regs[0]
//extern "C"
//{
/**
* @brief fortran function interfaces
*/
/*****************************************************************************/

void init_(char* configFile, int* length)
{
	LoadBalancer *lb = new LoadBalancer();
	// std::cout<<"start initializing ......"<<std::endl;

	para.setParaFile(configFile, *length);

	// printf("%s\n", configFile);

	// int numproces, rank;
	// MPI_Init(NULL, NULL);
	// MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// MPI_Comm_size(MPI_COMM_WORLD, &numproces);

	int nPara = 4;
	char meshFile[100];
	// para.getPara(&nPara, meshFile, "char*", "domain1", "region", "0", "path");
	para.getPara<char>(&nPara, meshFile, "domain1", "region", "0", "path");
	char resultFile[100];
	para.getPara<char>(&nPara, resultFile, "domain1", "region", "0", "resPath");

	/// initialization before load balance
	Region reg;
	// regs.resize(1);
	regs.push_back(reg);
	REGION.initBeforeBalance(meshFile);

	/// load balance in region
	lb->LoadBalancer_3(regs);

	/// initialization after load balance
	REGION.initAfterBalance();

	// DELETE_POINTER(lb);
}

void get_elements_num_(Label* eleNum)
{
	eleNum[0] = REGION.getMesh().getTopology().getCellsNum();
}


void get_faces_num_(Label* faceNum)
{
	faceNum[0] = REGION.getMesh().getTopology().getFacesNum();
}


void get_inner_faces_num_(Label* innFaceNum)
{
	innFaceNum[0] = REGION.getMesh().getTopology().getInnFacesNum();
}


void get_bnd_faces_num_(Label* bndFaceNum)
{
	bndFaceNum[0] = REGION.getMesh().getTopology().getBndFacesNum();
}


void get_nodes_num_(Label* nodeNum)
{
	nodeNum[0] = REGION.getMesh().getOwnNodes().size();
}

/*****************************************************************************/

void get_ele_2_face_pos_(Label* pos)
{
	Label* tmp = REGION.getMesh().getTopology().getCell2Face().startIdx;
	Label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_ele_2_face_(Label* ele2Face)
{
	Label* tmpData = REGION.getMesh().getTopology().getCell2Face().data;
	Label* tmp = REGION.getMesh().getTopology().getCell2Face().startIdx;
	Label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			ele2Face[j] = tmpData[j]+1;
		}
	}
}


void get_ele_2_ele_pos_(Label* pos)
{
	Label* tmp = REGION.getMesh().getTopology().getCell2Cell().startIdx;
	Label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_ele_2_ele_(Label* ele2Ele)
{
	Label* tmpData = REGION.getMesh().getTopology().getCell2Cell().data;
	Label* tmp = REGION.getMesh().getTopology().getCell2Cell().startIdx;
	Label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			ele2Ele[j] = tmpData[j];
		}
	}
}


void get_ele_2_node_pos_(Label* pos)
{
	Label* tmp = REGION.getMesh().getTopology().getCell2Node().startIdx;
	Label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_ele_2_node_(Label* ele2Node)
{
	Label* tmpData = REGION.getMesh().getTopology().getCell2Node().data;
	Label* tmp = REGION.getMesh().getTopology().getCell2Node().startIdx;
	Label eleNum = REGION.getMesh().getTopology().getCellsNum();
	Table<Label, Label> coordMap = REGION.getMesh().getCoordMap();
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			ele2Node[j] = coordMap[tmpData[j]-1]+1;
		}
	}
}


void get_inn_face_2_node_pos_(Label* pos)
{
	Label* tmp = REGION.getMesh().getTopology().getFace2Node().startIdx;
	Label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_inn_face_2_node_(Label* face2Node)
{
	Label* tmpData = REGION.getMesh().getTopology().getFace2Node().data;
	Label* tmp = REGION.getMesh().getTopology().getFace2Node().startIdx;
	Label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	Table<Label, Label> coordMap = REGION.getMesh().getCoordMap();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Node[j] = coordMap[tmpData[j]-1]+1;
		}
	}
}


void get_bnd_face_2_node_pos_(Label* pos)
{
	Label* tmp = REGION.getBoundary().getTopology().getFace2Node().startIdx;
	Label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_bnd_face_2_node_(Label* face2Node)
{
	Label* tmpData = REGION.getBoundary().getTopology().getFace2Node().data;
	Label* tmp = REGION.getBoundary().getTopology().getFace2Node().startIdx;
	Label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Node[j] = tmpData[j];
		}
	}
}


void get_inn_face_2_ele_pos_(Label* pos)
{
	Label* tmp = REGION.getMesh().getTopology().getFace2Cell().startIdx;
	Label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_inn_face_2_ele_(Label* face2Ele)
{
	Label* tmpData = REGION.getMesh().getTopology().getFace2Cell().data;
	Label* tmp = REGION.getMesh().getTopology().getFace2Cell().startIdx;
	Label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Ele[j] = tmpData[j]+1;
		}
	}
}


void get_bnd_face_2_ele_pos_(Label* pos)
{
	Label* tmp = REGION.getBoundary().getTopology().getFace2Cell().startIdx;
	Label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_bnd_face_2_ele_(Label* face2Ele)
{
	Label* tmpData = REGION.getBoundary().getTopology().getFace2Cell().data;
	Label* tmp = REGION.getBoundary().getTopology().getFace2Cell().startIdx;
	Label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Ele[j] = tmpData[j]+1;
		}
	}
}

/******************************************************************************/
void get_ele_type_(Label* eleType)
{
	Array<Label> tmp = REGION.getMesh().getTopology().getCellType();
	for (int i = 0; i < tmp.size(); ++i)
	{
		eleType[i] = tmp[i];
	}
}

void get_coords_(Scalar* coords)
{
	Scalar* tmpData = REGION.getMesh().getOwnNodes().getXYZ().data;
	Label* tmp     = REGION.getMesh().getOwnNodes().getXYZ().startIdx;
	Label nodeNum = REGION.getMesh().getOwnNodes().size();
	for (int i = 0; i < nodeNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			coords[j] = tmpData[j];
		}
	}
}

/*****************************************************************************/

// }