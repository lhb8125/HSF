/**
* @file: mesh.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-25 11:21:52
* @last Modified by:   lenovo
* @last Modified time: 2020-01-10 16:02:03
*/
#include <cstdio>
#include <iostream>
#include <typeinfo>
#include <stdlib.h>
#include <algorithm>
#include <assert.h>
#include "mpi.h"
#include "pcgnslib.h"
#include "utilities.hpp"
#include "mesh.hpp"
#include "cgnslib.h"

// #define INTEGER32
namespace HSF
{

void Mesh::readCGNSFilePar(const char* filePtr, int fileIdx)
{
	int rank, numProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	par_std_out_("This is rank %d in %d processes\n", rank, numProcs);

	int iFile, nBases=0, cellDim=0, physDim=0;
	int iBase=1, iZone=1;
	char basename[CHAR_DIM];

	if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK ||
        cgp_pio_mode(CGP_INDEPENDENT) != CG_OK)
		Terminate("initCGNSMPI", cg_get_error());
	if(cgp_open(filePtr, CG_MODE_READ, &iFile) ||
        cg_nbases(iFile, &nBases) ||
        cg_base_read(iFile, iBase, basename, &cellDim, &physDim))
		Terminate("readBaseInfo", cg_get_error());
        // cgp_error_exit();
	par_std_out_("nBases: %d, basename: %s, cellDim: %d, physDim: %d\n", nBases, basename, cellDim, physDim);
	MPI_Barrier(MPI_COMM_WORLD);

    int precision;
    cg_precision(iFile, &precision);
    par_std_out_("precision: %d\n", precision);

	int nZones;
	if(cg_nzones(iFile, iBase, &nZones))
		Terminate("readZoneInfo", cg_get_error());
	par_std_out_("nZones: %d\n", nZones);

    this->nodeNumGlobal_.push_back(0);
    this->eleNumGlobal_.push_back(0);
    for (int i = 0; i < nZones; ++i)
    {
        readOneZone(iFile, iBase, i+1);
    }

	if(cgp_close(iFile))
		Terminate("closeCGNSFile",cg_get_error());

    // if(this->meshType_==Boco) return;
    Array<Array<Array<label64> > > zc_pnts(nZones),zc_donor_pnts(nZones);
    Array<Array<char*> > zc_name(nZones);
    Array<Array<Array<Array<label64> > > > nodes(nZones);
    readZoneConnectivity(filePtr,zc_pnts,zc_donor_pnts,zc_name, nodes);
    // if(this->meshType_==Inner) 
        mergeInterfaceNodes(zc_pnts, zc_donor_pnts, zc_name, nodes);
    if(this->meshType_==Boco)
        removeInterfaceNodes(zc_pnts);
    // DELETE_POINTER(node);

}

void Mesh::readOneZone(const int iFile, const int iBase, const int iZone)
{
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    par_std_out_("This is rank %d in %d processes\n", rank, numProcs);

    cgsize_t sizes[3];
    char *zoneName = new char[CHAR_DIM];
    ZoneType_t zoneType;
    if(cg_zone_read(iFile, iBase, iZone, zoneName, sizes) ||
        cg_zone_type(iFile, iBase, iZone, &zoneType) ||
        zoneType != Unstructured)
        Terminate("readZoneInfo", cg_get_error());
    par_std_out_("Zones: %d, zoneName: %s, zoneType: %d, nodeNum, %d, eleNum: %d, bndEleNum: %d\n", iZone, zoneName, zoneType, sizes[0], sizes[1], sizes[2]);
    this->eleNum_  += sizes[1];
    this->zoneName_.push_back(zoneName);

    int precision;
    cg_precision(iFile, &precision);
    int nCoords;
    if(cg_ncoords(iFile, iBase, iZone, &nCoords) ||
        nCoords != 3)
        Terminate("readNCoords", cg_get_error());
    int nnodes = (sizes[0] + numProcs - 1) / numProcs;
    cgsize_t start  = nnodes * rank + 1;
    cgsize_t end    = nnodes * (rank + 1);
    if (end > sizes[0]) end = sizes[0];
    nnodes = end - start + 1;
    Array<scalar*> coords;
    par_std_out_("The vertices range of processor %d is (%d, %d). \n", rank, start, end);
    DataType_t dataType;
    char coordName[CHAR_DIM];
    vector<scalar> vecx(nnodes);
    vector<scalar> vecy(nnodes);
    vector<scalar> vecz(nnodes);
    scalar* x = &vecx[0];
    scalar* y = &vecy[0];
    scalar* z = &vecz[0];
    if(cg_coord_info(iFile, iBase, iZone, 1, &dataType, coordName) ||
        // sizeof(dataType)!=sizeof(scalar) ||
        cgp_coord_read_data(iFile, iBase, iZone, 1, &start, &end, x))
    {
        printf("%d\n", iZone);
        Terminate("readCoords", cg_get_error());
    }
    if(dataType==RealSingle && sizeof(scalar)==8)
        Terminate("readCoords","The data type of scalar does not match");
    if(dataType==RealDouble && sizeof(scalar)==4)
        Terminate("readCoords","The data type of scalar does not match");
    if(cg_coord_info(iFile, iBase, iZone, 2, &dataType, coordName) ||
        // sizeof(dataType)!=sizeof(scalar) ||
        cgp_coord_read_data(iFile, iBase, iZone, 2, &start, &end, y))
        Terminate("readCoords", cg_get_error());
    if(cg_coord_info(iFile, iBase, iZone, 3, &dataType, coordName) ||
        // sizeof(dataType)!=sizeof(scalar) ||
        cgp_coord_read_data(iFile, iBase, iZone, 3, &start, &end, z))
        Terminate("readCoords", cg_get_error());
    MPI_Barrier(MPI_COMM_WORLD);
    // par_std_out_("%d\n", sizeof(dataType));

    // Array<label64> local_2_global;
    // Array<label64> global_2_local;
    // vector<label64> realNode;
    label nodeStartId;
    nodeStartId = this->nodeNumGlobal_[iZone-1];
    // for (int i = 0; i < nnodes; ++i)
    // {
    //     local_2_global.push_back(i);
    //     global_2_local.push_back(i);
    //     realNode.push_back(i+nodeStartId+1);
    // }

    // // 本zone所有与其他zone的连接面
    // for (int i = 0; i < zc_name.size(); ++i)
    // {
    //     // 之前所记录的zone
    //     for (int j = 0; j < this->zoneName_.size(); ++j)
    //     {
    //         // 如果名字相同，则这两个zone相连接
    //         if(zc_name[i]==zoneName_[j])
    //         {
    //             // 如果某连接面上的点位于本进程中
    //             for (int k = 0; k < zc_pnts.size(); ++k)
    //             {
    //                 if(zc_pnts[i][k]<end && zc_pnts[i][k]>=start)
    //                 {
    //                     local_2_global.erase(local_2_global.begin()+k);
    //                     global_2_local[k] = -1;
    //                     // vecx.erase(vecx.begin()+k);
    //                     // vecy.erase(vecy.begin()+k);
    //                     // vecz.erase(vecz.begin()+k);
    //                     realNode[zc_pnts[i][k]-1]
    //                         = nodeNumGlobal_[j]
    //                         + node_global_2_local_[j][zc_donor_pnts[i][k]-1];
    //                         + 1;
    //                 } else
    //                 {
    //                     printf("%d,%d,%d,%d,%d\n", i,k,zc_pnts[i][k],start,end);
    //                     Terminate("error","error");
    //                 }
    //             }
    //         }
    //     }
    // }
    // int absentNodes=0;
    // // 消除属于交界面上其他zone已存储的节点，记录映射
    // for (int i = 0; i < nnodes; ++i)
    // {
    //     if(global_2_local[i]==-1) absentNodes++;
    //     else global_2_local[i] -= absentNodes;
    // }
    // this->node_local_2_global_.push_back(local_2_global);
    // this->node_global_2_local_.push_back(global_2_local);
    nnodes = vecx.size();
    // Nodes *node = new Nodes(x, y, z, nnodes);
    Nodes node(x, y, z, nnodes);
    this->nodes_.add(&node);
    // this->nodes_.setStart(start);
    // this->nodes_.setEnd(end);
    this->nodeStartIdx_.push_back(start);
    this->nodeEndIdx_.push_back(start+nnodes-1);
    this->nodeNumLocal_.push_back(nnodes);
    this->nodeNumGlobal_.push_back(this->nodeNumGlobal_.back()+sizes[0]);
    label zoneEleNum = sizes[1];
    par_std_out_("zone: %d, start: %d, nnodes: %d\n", iZone, this->nodeNumGlobal_.back(),this->nodes_.size());
    // this->nodeNumPerZone_.push_back(sizes[0]);
    // par_std_out_("Zone: %d, node start: %d, ")
    // node.setStart(start);
    // node.setEnd(end);
    // this->nodes.push_back(node);
    // int i,j,k,n,nn,ne;
    // nn = 0;
    // if(rank==0){
        
    // }
    int nSecs;
    if(cg_nsections(iFile, iBase, iZone, &nSecs))
        Terminate("readNSections", cg_get_error());
    int iSec;
    par_std_out_("nSecs: %d\n", nSecs);
    for (int iSec = 1; iSec <= nSecs; ++iSec)
    {
        char secName[CHAR_DIM];
        cgsize_t start, end;
        ElementType_t type;
        int nBnd, parentFlag;
        if(cg_section_read(iFile, iBase, iZone, iSec, secName, 
            &type, &start, &end, &nBnd, &parentFlag))
            Terminate("readSectionInfo", cg_get_error());
        // par_std_out_("%d\n", this->meshType_);
        zoneEleNum += end-start+1;
        if(! Section::compareEleType(type, this->meshType_)) continue;
        par_std_out_("iSec: %d, sectionName: %s, type: %d, start: %d, end: %d, nBnd: %d\n", iSec, secName, type, start, end, nBnd);

        Section sec;
        sec.name = new char[CHAR_DIM];
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
        sec.iZone  = iZone-1;
        map<label64,label64>::iterator iter;
        if(precision==64)
        {
            par_std_out_("This is 64 precision\n");
            sec.conn   = (label*)elements;
            for (int i = 0; i < sec.num*Section::nodesNumForEle(type); ++i)
            {
                // label64 tmp = sec.conn[i]+nodeStartId;
                // sec.conn[i] = realNode[sec.conn[i]-1];
                // printf("%d,%d,%d\n", i,tmp,sec.conn[i]);
                // iter = realNode.find(sec.conn[i]);
                // if(iter!=realNode.end())
                    // sec.conn[i] = iter->second;
                // else
                sec.conn[i] += nodeStartId;
            }
        } else if(precision==32)
        {
            int *eles32 = (int*)&elements[0];
            label* eles64 = new label[nEles*Section::nodesNumForEle(type)];
            for (int i = 0; i < nEles*Section::nodesNumForEle(type); ++i)
            {
                eles64[i] = (label)eles32[i] + nodeStartId;
                // eles64[i] = (label)eles32[i];
            }
            sec.conn = eles64;
            eles32 = NULL;
            eles64 = NULL;
            DELETE_POINTER(elements);
        }
        this->secs_.push_back(sec);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    this->eleNumGlobal_.push_back(this->eleNumGlobal_.back()+zoneEleNum);

    // DELETE_POINTER(z);
    // DELETE_POINTER(y);
    // DELETE_POINTER(x);
}

void Mesh::writeCGNSFilePar(const char* filePtr)
{
    int idx = 0;
	int nodesPerSide = 5;
	int nodeNum = 0;
    // for (int i = 0; i < this->nodeNumGlobal_.size(); ++i)
    // {
    //     nodeNum += this->nodeNumGlobal_[i];
    // }
    nodeNum = this->nodeNumGlobal_.back();
	int eleNum  = this->eleNum_;

	int rank, numProcs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	// par_std_out_("This is rank %d in %d processes\n", rank, numProcs);

	int iFile, nBases, cellDim, physDim, Cx, Cy, Cz;
	int iBase=1, iZone=1;
	char basename[CHAR_DIM];

	cgsize_t sizes[3];
	sizes[0] = nodeNum;
	sizes[1] = eleNum;
	sizes[2] = 0;

	if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK ||
        cgp_pio_mode(CGP_INDEPENDENT) != CG_OK)
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
    // cgsize_t start  = this->nodes_.getStart();
    // cgsize_t end  = this->nodes_.getEnd();
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

    // printf("%d, %d, %d\n", start, end, this->nodes_.ize());

    // /* write the coordinate data in parallel */
    int coordIdx = 0;
    for (int i = 0; i < this->nodeStartIdx_.size(); ++i)
    {
        // cgsize_t start  = this->nodeNumGlobal_[i]+this->nodeStartIdx_[i];
        cgsize_t start  = this->nodeStartIdx_[i]+this->nodeNumGlobal_[i];
        cgsize_t end    = this->nodeEndIdx_[i]+this->nodeNumGlobal_[i];
        // if(i>0) 
        // {
        //     start += this->nodeNumGlobal_[i-1];
        //     end   += this->nodeNumGlobal_[i-1];
        // }
        scalar* tmpX = &x[coordIdx];
        scalar* tmpY = &y[coordIdx];
        scalar* tmpZ = &z[coordIdx];
        par_std_out_("from %d to %d, num: %d\n", start, end, nnodes);
        // for (int j= 0; j < end-start+1; ++j)
        // {
        //     printf("%f, %f, %f\n", tmpX[j], tmpY[j], tmpZ[j]);
        // }
        if (cgp_coord_write_data(iFile, iBase, iZone, Cx, &start, &end, tmpX) ||
            cgp_coord_write_data(iFile, iBase, iZone, Cy, &start, &end, tmpY) ||
            cgp_coord_write_data(iFile, iBase, iZone, Cz, &start, &end, tmpZ))
            Terminate("writeCoords", cg_get_error());
        coordIdx += end-start+1;
        MPI_Barrier(MPI_COMM_WORLD);
    }
 //    if (cgp_coord_write_data(iFile, iBase, iZone, Cx, &start, &end, x) ||
 //        cgp_coord_write_data(iFile, iBase, iZone, Cy, &start, &end, y) ||
 //        cgp_coord_write_data(iFile, iBase, iZone, Cz, &start, &end, z))
 //        Terminate("writeCoords", cg_get_error());
	// MPI_Barrier(MPI_COMM_WORLD);

    Array<label> cellBlockStartIdx = this->getBlockTopology().getCellBlockStartIdx();
    ArrayArray<label> conn = this->getBlockTopology().getCell2Node();
    Array<label> cellType = this->getBlockTopology().getCellType();
    par_std_out_("write %d sections\n", cellBlockStartIdx.size()-1);

    label *cellStartId = new label[numProcs+1];
    cellStartId[0] = 0;
    for (int iSec = 0; iSec < cellBlockStartIdx.size()-1; ++iSec)
    {
        label num = cellBlockStartIdx[iSec+1]-cellBlockStartIdx[iSec];
        par_std_out_("%d\n", iSec);
        MPI_Allgather(&num, 1, MPI_LABEL, &cellStartId[1], 1, MPI_LABEL, MPI_COMM_WORLD);
        par_std_out_("%d\n", num);
        for (int i = 0; i < numProcs; ++i)
        {
            cellStartId[i+1] += cellStartId[i];
        }
        ElementType_t eleType = (ElementType_t)cellType[iSec];
        // par_std_out_("writeSecInfo\n");
        int S;
        if(cgp_section_write(iFile, iBase, iZone, Section::typeToWord(eleType),
            eleType, cellStartId[0]+1, cellStartId[numProcs], 0, &S))
            Terminate("writeSecInfo", cg_get_error());
        par_std_out_("writeSecInfo\n");

        cgsize_t *data = (cgsize_t*)&conn.data[conn.startIdx[cellBlockStartIdx[iSec]]];
        // conn.display();
        // 如果该block内无网格单元，则跳过
        if(num==0)
        {
            // par_std_out_("rank: %d, start: %d, end: %d\n",rank, cellStartId[rank]+1,cellStartId[rank]+1);
            // if(cellStartId[rank]+1>=cellStartId[numProcs]) cellStartId[rank]=cellStartId[numProcs]-1;
            // if(cgp_elements_write_data(iFile, iBase, iZone, S, cellStartId[rank]+1, cellStartId[rank]+1, NULL))
            //     Terminate("writeSecConn", cg_get_error());
        }else 
        {
            par_std_out_("rank: %d, start: %d, end: %d\n",rank, cellStartId[rank]+1,cellStartId[rank+1]);
            if(cgp_elements_write_data(iFile, iBase, iZone, S, cellStartId[rank]+1,
                cellStartId[rank+1], data))
                Terminate("writeSecConn", cg_get_error());
        }
        par_std_out_("writeSecConn\n");

        cellStartId[0] = cellStartId[numProcs];
    }

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
	char basename[CHAR_DIM];

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

    if(cgp_close(iFile))
        Terminate("closeCGNSFile",cg_get_error());
    
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



void Mesh::fetchNodes(char* filename)
{
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int nZones = this->nodeNumGlobal_.size()-1;
    int iFile;
    int iBase=1;
    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
    if(cgp_open(filename, CG_MODE_MODIFY, &iFile))
        Terminate("openFile", cg_get_error());


    Array<Array<label> > cell2NodeArr;
    transformArray(this->getTopology().getCell2Node(), cell2NodeArr);

    label* nodeStartId = new label[nprocs*nZones+1];
    label tmpIdx = 1;
    // 收集各进程node数量，并按照文件编号和进程编号顺序放置
    for (int iZone = 0; iZone < nZones; ++iZone)
    {
        label nodeNum = this->nodeNumLocal_[iZone];
        MPI_Allgather(&nodeNum, 1, MPI_LABEL, &nodeStartId[tmpIdx], 1, MPI_LABEL, MPI_COMM_WORLD);
        tmpIdx += nprocs;
    }
    // 递增方式对node数量进行编号
    nodeStartId[0] = 0;
    label maxNodeNum = 0;
    for (int i = 1; i <= nprocs*nZones; ++i)
    {
        maxNodeNum = maxNodeNum > nodeStartId[i] ? maxNodeNum : nodeStartId[i];
        nodeStartId[i] += nodeStartId[i-1];
    }

    int nBlocks = nprocs*nZones;
    // 找出本进程所需node局部编号
    Array<Array<label> > nodeINeed(nBlocks);
    for (int k = 0; k < nBlocks; ++k)
    {
        for (int i = 0; i < cell2NodeArr.size(); ++i)
        {
            // printf("cell: %d\n", i);
            for (int j = 0; j < cell2NodeArr[i].size(); ++j)
            {
                // 如果网格单元的格点落在该分段内，则记录该格点编号在该分段内的相对位置
                if(cell2NodeArr[i][j]<=nodeStartId[k+1] && cell2NodeArr[i][j]>nodeStartId[k])
                {
                    nodeINeed[k].push_back(cell2NodeArr[i][j]-1-nodeStartId[k]);
                    // nodeINeed[k].push_back(this->nodeNumPerZone_[cell2NodeArr[i][j]-1]);
                    // if(rank==0) printf("%d, %d, %d\n", i, k, cell2NodeArr[i][j]-nodeStartId[k]-1);
                }
            }
        }
    }

    // 排序去重
    for (int i = 0; i < nBlocks; ++i)
    {
        sort(nodeINeed[i].begin(),nodeINeed[i].end());
        nodeINeed[i].erase(unique(nodeINeed[i].begin(), nodeINeed[i].end()), nodeINeed[i].end());
    }

    scalar *x = new scalar[maxNodeNum];
    scalar *y = new scalar[maxNodeNum];
    scalar *z = new scalar[maxNodeNum];
    Array<Array<scalar> > xarr(nBlocks), yarr(nBlocks), zarr(nBlocks);
    label idxStart[nBlocks];
    label id = 0;
    for (int iZone = 1; iZone <= nZones; ++iZone)
    {
        for (int irank = 0; irank < nprocs; ++irank)
        {
            int iblock = (iZone-1)*nprocs+irank;
            // cgsize_t start = this->nodeStartIdx_[fileIdx];
            // cgsize_t end = this->nodeEndIdx_[fileIdx];
            cgsize_t start = nodeStartId[iblock]+1-this->nodeNumGlobal_[iZone-1];
            cgsize_t end   = nodeStartId[iblock+1]-this->nodeNumGlobal_[iZone-1];
            // int nFile = fileIdx;
            // while(nFile>0) 
            // {
            //     start -= this->nodeNumGlobal_[nFile-1];
            //     end   -= this->nodeNumGlobal_[nFile-1];
            //     nFile--;
            // }
            // printf("%d, %d, %d, %d\n", iZone, iblock, start, end);
            if(cgp_coord_read_data(iFile, iBase, iZone, 1, &start, &end, x) ||
                cgp_coord_read_data(iFile, iBase, iZone, 2, &start, &end, y) ||
                cgp_coord_read_data(iFile, iBase, iZone, 3, &start, &end, z))
                Terminate("readCoords", cg_get_error());
            for (int j = 0; j < nodeINeed[iblock].size(); ++j)
            {
                // node的原始绝对编号与本地编号的映射
                coordMap_.insert(pair<label, label>(nodeINeed[iblock][j]+nodeStartId[iblock], id));
                xarr[iblock].push_back(x[nodeINeed[iblock][j]]);
                yarr[iblock].push_back(y[nodeINeed[iblock][j]]);
                zarr[iblock].push_back(z[nodeINeed[iblock][j]]);
                id++;
            }
            // printf("%d, %d\n", fileIdx, id);
            // idxStart[idx] = id;
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    // printf("rank: %d, local node num: %d\n", rank, id);

    Array<scalar> coordX, coordY, coordZ;
    for (int i = 0; i < nBlocks; ++i)
    {
        coordX.insert(coordX.end(), xarr[i].begin(), xarr[i].end());
        coordY.insert(coordY.end(), yarr[i].begin(), yarr[i].end());
        coordZ.insert(coordZ.end(), zarr[i].begin(), zarr[i].end());
        if(i>0) idxStart[i] += idxStart[i-1];
    }
    Nodes node(coordX, coordY, coordZ);
    this->ownNodes_ = new Nodes(coordX, coordY, coordZ);

    DELETE_POINTER(z);
    DELETE_POINTER(y);
    DELETE_POINTER(x);
    DELETE_POINTER(nodeStartId);

    if(cgp_close(iFile))
        Terminate("closeCGNSFile",cg_get_error());
}

void Mesh::readZoneConnectivity(const char* filePtr,
    Array<Array<Array<label64> > >& zc_pnts,
    Array<Array<Array<label64> > >& zc_donor_pnts,
    Array<Array<char*> >& zc_name,
    Array<Array<Array<Array<label64> > > >& nodes)
{
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK ||
        cgp_pio_mode(CGP_INDEPENDENT) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
    int iFile, iBase=1, iZone;
    int nBases;
    if(cgp_open(filePtr, CG_MODE_READ, &iFile))
        Terminate("readBaseInfo", cg_get_error());
    int nZones;
    if(cg_nzones(iFile, iBase, &nZones))
        Terminate("readZoneInfo", cg_get_error());

    for (int iZone = 1; iZone <= nZones; ++iZone)
    {
        label32 nconn;
        if(cg_nconns(iFile,iBase,iZone,&nconn))
            Terminate("read1to1Conn",cg_get_error());
        char connectname[CHAR_DIM];
        GridLocation_t location;
        GridConnectivityType_t connect_type;
        PointSetType_t ptset_type;
        cgsize_t npnts;
        ZoneType_t donor_zonetype;
        PointSetType_t donor_ptset_type;
        DataType_t donor_datatype;
        cgsize_t ndata_donor;
        int tot_pnts = 0;

        vector<vector<bool> > visits(nconn);

        for (int iconn = 1; iconn <= nconn; ++iconn)
        {
            char *donorname = new char[CHAR_DIM];
            if(cg_conn_info(iFile,iBase,iZone,iconn,connectname,&location,
                &connect_type,&ptset_type,&npnts,donorname,&donor_zonetype,
                &donor_ptset_type,&donor_datatype,&ndata_donor))
                Terminate("readConnInfo",cg_get_error());
            // if(rank==0) printf("connectname: %s, location: %s, connect_type: %s\n",
                // connectname,  Section::GridLocationToWord(location),
                // Section::ConnTypeToWord(connect_type));
            // if(rank==0) printf("ptset_type: %s, donorname: %s, donor_zonetype: %d\n",
                // Section::PtSetToWord(ptset_type), donorname, donor_zonetype);
            // 周期性边界会触发此报警，待处理
            // if(donor_zonetype!=Unstructured || ptset_type!=PointList 
            //     || donor_datatype!=LongInteger || npnts!=ndata_donor)
            //     Terminate("readConnInfo","The zone type is not supported");
            Array<label64> pnts(npnts);
            Array<label64> donor_pnts(ndata_donor);
            // cgsize_t *pnts = new cgsize_t[npnts];
            // cgsize_t *donor_pnts = new cgsize_t[ndata_donor];
            if(cg_conn_read(iFile,iBase,iZone,iconn,(cgsize_t*)&pnts[0],
                donor_datatype, (cgsize_t*)&donor_pnts[0]))
                Terminate("readConn",cg_get_error());
            zc_pnts[iZone-1].push_back(pnts);
            zc_donor_pnts[iZone-1].push_back(donor_pnts);
            zc_name[iZone-1].push_back(donorname);
            tot_pnts += npnts;
            vector<vector<label64> > tmpNodes(npnts);
            nodes[iZone-1].push_back(tmpNodes);
            vector<bool> tmpVisits(npnts, false);
            visits[iconn-1] = tmpVisits ;
            // if(rank==0) printf("iZone: %d, iconn: %d\n", iZone, iconn);
        }

        // 读取section信息
        int nSecs;
        if(cg_nsections(iFile, iBase, iZone, &nSecs))
            Terminate("readNSections", cg_get_error());
        int iSec;
        for (int iSec = 1; iSec <= nSecs; ++iSec)
        {
            char secName[CHAR_DIM];
            cgsize_t start, end;
            ElementType_t type;
            int nBnd, parentFlag;
            if(cg_section_read(iFile, iBase, iZone, iSec, secName, 
                &type, &start, &end, &nBnd, &parentFlag))
                Terminate("readSectionInfo", cg_get_error());
            if(! Section::compareEleType(type, Boco)) continue;
            par_std_out_("iSec: %d, sectionName: %s, type: %d, start: %d, end: %d, nBnd: %d\n", iSec, secName, type, start, end, nBnd);

            int nEles = end-start+1;
            int nodesNum = Section::nodesNumForEle(type);
            cgsize_t* elements = new cgsize_t[nEles*nodesNum];
            if(cgp_elements_read_data(iFile, iBase, iZone, iSec, start, end, elements))
                Terminate("readElements", cg_get_error());
            // 对于所有的二维单元
            for (int iele = 0; iele < nEles; ++iele)
            {
                // 对于所有的连接单元
                for (int iconn = 0; iconn < zc_pnts[iZone-1].size(); ++iconn)
                {
                    for (int ipnts = 0; ipnts < zc_pnts[iZone-1][iconn].size(); ++ipnts)
                    {
                        if(visits[iconn][ipnts]) continue;
                        // 如果他们相等
                        if(zc_pnts[iZone-1][iconn][ipnts]==iele+start)
                        {
                            visits[iconn][ipnts] = true;
                            // 对于该二维单元的所有节点
                            for (int j = 0; j < nodesNum; ++j)
                            {
                                nodes[iZone-1][iconn][ipnts].push_back(elements[iele*nodesNum+j]);
                            }
                        }
                    }
                }
            }
        }
    }
    if(cgp_close(iFile))
        Terminate("closeCGNSFile",cg_get_error());
}

void Mesh::removeInterfaceNodes(Array<Array<Array<label64> > >& zc_pnts)
{
    Array<label64> removeList;
    for (int iZone = 0; iZone < zc_pnts.size(); ++iZone)
    {
        for (int iconn = 0; iconn < zc_pnts[iZone].size(); ++iconn)
        {
            for (int ipnt = 0; ipnt < zc_pnts[iZone][iconn].size(); ++ipnt)
            {
                removeList.push_back(zc_pnts[iZone][iconn][ipnt]
                    + this->eleNumGlobal_[iZone]);
            }
        }
    }
    label secNum = this->secs_.size();
    /// record the type of cells
    for (int i = 0; i < secNum; ++i)
    {
        int nodeNum = Section::nodesNumForEle(secs_[i].type);
        label* newConn = new label[secs_[i].num*nodeNum];
        label newSecNum = 0;
        for (int j = 0; j < secs_[i].num; ++j)
        {
            Array<label64>::iterator result
                = find(removeList.begin(),removeList.end(),
                    this->eleNumGlobal_[secs_[i].iZone]+secs_[i].iStart+j);
            // 如果该单元不在交界面上
            if(result==removeList.end())
            {
                for (int k = 0; k < nodeNum; ++k)
                {
                    newConn[newSecNum*nodeNum+k] = secs_[i].conn[k+j*nodeNum];
                }
                newSecNum++;
            }
        }
        DELETE_POINTER(secs_[i].conn);
        secs_[i].conn = newConn;
        secs_[i].num = newSecNum;
    }
}

void Mesh::mergeInterfaceNodes(Array<Array<Array<label64> > >& zc_pnts,
    Array<Array<Array<label64> > >& zc_donor_pnts,
    Array<Array<char*> >& zc_name,
    Array<Array<Array<Array<label64> > > >& nodes)
{
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    for (int iZone = 0; iZone < zc_pnts.size(); ++iZone)
    {
        for (int idonor = 0; idonor < iZone; ++idonor)
        {
            for (int iconn = 0; iconn < zc_pnts[iZone].size(); ++iconn)
            {
                if(strcmp(zc_name[iZone][iconn],this->zoneName_[idonor])==0)
                {
                    // printf("izone: %d, iconn: %d, idonor: %d, name: %s\n", iZone, iconn, idonor, zc_name[iZone][iconn]);
                    for (int ipnt = 0; ipnt < zc_pnts[iZone][iconn].size(); ++ipnt)
                    {
                        vector<label64> localNodes =  nodes[iZone][iconn][ipnt];
                        vector<label64> neighborNodes
                            = findNeighborNodes(zc_pnts, idonor,
                            zc_donor_pnts[iZone][iconn][ipnt], nodes);
                        // printf("izone: %d, iconn: %d, idonor: %d, owner_pnt: %d\n", iZone, iconn, idonor, zc_donor_pnts[iZone][iconn][ipnt]);
                        assert(localNodes.size()==neighborNodes.size());
                        for (int inode = 0; inode < localNodes.size(); ++inode)
                        {
                            // 如果本地的节点没有被记录过，则记录该节点对应的之前存储过的节点
                            label64 local_node_global_id 
                                = this->nodeNumGlobal_[iZone]+localNodes[inode];
                            // if(local_node_global_id==653) printf("%d,%d,%d\n", iZone,idonor,rank);
                            // if(iZone==3 && idonor==0 && rank==0) printf("%d,%d,%d\n", inode,local_node_global_id,zc_node_map_.find(local_node_global_id)==zc_node_map_.end());
                            if(zc_node_map_.find(local_node_global_id)==zc_node_map_.end())
                            {
                                if(iZone==3 && idonor==0 && rank==0)
                                {
                                    // printf("The %d(%d) node in zone %d is replaced by the %d(%d) node in zone %d\n",
                                    //     localNodes[inode],local_node_global_id, iZone,
                                    //     neighborNodes[inode],this->nodeNumGlobal_[idonor]+neighborNodes[inode],idonor);
                                }
                                zc_node_map_[local_node_global_id]
                                    = this->nodeNumGlobal_[idonor]+neighborNodes[inode];
                            }
                        }
                    }
                }
            }
        }
    }
    label secNum = this->secs_.size();
    /// record the type of cells
    for (int i = 0; i < secNum; ++i)
    {
        for (int j = 0; j < secs_[i].num*Section::nodesNumForEle(secs_[i].type); ++j)
        {
            if(zc_node_map_.find(this->secs_[i].conn[j])!=zc_node_map_.end())
            {
                // printf("The %d node in section %d is replaced by the %d node\n", this->secs_[i].conn[j],i,zc_node_map_[this->secs_[i].conn[j]]);
                this->secs_[i].conn[j] = zc_node_map_[this->secs_[i].conn[j]];
            }
        }
    }
}

vector<label64>& Mesh::findNeighborNodes(Array<Array<Array<label64> > >& zc_pnts,
    int iZone, label64 owner_pnt, Array<Array<Array<Array<label64> > > >& nodes)
{
    // printf("iZone: %d\n", iZone);
    for (int iconn = 0; iconn < zc_pnts[iZone].size() ; ++iconn)
    {
        // printf("iconn: %d\n", iconn);
        for (int ipnt = 0; ipnt < zc_pnts[iZone][iconn].size(); ++ipnt)
        {
            // 如果本zone的交界面单元对应neighbor等于owner_pnt，则二者对应
            if(zc_pnts[iZone][iconn][ipnt]==owner_pnt)
            {
                // printf("%d,%d,%d,%d\n", ipnt,iconn, zc_pnts[iZone][iconn][ipnt],owner_pnt);
                // printf("%d,%d\n", iZone, iconn);
                return nodes[iZone][iconn][ipnt];
            }
        }
    }
}

} //end namespace HSF