/**
* @file: mesh.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-25 11:21:52
* @last Modified by:   lenovo
* @last Modified time: 2019-11-29 10:52:19
*/
#include <cstdio>
#include <iostream>
#include <typeinfo>
#include <stdlib.h>
#include <assert.h>
#include "mpi.h"
#include "pcgnslib.h"
#include "utilities.hpp"
#include "mesh.hpp"
#include "cgnslib.h"

// #define INTEGER32
namespace HSF
{

void Mesh::readCGNSFilePar(const char* filePtr)
{
	int rank, numProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	par_std_out_("This is rank %d in %d processes\n", rank, numProcs);

	int iFile, nBases=0, cellDim=0, physDim=0;
	int iBase=1, iZone=1;
	char basename[20];

	if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
		Terminate("initCGNSMPI", cg_get_error());
// par_std_out_("read CGNS info\n");
// par_std_out_("read CGNS info\n");
	if(cgp_open(filePtr, CG_MODE_READ, &iFile) ||
        cg_nbases(iFile, &nBases) ||
        cg_base_read(iFile, iBase, basename, &cellDim, &physDim))
		Terminate("readBaseInfo", cg_get_error());
        // cgp_error_exit();
// par_std_out_("finish read CGNS info\n");
	par_std_out_("nBases: %d, basename: %s, cellDim: %d, physDim: %d\n", nBases, basename, cellDim, physDim);
	MPI_Barrier(MPI_COMM_WORLD);

    int precision;
    cg_precision(iFile, &precision);
    par_std_out_("precision: %d\n", precision);

	char zoneName[20];
	int nZones;
	cgsize_t sizes[3];
	ZoneType_t zoneType;
	if(cg_nzones(iFile, iBase, &nZones) ||
		cg_zone_read(iFile, iBase, iZone, zoneName, sizes) ||
		cg_zone_type(iFile, iBase, iZone, &zoneType) ||
		zoneType != Unstructured)
		Terminate("readZoneInfo", cg_get_error());
	par_std_out_("nZones: %d, zoneName: %s, zoneType: %d, nodeNum, %d, eleNum: %d, bndEleNum: %d\n", nZones, zoneName, zoneType, sizes[0], sizes[1], sizes[2]);
	this->nodeNum_ = sizes[0];
	this->eleNum_  = sizes[1];

	int nCoords;
	if(cg_ncoords(iFile, iBase, iZone, &nCoords) ||
		nCoords != 3)
		Terminate("readNCoords", cg_get_error());
    int nnodes = (nodeNum_ + numProcs - 1) / numProcs;
    cgsize_t start  = nnodes * rank + 1;
    cgsize_t end    = nnodes * (rank + 1);
    if (end > nodeNum_) end = nodeNum_;
    nnodes = end - start + 1;
    Array<scalar*> coords;
    par_std_out_("The vertices range of processor %d is (%d, %d). \n", rank, start, end);
	DataType_t dataType;
	char coordName[20];
	scalar* x = new scalar[nnodes];
	if(cg_coord_info(iFile, iBase, iZone, 1, &dataType, coordName) ||
		// sizeof(dataType)!=sizeof(scalar) ||
		cgp_coord_read_data(iFile, iBase, iZone, 1, &start, &end, x))
		Terminate("readCoords", cg_get_error());
    if(dataType==RealSingle && sizeof(scalar)==8)
        Terminate("readCoords","The data type of scalar does not match");
    if(dataType==RealDouble && sizeof(scalar)==4)
        Terminate("readCoords","The data type of scalar does not match");
	scalar* y = new scalar[nnodes];
	if(cg_coord_info(iFile, iBase, iZone, 2, &dataType, coordName) ||
		// sizeof(dataType)!=sizeof(scalar) ||
		cgp_coord_read_data(iFile, iBase, iZone, 2, &start, &end, y))
		Terminate("readCoords", cg_get_error());
	scalar* z = new scalar[nnodes];
	if(cg_coord_info(iFile, iBase, iZone, 3, &dataType, coordName) ||
		// sizeof(dataType)!=sizeof(scalar) ||
		cgp_coord_read_data(iFile, iBase, iZone, 3, &start, &end, z))
		Terminate("readCoords", cg_get_error());
	MPI_Barrier(MPI_COMM_WORLD);
    // par_std_out_("%d\n", sizeof(dataType));

    // Nodes *node = new Nodes(x, y, z, nnodes);
    Nodes node(x, y, z, nnodes);
	this->nodes_.copy(&node);
	this->nodes_.setStart(start);
	this->nodes_.setEnd(end);
    // int i,j,k,n,nn,ne;
    // nn = 0;
    // if(rank==0){
    // for (int i = 0; i < nnodes; ++i)
    // {
    //     par_std_out_("x: %lf, ", x[i]);
    //     par_std_out_("y: %lf, ", y[i]);
    //     par_std_out_("z: %lf\n", z[i]);
    	
    // }
	int nSecs;
	if(cg_nsections(iFile, iBase, iZone, &nSecs))
		Terminate("readNSections", cg_get_error());
	int iSec;
    par_std_out_("nSecs: %d\n", nSecs);
	for (int iSec = 1; iSec <= nSecs; ++iSec)
	{
		char secName[20];
		cgsize_t start, end;
		ElementType_t type;
		int nBnd, parentFlag;
		if(cg_section_read(iFile, iBase, iZone, iSec, secName, 
			&type, &start, &end, &nBnd, &parentFlag))
			Terminate("readSectionInfo", cg_get_error());
        // par_std_out_("%d\n", this->meshType_);
        if(! Section::compareEleType(type, this->meshType_)) continue;
        par_std_out_("iSec: %d, sectionName: %s, type: %d, start: %d, end: %d, nBnd: %d\n", iSec, secName, type, start, end, nBnd);

    	Section sec;
    	sec.name = new char[20];
    	strcpy(sec.name, secName);
    	sec.type   = type;
    	sec.nBnd   = nBnd;
		/// if the section does not match the type of mesh, then ignore it.
		label secStart = start-1;
		label eleNum = end-start+1;
    	int nEles = (eleNum + numProcs - 1) / numProcs;
    	start  = nEles * rank + 1;
    	end    = nEles * (rank + 1);
    	if (end > eleNum) end = eleNum;
    	par_std_out_("processor %d will read elements from %d to %d.\n", rank, start+secStart, end+secStart);

    	cgsize_t* elements = new cgsize_t[nEles*Section::nodesNumForEle(type)];
    	if(cgp_elements_read_data(iFile, iBase, iZone, iSec, start+secStart, end+secStart, elements))
    		Terminate("readElements", cg_get_error());
		MPI_Barrier(MPI_COMM_WORLD);

        par_std_out_("%d\n", precision);
        sec.iStart = start+secStart;
        sec.iEnd   = end+secStart;
    	sec.num    = end-start+1;
        if(precision==64)
        {
            par_std_out_("This is 64 precision\n");
            sec.conn   = (label*)elements;
        } else if(precision==32)
        {
            int *eles32 = (int*)&elements[0];
            label* eles64 = new label[nEles*Section::nodesNumForEle(type)];
            for (int i = 0; i < nEles; ++i)
    	    {
                for (int j = 0; j < Section::nodesNumForEle(type); ++j)
                {
                    eles64[i*Section::nodesNumForEle(type)+j] = (label)eles32[i*Section::nodesNumForEle(type)+j];
                }
        	}
            sec.conn = eles64;
            eles32 = NULL;
            eles64 = NULL;
            DELETE_POINTER(elements);
        }
        this->secs_.push_back(sec);
	}
    MPI_Barrier(MPI_COMM_WORLD);

    // readBoundaryCondition(iFile, iBase, iZone);
    // MPI_Barrier(MPI_COMM_WORLD);

	if(cgp_close(iFile))
		Terminate("closeCGNSFile",cg_get_error());

    // DELETE_POINTER(node);
    DELETE_POINTER(z);
    DELETE_POINTER(y);
    DELETE_POINTER(x);

}

void Mesh::writeCGNSFilePar(const char* filePtr)
{
	int nodesPerSide = 5;
	int nodeNum = this->nodeNum_;
	int eleNum  = this->eleNum_;

	int rank, numProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	// par_std_out_("This is rank %d in %d processes\n", rank, numProcs);

	int iFile, nBases, cellDim, physDim, Cx, Cy, Cz;
	int iBase=1, iZone=1;
	char basename[20];

	cgsize_t sizes[3];
	sizes[0] = nodeNum;
	sizes[1] = eleNum;
	sizes[2] = 0;

	if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
		Terminate("initCGNSMPI", cg_get_error());
	if(cgp_open(filePtr, CG_MODE_WRITE, &iFile) ||
		cg_base_write(iFile, "Base", 3, 3, &iBase) ||
        cg_zone_write(iFile, iBase, "Zone", sizes, Unstructured, &iZone))
		Terminate("writeBaseInfo", cg_get_error());
    /* print info */
    if (rank == 0) {
        par_std_out_("writing %d coordinates and %d hex elements to %s\n",
            nodeNum, eleNum, filePtr);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    DataType_t dataType;
    if(sizeof(scalar)==4) dataType = RealSingle;
    else dataType = RealDouble;
    /* create data nodes for coordinates */
    if (cgp_coord_write(iFile, iBase, iZone, dataType, "CoordinateX", &Cx) ||
        cgp_coord_write(iFile, iBase, iZone, dataType, "CoordinateY", &Cy) ||
        cgp_coord_write(iFile, iBase, iZone, dataType, "CoordinateZ", &Cz))
        Terminate("writeCoordInfo", cg_get_error());
    MPI_Barrier(MPI_COMM_WORLD);

    /* number of nodes and range this process will write */
    int nnodes = this->nodes_.size();
    cgsize_t start  = this->nodes_.getStart();
    cgsize_t end    = this->nodes_.getEnd();
    /* create the coordinate data for this process */
    scalar* x = new scalar[nnodes];
    scalar* y = new scalar[nnodes];
    scalar* z = new scalar[nnodes];
    for (int i = 0; i < nnodes; ++i)
    {
    	x[i] = this->nodes_.getX()[i];
    	y[i] = this->nodes_.getY()[i];
    	z[i] = this->nodes_.getZ()[i];
    	// par_std_out_("vertex: %d, x, %f, y, %f, z, %f\n", i, x[i], y[i], z[i]);
    }

    // /* write the coordinate data in parallel */
    if (cgp_coord_write_data(iFile, iBase, iZone, Cx, &start, &end, x) ||
        cgp_coord_write_data(iFile, iBase, iZone, Cy, &start, &end, y) ||
        cgp_coord_write_data(iFile, iBase, iZone, Cz, &start, &end, z))
        Terminate("writeCoords", cg_get_error());
	MPI_Barrier(MPI_COMM_WORLD);


    /// write connectivity
    ArrayArray<label> conn = this->getTopology().getCell2Node();
    Array<label> cellType = this->getTopology().getCellType();
    label *cellStartId = new label[numProcs+1];
    MPI_Allgather(&conn.num, 1, MPI_LABEL, &cellStartId[1], 1, MPI_LABEL, MPI_COMM_WORLD);
    cellStartId[0] = 0;
    // if(rank==1)
    // {
        for (int i = 0; i < numProcs; ++i)
        {
            cellStartId[i+1] += cellStartId[i];
            // cellStartId[i]++;
        }
        // par_std_out_("\n");
    // }
    /*
    * filter section
    * code
    */
    int iSec;
    ElementType_t eleType = (ElementType_t)cellType[0];
    // par_std_out_("internal faces: %d, %d\n", 1, cellStartId[numProcs]);
    if(cgp_section_write(iFile, iBase, iZone, typeToString(eleType), eleType,
        1, cellStartId[numProcs], 0, &iSec))
        Terminate("writeSecInfo", cg_get_error());
    // par_std_out_("%d, %d\n", start, end);
    cgsize_t *data = (cgsize_t*)conn.data;
    if(cgp_elements_write_data(iFile, iBase, iZone, iSec, cellStartId[rank]+1,
        cellStartId[rank+1], data))
        Terminate("writeSecConn", cg_get_error());

    DELETE_POINTER(cellStartId);

	if(cgp_close(iFile))
		Terminate("closeCGNSFile",cg_get_error());

    DELETE_POINTER(cellStartId);
    DELETE_POINTER(z);
    DELETE_POINTER(y);
    DELETE_POINTER(x);
}

void Mesh::initCGNSFilePar(const char* filePtr)
{
	int nodesPerSide = 5;
	int nodeNum = nodesPerSide*nodesPerSide*nodesPerSide;
	int eleNum  = (nodesPerSide-1)*(nodesPerSide-1)*(nodesPerSide-1);
    int faceNum = 6*(nodesPerSide-1)*(nodesPerSide-1);
    // int faceNum = 0;

	int rank, numProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	par_std_out_("This is rank %d in %d processes\n", rank, numProcs);

	int iFile, nBases, cellDim, physDim, Cx, Cy, Cz;
	int iBase=1, iZone=1;
	char basename[20];

	cgsize_t sizes[3];
	sizes[0] = nodeNum;
	sizes[1] = eleNum;
	sizes[2] = 0;

	if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
		Terminate("initCGNSMPI", cg_get_error());
	if(cgp_open(filePtr, CG_MODE_WRITE, &iFile) ||
		cg_base_write(iFile, "Base", 3, 3, &iBase) ||
        cg_zone_write(iFile, iBase, "Zone", sizes, Unstructured, &iZone))
		Terminate("writeBaseInfo", cg_get_error());
    /* print info */
    if (rank == 0) {
        par_std_out_("writing %d coordinates and %d hex elements and %d quad elements to %s\n",
            nodeNum, eleNum, faceNum, filePtr);
    }
    /* create data nodes for coordinates */
    DataType_t dataType;
    if(sizeof(scalar)==4) dataType = RealSingle;
    else dataType = RealDouble;
    if (cgp_coord_write(iFile, iBase, iZone, dataType, "CoordinateX", &Cx) ||
        cgp_coord_write(iFile, iBase, iZone, dataType, "CoordinateY", &Cy) ||
        cgp_coord_write(iFile, iBase, iZone, dataType, "CoordinateZ", &Cz))
        Terminate("writeCoordInfo", cg_get_error());
    /* number of nodes and range this process will write */
    int nnodes = (nodeNum + numProcs - 1) / numProcs;
    cgsize_t start  = nnodes * rank + 1;
    cgsize_t end    = nnodes * (rank + 1);
    if (end > nodeNum) end = nodeNum;
    /* create the coordinate data for this process */
    scalar* x = (scalar *)malloc(nnodes * sizeof(scalar));
    scalar* y = (scalar *)malloc(nnodes * sizeof(scalar));
    scalar* z = (scalar *)malloc(nnodes * sizeof(scalar));
    int i,j,k,n,nn,ne;
    nn = 0;
    for (n = 1, k = 0; k < nodesPerSide; k++) {
        for (j = 0; j < nodesPerSide; j++) {
            for (i = 0; i < nodesPerSide; i++, n++) {
                if (n >= start && n <= end) {
                    x[nn] = (scalar)i;
                    y[nn] = (scalar)j;
                    z[nn] = (scalar)k;
                    nn++;
                }
            }
        }
    }

    /* write the coordinate data in parallel */
    if (cgp_coord_write_data(iFile, iBase, iZone, Cx, &start, &end, x) ||
        cgp_coord_write_data(iFile, iBase, iZone, Cy, &start, &end, y) ||
        cgp_coord_write_data(iFile, iBase, iZone, Cz, &start, &end, z))
        Terminate("writeCoords", cg_get_error());

    /* create data node for elements */
    int iSec=1;
    if (cgp_section_write(iFile, iBase, iZone, "Hex", HEXA_8, 1, eleNum, 0, &iSec))
        Terminate("writeSecInfo", cg_get_error());
 
    /* number of elements and range this process will write */
    int nelems = (eleNum + numProcs - 1) / numProcs;
    start  = nelems * rank + 1;
    end    = nelems * (rank + 1);
    if (end > eleNum) end = eleNum;

    /* create the hex element data for this process */
    cgsize_t* e = (cgsize_t *)malloc(8 * nelems * sizeof(cgsize_t));
    nn = 0;
    for (n = 1, k = 1; k < nodesPerSide; k++) {
        for (j = 1; j < nodesPerSide; j++) {
            for (i = 1; i < nodesPerSide; i++, n++) {
                if (n >= start && n <= end) {
                    ne = i + nodesPerSide*((j-1)+nodesPerSide*(k-1));
                    e[nn++] = ne;
                    e[nn++] = ne + 1;
                    e[nn++] = ne + 1 + nodesPerSide;
                    e[nn++] = ne + nodesPerSide;
                    ne += nodesPerSide * nodesPerSide;
                    e[nn++] = ne;
                    e[nn++] = ne + 1;
                    e[nn++] = ne + 1 + nodesPerSide;
                    e[nn++] = ne + nodesPerSide;
                }
            }
        }
    }

    /* write the element connectivity in parallel */
    if (cgp_elements_write_data(iFile, iBase, iZone, iSec, start, end, e))
        Terminate("writeSecConn", cg_get_error());

    iSec++;
    label* faces = new label[4*faceNum];
    /// back face
    i=1;
    int nf=0;
    for (int j = 1; j < nodesPerSide; ++j)
    {
    	for (int k = 1; k < nodesPerSide; ++k)
    	{
    		nn = i+(j-1)*nodesPerSide+(k-1)*nodesPerSide*nodesPerSide;
    		faces[nf++] = nn;
    		faces[nf++] = nn+nodesPerSide*nodesPerSide;
    		faces[nf++] = nn+nodesPerSide*(nodesPerSide+1);
    		faces[nf++] = nn+nodesPerSide;
    	}
    }

    /// front face
    i=nodesPerSide;
    for (int j = 1; j < nodesPerSide; ++j)
    {
    	for (int k = 1; k < nodesPerSide; ++k)
    	{
    		nn = i+(j-1)*nodesPerSide+(k-1)*nodesPerSide*nodesPerSide;
    		faces[nf++] = nn;
    		faces[nf++] = nn+nodesPerSide;
    		faces[nf++] = nn+nodesPerSide*(nodesPerSide+1);
    		faces[nf++] = nn+nodesPerSide*nodesPerSide;
    	}
    }

    /// left face
    j=1;
    for (int i = 1; i < nodesPerSide; ++i)
    {
    	for (int k = 1; k < nodesPerSide; ++k)
    	{
    		nn = i+(j-1)*nodesPerSide+(k-1)*nodesPerSide*nodesPerSide;
    		faces[nf++] = nn;
    		faces[nf++] = nn+1;
    		faces[nf++] = nn+1+nodesPerSide*nodesPerSide;
    		faces[nf++] = nn+nodesPerSide*nodesPerSide;
    	}
    }

    /// right face
    j=nodesPerSide;
    for (int i = 1; i < nodesPerSide; ++i)
    {
    	for (int k = 1; k < nodesPerSide; ++k)
    	{
    		nn = i+(j-1)*nodesPerSide+(k-1)*nodesPerSide*nodesPerSide;
    		faces[nf++] = nn;
    		faces[nf++] = nn+nodesPerSide*nodesPerSide;
    		faces[nf++] = nn+1+nodesPerSide*nodesPerSide;
    		faces[nf++] = nn+1;
    	}
    }

    /// bottom face
    k=1;
    for (int i = 1; i < nodesPerSide; ++i)
    {
    	for (int j = 1; j < nodesPerSide; ++j)
    	{
    		nn = i+(j-1)*nodesPerSide+(k-1)*nodesPerSide*nodesPerSide;
    		faces[nf++] = nn;
    		faces[nf++] = nn+nodesPerSide;
    		faces[nf++] = nn+1+nodesPerSide;
    		faces[nf++] = nn+1;
    	}
    }

    /// top face
    k=nodesPerSide;
    for (int i = 1; i < nodesPerSide; ++i)
    {
    	for (int j = 1; j < nodesPerSide; ++j)
    	{
    		nn = i+(j-1)*nodesPerSide+(k-1)*nodesPerSide*nodesPerSide;
    		faces[nf++] = nn;
    		faces[nf++] = nn+1;
    		faces[nf++] = nn+1+nodesPerSide;
    		faces[nf++] = nn+nodesPerSide;
    	}
    }
    if (cgp_section_write(iFile, iBase, iZone, "Wall", QUAD_4, eleNum+1, eleNum+faceNum, 0, &iSec))
        Terminate("writeSecInfo", cg_get_error());
    int nfaces = (faceNum + numProcs - 1) / numProcs;
    start  = nfaces * rank + 1;
    end    = nfaces * (rank + 1);
    if (end > faceNum) end = faceNum;
    // if(rank==0)
    // {
    par_std_out_("start: %d, end: %d\n", start, end);
    	// if(cgp_section_write_data(iFile, iBase, iZone, "Wall", QUAD_4, eleNum+1, eleNum+faceNum, 0, faces, &iSec))
        	// Terminate("writeSecConn", cg_get_error());
    // }
    cgsize_t *cg_faces = (cgsize_t*)&faces[4*(start-1)];
    if (cgp_elements_write_data(iFile, iBase, iZone, iSec, start+eleNum, end+eleNum, cg_faces))
        Terminate("writeSecConn", cg_get_error());

    int iBC;
    cgsize_t range[] = {eleNum+1, eleNum+faceNum};
    if(cg_boco_write(iFile, iBase, iZone, "Walls", BCWall, PointRange, 2, range, &iBC))
    	Terminate("writeBC", cg_get_error());
    if(cg_boco_gridlocation_write(iFile, iBase, iZone, iBC, CellCenter))
    	Terminate("writeBCLocation", cg_get_error());

    DELETE_POINTER(faces);
    free(e);
    free(x);
    free(y);
    free(z);
}

void Mesh::readCGNSFile(const char* filePtr)
{
	// bool is_3D_cal = true;

	// label iFile,ier;

	// // par_std_out_("reading CGNS files: %s ......\n", filePtr);
	// /// open cgns file
 //    if(cg_open(filePtr, CG_MODE_READ, &iFile) != CG_OK)
 //    	Terminate("readGridCGNS", cg_get_error());

	// /// read base information
	// label nBases;
	// if(cg_nbases(iFile, &nBases) != CG_OK)
	// 	Terminate("readNBases", cg_get_error());
	// if(nBases!=1) 
	// 	Terminate("readNBases", "This example assumes one base");
	// label iBase = 1;

	// char basename[20];
	// label cellDim,physDim;
	// if(cg_base_read(iFile, iBase, basename, &cellDim, &physDim) != CG_OK)
	// 	Terminate("readBaseInfo", cg_get_error());
	// par_std_out_("nBases: %d, basename: %s, cellDim: %d, physDim: %d\n", nBases, basename, cellDim, physDim);


	// /// read zone information
	// label nZones;
	// if(cg_nzones(iFile, iBase, &nZones) != CG_OK)
	// 	Terminate("readNZones", cg_get_error());
	// if(nZones!=1) 
	// 	Terminate("readNZones", "This example assumes one zone");
	// label iZone = 1;
	// ZoneType_t zoneType;
	// if(cg_zone_type(iFile, iBase, iZone, &zoneType) != CG_OK)
 //     	Terminate("readZoneType", cg_get_error());
 //   	if(zoneType != Unstructured)
 //     	Terminate("readZoneType", "Unstructured zone expected");
	// char zoneName[20];
	// /// fixed value for unstructured mesh: 3
	// label size[3];
	// if(cg_zone_read(iFile, iBase, iZone, zoneName, size) != CG_OK)
	// 	Terminate("readZoneInfo", cg_get_error());

	// label nodesNum = size[0];
	// label cellsNum = size[1];
	// label bocoNum  = size[2];
	// par_std_out_("nZones: %d, zoneName: %s, nodesNum: %d,cellsNum, %d, bocoNum: %d\n",
	// 	nZones, zoneName, size[0], size[1], size[2]);


 //    /// read coordinate information
 //    label nCoords;
 //    if(cg_ncoords(iFile, iBase, iZone, &nCoords) != CG_OK)
 //    	Terminate("readNCoords", cg_get_error());
 //    if(nCoords!=3 && is_3D_cal) 
	// 	Terminate("readNCoords", "This example assumes three dimensions");
	// /// make sure data type is in double precision
	// char coordName[20];
	// DataType_t coordType;
 //    if(cg_coord_info(iFile, iBase, iZone, 1, &coordType, coordName) != CG_OK)
 //    	Terminate("readCoordInfo", cg_get_error());
 //    if(coordType != RealDouble && sizeof(scalar)==8) 
	// 	Terminate("readCoordInfo", "This example assumes double precision");
 //    if(cg_coord_info(iFile, iBase, iZone, 2, &coordType, coordName) != CG_OK)
 //    	Terminate("readCoordInfo", cg_get_error());
 //    if(coordType != RealDouble && sizeof(scalar)==8) 
	// 	Terminate("readCoordInfo", "This example assumes double precision");
 //    if(cg_coord_info(iFile, iBase, iZone, 3, &coordType, coordName) != CG_OK)
 //    	Terminate("readCoordInfo", cg_get_error());
 //    if(coordType != RealDouble && sizeof(scalar)==8) 
	// 	Terminate("readCoordInfo", "This example assumes double precision");

	// /// read coordinates X, Y and Z
	// scalar *coordX, *coordY, *coordZ, *coordk;
	// coordX = new scalar[nodesNum];
	// coordY = new scalar[nodesNum];
	// coordZ = new scalar[nodesNum];

	// // coordk = new scalar[nodesNum];
	// label one = 1;
	// // par_std_out_("%p,%p,%p,%p\n", coordX,coordY,coordZ,coordk);
 //    if(cg_coord_read(iFile, iBase, iZone, "CoordinateX", coordType, &one,
 //                    &nodesNum, coordX) != CG_OK)
 //    	Terminate("readCoordX", cg_get_error());
 //    if(cg_coord_read(iFile, iBase, iZone, "CoordinateY", coordType, &one,
 //                    &nodesNum, coordY) != CG_OK)
 //    	Terminate("readCoordY", cg_get_error());
 //    if(cg_coord_read(iFile, iBase, iZone, "CoordinateZ", coordType, &one,
 //                    &nodesNum, coordZ) != CG_OK)
 //    	Terminate("readCoordZ", cg_get_error());
 //    this->nodes_.copy(new Nodes(coordX, coordY, coordZ, nodesNum));

 //    /// read section information
 //    label nSections;
 //    if(cg_nsections(iFile, iBase, iZone, &nSections) != CG_OK)
 //    	Terminate("readNSection", cg_get_error());
 //    char secName[20];
 //    label iStart, iEnd, nBnd, parentFlag;
 //    ElementType_t secType;
	// for (int iSec = 1; iSec <= nSections; ++iSec)
	// {
	// 	Section sec;
	// 	if(cg_section_read(iFile, iBase, iZone, iSec, secName,
	// 		&secType, &iStart, &iEnd, &nBnd, &parentFlag) != CG_OK)
	// 		Terminate("readSectionInfo", cg_get_error());
	// 	if(!Section::compareEleType((int)secType, this->meshType_)) continue;
	// 	sec.name = secName;
	// 	sec.type = secType;
	// 	sec.iStart = iStart;
	// 	sec.iEnd = iEnd;
	// 	sec.nBnd = nBnd;
	// 	sec.num  = iEnd-iStart+1;
	// 	sec.conn = new label[Section::nodesNumForEle(secType)*sec.num];
 //        if(cg_elements_read(iFile, iBase, iZone, iSec, sec.conn, NULL) != CG_OK)
 //       		Terminate("readConnectivity", cg_get_error());
	// 	this->secs_.push_back(sec);
	// 	par_std_out_("Section: %d, name: %s, type: %d, start: %d, end: %d\n", iSec, secName, secType, iStart, iEnd);
	// }
 //    if(cg_close(iFile) != CG_OK)
 //    	Terminate("closeGridCGNS", cg_get_error());
}

char* Mesh::typeToString(ElementType_t eleType)
{
    switch(eleType)
    {
        case HEXA_8 : return "HEXA_8";
        case HEXA_27: return "HEXA_27";
        case QUAD_4 : return "QUAD_4";
        case TETRA_4 : return "TETRA_4";
        case TRI_3: return "TRI_3";
        case QUAD_9: return "QUAD_9";
        // default: return "unknown type";
        default: Terminate("transform type to string", "unknown type");
    }
}

void Mesh::fetchNodes(char* filePtr)
{
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int iFile;
    int iBase=1,iZone=1;
    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
    if(cgp_open(filePtr, CG_MODE_MODIFY, &iFile))
        Terminate("readBaseInfo", cg_get_error());

    Array<Array<label> > cell2NodeArr;
    transformArray(this->getTopology().getCell2Node(), cell2NodeArr);

    label nodeNum = this->nodes_.size();
    label *nodeStartId = new label[nprocs+1];
    MPI_Allgather(&nodeNum, 1, MPI_LABEL, &nodeStartId[1], 1, MPI_LABEL, MPI_COMM_WORLD);
    nodeStartId[0] = 0;
    for (int i = 1; i <= nprocs; ++i)
    {
        nodeStartId[i] += nodeStartId[i-1];
    }

    int nBlocks = nprocs;
    // 找出本进程所需node局部编号
    Array<Array<label> > nodeINeed(nBlocks);
    for (int i = 0; i < cell2NodeArr.size(); ++i)
    {
        for (int j = 0; j < cell2NodeArr[i].size(); ++j)
        {
            for (int k = 0; k < nBlocks; ++k)
            {
                if(cell2NodeArr[i][j]<=nodeStartId[k+1] && cell2NodeArr[i][j]>nodeStartId[k])
                    nodeINeed[k].push_back(cell2NodeArr[i][j]-nodeStartId[k]-1);
            }
        }
    }

    // 排序去重
    for (int i = 0; i < nBlocks; ++i)
    {
        sort(nodeINeed[i].begin(),nodeINeed[i].end());
        nodeINeed[i].erase(unique(nodeINeed[i].begin(), nodeINeed[i].end()), nodeINeed[i].end());
    }

    scalar *x = new scalar[nodeStartId[1]-nodeStartId[0]];
    scalar *y = new scalar[nodeStartId[1]-nodeStartId[0]];
    scalar *z = new scalar[nodeStartId[1]-nodeStartId[0]];
    Array<Array<scalar> > xarr(nBlocks), yarr(nBlocks), zarr(nBlocks);
    label idxStart[nBlocks];
    label id = 0;
    for (int i = 0; i < nBlocks; ++i)
    {
        // label idx = (rank+i)%nBlocks;
        // printf("%d, %d, %d\n", rank, idx, i);
        cgsize_t start = nodeStartId[i]+1;
        cgsize_t end = nodeStartId[i+1];
        if(cgp_coord_read_data(iFile, iBase, iZone, 1, &start, &end, x) ||
            cgp_coord_read_data(iFile, iBase, iZone, 2, &start, &end, y) ||
            cgp_coord_read_data(iFile, iBase, iZone, 3, &start, &end, z))
            Terminate("readCoords", cg_get_error());
        for (int j = 0; j < nodeINeed[i].size(); ++j)
        {
            coordMap_.insert(pair<label, label>(nodeINeed[i][j]+nodeStartId[i], id));
            xarr[i].push_back(x[nodeINeed[i][j]]);
            yarr[i].push_back(y[nodeINeed[i][j]]);
            zarr[i].push_back(z[nodeINeed[i][j]]);
            id++;
        }
        // idxStart[idx] = id;
        MPI_Barrier(MPI_COMM_WORLD);
    }

    Array<scalar> coordX, coordY, coordZ;
    for (int i = 0; i < nBlocks; ++i)
    {
        coordX.insert(coordX.end(), xarr[i].begin(), xarr[i].end());
        coordY.insert(coordY.end(), yarr[i].begin(), yarr[i].end());
        coordZ.insert(coordZ.end(), zarr[i].begin(), zarr[i].end());
        if(i>0) idxStart[i] += idxStart[i-1];
    }
    this->ownNodes_.copy(new Nodes(coordX, coordY, coordZ));

    // for (it = coordMap_.begin(); it != coordMap_.end() ; ++it)
    // {
    //     for (int i = 0; i < nBlocks; ++i)
    //     {
    //         if(it->first<=nodeStartId[i+1] && it->first>nodeStartId[k])
    //     }
    // }

    // label *bonus = new label[nBlocks];
    // bonus[0] = 0;
    // for (int i = 1; i < nBlocks; ++i)
    // {
    //     bonus[i] = bonus[i-1]+nodeINeed[i-1].size();
    // }
    // Table<label, label> coordBonus;
    // Table<label, label>::iterator it;
    // for (int i = 0; i < cell2NodeArr.size(); ++i)
    // {
    //     for (int j = 0; j < cell2NodeArr[i].size(); ++j)
    //     {
    //         // it=localCellMap.find(cell2NodeArr[i][j]);
    //         // if(it!=localCellMap.end()) 
    //         // {
    //         //     cell2NodeArr[i][j] = it->second;
    //         //     continue;
    //         // }
    //         for (int k = 0; k < nBlocks; ++k)
    //         {
    //             if(cell2NodeArr[i][j]<=nodeStartId[k+1] && cell2NodeArr[i][j]>nodeStartId[k])
    //             {
    //                 label originId = cell2NodeArr[i][j]-1;
    //                 it = coordMap_.find(originId);
    //                 if(it!=coordMap_.end())
    //                 {
    //                     // cell2NodeArr[i][j] = it->second+bonus[k]+1;
    //                     // it->second += bonus[k];
    //                     coordMap_[originId] = it->second+bonus[k];
    //                 } else
    //                 {
    //                     Terminate("rearrange cell2Node", "the relative index is miss");
    //                 }
    //                 // localCellMap.insert(pair<label, label>(cell2NodeArr[i][j], bonus[k]));
    //                 // cell2NodeArr[i][j] = bonus[k]++;
    //             }
    //         }
    //     }
    // }

    Table<label, label>::iterator it;
    for (int i = 0; i < cell2NodeArr.size(); ++i)
    {
        for (int j = 0; j < cell2NodeArr[i].size(); ++j)
        {
            label originId = cell2NodeArr[i][j]-1;
            it = coordMap_.find(originId);
            if(it!=coordMap_.end())
            {
                cell2NodeArr[i][j] = it->second+1;
            } else
            {
                Terminate("rearrange cell2Node", "the relative index is miss");
            }
        }
    }
    // if(rank==1)
    // {
    //     for (int i = 0; i < coordX.size(); ++i)
    //     {
    //         printf("%d, %f, %f, %f\n", i, coordX[i], coordY[i], coordZ[i]);
    //     }
    // }
    // if(rank==1)
    // {
    //     for (int i = 0; i < cell2NodeArr.size(); ++i)
    //     {
    //         printf("the %dth elements: ", i);
    //         for (int j = 0; j < cell2NodeArr[i].size(); ++j)
    //         {
    //             printf("%d, ", cell2NodeArr[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }

    // ArrayArray<label> cell2Node = transformArray(cell2NodeArr);
    // this->getTopology().setCell2Node(cell2Node);

    DELETE_POINTER(z);
    DELETE_POINTER(y);
    DELETE_POINTER(x);
    DELETE_POINTER(nodeStartId);

    if(cgp_close(iFile))
        Terminate("closeCGNSFile",cg_get_error());
}

} //end namespace HSF