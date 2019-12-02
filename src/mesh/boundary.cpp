/**
* @file: boundary.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-26 09:25:10
* @last Modified by:   lenovo
* @last Modified time: 2019-11-29 17:26:11
*/
#include "mpi.h"
#include "boundary.hpp"
// #include "loadBalancer.hpp"
#include "pcgnslib.h"

namespace HSF
{
    
void Boundary::readBoundaryCondition(const char* filePtr)
{
	int iFile;

	// open cgns file
    if(cg_open(filePtr, CG_MODE_READ, &iFile))
    	Terminate("readGridCGNS", cg_get_error());
    label iBase = 1;
    label iZone = 1;

    if(cg_goto(iFile, iBase, "Zone", 1, "ZoneBC_t", 1, "BC_t", 1, "end"))
    	Terminate("goZoneBC", cg_get_error());

    // read boundary condition
    int nBocos;
    if(cg_nbocos(iFile, iBase, iZone, &nBocos))
        Terminate("readNBocos", cg_get_error());

    char bocoName[40];
    int normalIndex[3];
    PointSetType_t ptsetType[1];
    cgsize_t normalListSize;
    int nDataSet;
    DataType_t normDataType;
    for(int iBoco=1; iBoco<=nBocos; iBoco++)
    {
        BCSection BCSec;
        /* Read the info for this boundary condition. */
        if(cg_boco_info(iFile, iBase, iZone, iBoco, BCSec.name, &BCSec.type,
            BCSec.ptsetType, &BCSec.nBCElems, &normalIndex[0], &normalListSize, &normDataType,
            &nDataSet))
            Terminate("readBocoInfo", cg_get_error());
        if(BCSec.ptsetType[0]!=PointRange || BCSec.ptsetType[0]!=PointList)
        {
            if(BCSec.ptsetType[0]==ElementRange) BCSec.ptsetType[0] = PointRange;
            else if(BCSec.ptsetType[0]==ElementList) BCSec.ptsetType[0] = PointList;
            if(cg_boco_gridlocation_read(iFile, iBase, iZone, iBoco, &BCSec.location))
                Terminate("readGridLocation", cg_get_error());
            if(BCSec.location==CellCenter)
            {
                par_std_out_("The boundary condition is defined on CellCenter\n");
            }
            else if(BCSec.location==EdgeCenter)
            {
                Terminate("readGridLocation","The boundary condition is defined on EdgeCenter\n");
            }
            else if(BCSec.location==Vertex)
            {
                BCSec.location = CellCenter;
                par_std_out_("The boundary condition is defined on Vertex\n");
            }
            else
            {
                par_std_out_("!!!!Error!!!!vertex: %d, FaceCenter: %d, EdgeCenter: %d, location: %d\n", Vertex, FaceCenter, EdgeCenter, BCSec.location);
            }
        }
        BCSec.BCElems = new cgsize_t[BCSec.nBCElems];
        if(cg_boco_read(iFile, iBase, iZone, iBoco, BCSec.BCElems, NULL))
        {
            Terminate("readBocoInfo", cg_get_error());
        }
        if(BCSec.ptsetType[0]==PointRange)
        {
            par_std_out_("iBoco: %d, name: %s, type: %d, nEles: %d, start: %d, end: %d\n", iBoco, BCSec.name, BCSec.type, BCSec.nBCElems, BCSec.BCElems[0], BCSec.BCElems[1]);
        }
        else if(BCSec.ptsetType[0]==PointList)
        {
            par_std_out_("iBoco: %d, name: %s, type: %d, nEles: %d\n", iBoco, BCSec.name, BCSec.type, BCSec.nBCElems);
        }
        this->BCSecs_.push_back(BCSec);
    }    
    if(cg_close(iFile))
        Terminate("closeCGNSFile", cg_get_error());
}

void Boundary::writeBoundaryCondition(const char* filePtr)
{
    int iFile;

    // open cgns file
    if(cg_open(filePtr, CG_MODE_WRITE, &iFile))
        Terminate("readGridCGNS", cg_get_error());
    label iBase = 1;
    label iZone = 1;

    if(cg_goto(iFile, iBase, "Zone", 1, "ZoneBC_t", 1, "BC_t", 1, "end"))
        Terminate("goZoneBC", cg_get_error());

    int nBocos = this->BCSecs_.size();
    int iBoco;
    for (int i = 0; i < nBocos; ++i)
    {
        // if(this->BCSecs_[i].ptsetType[0]==PointRange) par_std_out_("PointRange\n");
        // else if(this->BCSecs_[i].ptsetType[0]==ElementRange) par_std_out_("ElementRange\n");
        // par_std_out_("PointRange: %d, ElementRange: %d\n", PointRange, ElementRange);
        if(cg_boco_write(iFile, iBase, iZone, this->BCSecs_[i].name,
            this->BCSecs_[i].type, this->BCSecs_[i].ptsetType[0], 
            this->BCSecs_[i].nBCElems, this->BCSecs_[i].BCElems, &iBoco))
            Terminate("writeBC", cg_get_error());
        if(cg_boco_gridlocation_write(iFile, iBase, iZone, iBoco, this->BCSecs_[i].location))
            Terminate("writeGridLocation",cg_get_error());
    }
    if(cg_close(iFile))
        Terminate("closeCGNSFile", cg_get_error());
}

void Boundary::exchangeBoundaryElements(Topology& innerTopo)
{
    int rank,nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    ArrayArray<label> cell2Node = innerTopo.getCell2Node();
    Array<label> cellType = innerTopo.getCellType();
    label cellStartId = innerTopo.getCellStartId();

    ArrayArray<label> face2NodeBnd = getTopology().getCell2Node();
    // par_std_out_("%d,%d\n", getSections().size(),this->secs_.size());
    Array<Array<label> > face2NodeBndArr;
    transformArray(face2NodeBnd, face2NodeBndArr);

    label cellNum = cell2Node.size();
    Array<Array<label> > faces2NodesTmp;
    for (int i = 0; i < cellNum; ++i)
    {
        label faceNumTmp = Section::facesNumForEle(cellType[i]);
        for (int j = 0; j < faceNumTmp; ++j)
        {
            Array<label> face2NodeTmp = Section::faceNodesForEle(
                &cell2Node.data[cell2Node.startIdx[i]], cellType[i], j);
            sort(face2NodeTmp.begin(), face2NodeTmp.end());
            // 局部编号
            face2NodeTmp.push_back(i+cellStartId);
            faces2NodesTmp.push_back(face2NodeTmp);
        }
    }

    // 对所有面单元进行排序
    Array<label> weight;
    for (int i = 0; i < faces2NodesTmp.size(); ++i)
    {
        weight.push_back(faces2NodesTmp[i][0]);
    }
    sort(faces2NodesTmp, weight);
    // quicksortArray(faces2NodesTmp, 0, faces2NodesTmp.size()-1);
    // 由面与node之间关系寻找cell与cell之间关系
    // 删除重复的内部面
    // 找到面与cell之间关系
    bool* isInner = new bool[faces2NodesTmp.size()];
    Array<Array<label> > face2NodeOwn, face2NodeNei;
    Array<label> face2CellOwn;
    // for (int i = 0; i < faces2NodesTmp.size(); ++i){isInner[i] = false;}
    for (int i = 0; i < face2NodeBndArr.size(); ++i)
    {
        Array<label> tmp;
        tmp.assign(face2NodeBndArr[i].begin(), face2NodeBndArr[i].end());
        sort(tmp.begin(), tmp.end());
        // if(isInner[i]) continue;
        // int end = 0;
        int end = findSortedArray(faces2NodesTmp, tmp, 1, faces2NodesTmp.size()-1);
        // printf("%d, %d\n", i, end);
        if(end==-1)
        {
            face2NodeNei.push_back(face2NodeBndArr[i]);
        } else
        {
            label ownerId 
                = faces2NodesTmp[end][faces2NodesTmp[end].size()-1];
            // 局部编号
            face2CellOwn.push_back(ownerId);
            // 本地存储原始face2node顺序关系
            face2NodeOwn.push_back(face2NodeBndArr[i]);            
        }
        // /// 默认两个面不相等
        // bool isEqual = false;
        // while(end<faces2NodesTmp.size() && 
        //     tmp[0] != faces2NodesTmp[end][0]) {end++;}
        // while(end<faces2NodesTmp.size() && 
        //     tmp[0] == faces2NodesTmp[end][0])
        // {
        //     /// 第一个node相等时，默认相等
        //     isEqual = true;
        //     /// 如果维度不相等，则两个面不相等
        //     // if(face2NodeBndArr[i].size()!=faces2NodesTmp[end].size())
        //     // {
        //     //     isEqual = false;
        //     //     break;
        //     // }
        //     /// 比较各个维度，不相等则跳出，标记不相等
        //     for (int j = 0; j < faces2NodesTmp[end].size()-1; ++j)
        //     {
        //         if(tmp[j]!=faces2NodesTmp[end][j])
        //         {
        //             isEqual = false;
        //             break;
        //         }
        //     }
        //     if(isEqual)
        //     {
        //         label ownerId 
        //             = faces2NodesTmp[end][faces2NodesTmp[end].size()-1];
        //         /// 局部编号
        //         face2CellOwn.push_back(ownerId);
        //         /// 本地存储原始face2node顺序关系
        //         face2NodeOwn.push_back(face2NodeBndArr[i]);
        //         break;
        //     }
        //     end++;
        // }
        // /// 记录边界面单元对应cell编号，外部进程单元记-1
        // if(!isEqual)
        // {
        //     /// 不属于本地进程的face存储排序后的node顺序关系
        //     face2NodeNei.push_back(face2NodeBndArr[i]);
        // }
    }
    // par_std_out_("rank: %d, owner face: %d, neighbor face: %d\n", rank, face2NodeOwn.size(), face2NodeNei.size());

    Array<label> face2CellNew;
    ArrayArray<label> face2NodeNeiTmp;
    transformArray(face2NodeNei, face2NodeNeiTmp);
    Array<Array<label> > face2NodeBndOwn = innerTopo.getFace2NodeBnd();
    Array<Array<label> > face2CellBndOwn = innerTopo.getFace2CellBnd();
    Array<label> face2CellBndOwnTmp;
    // par_std_out_("rank: %d, boundary face: %d\n", rank, face2NodeBndOwn.size());
    for (int i = 0; i < face2CellBndOwn.size(); ++i)
    {
        face2CellBndOwnTmp.push_back(face2CellBndOwn[i][0]);
    }

    // 将不属于本进程的边界面发送给相应进程
    int bndFaces = face2NodeNeiTmp.num;

    int* bndFaces_mpi = new int[nprocs];
    MPI_Allgather(&bndFaces, 1, MPI_INT, bndFaces_mpi, 1, MPI_INT, MPI_COMM_WORLD);
    int bndFacesSum = 0;
    int ownerLoc = 0;
    int countIdx_r[nprocs], dispIdx_r[nprocs];
    for (int i = 0; i < nprocs; ++i)
    {
        // if(i==rank) ownerLoc = bndFacesSum+rank;
        dispIdx_r[i] = bndFacesSum+i;
        countIdx_r[i] = bndFaces_mpi[i]+1;
        bndFacesSum += bndFaces_mpi[i];
        // if(rank==0) par_std_out_("rank: %d, bndFaces: %d\n", i, bndFaces_mpi[i]);
    }
    label* startIdx_mpi = new label[bndFacesSum+nprocs];
#if 1
    MPI_Allgatherv(face2NodeNeiTmp.startIdx, bndFaces+1, MPI_LABEL, startIdx_mpi, countIdx_r, dispIdx_r, MPI_LABEL, MPI_COMM_WORLD);
    int bndNodesSum = 0;
    int countData_r[nprocs], dispData_r[nprocs];
    for (int i = 0; i < nprocs; ++i)
    {
        countData_r[i] = startIdx_mpi[dispIdx_r[i]+countIdx_r[i]-1];
        dispData_r[i] = bndNodesSum;
        bndNodesSum += countData_r[i];
        // if(rank==0) par_std_out_("count: %d, displacement: %d\n", countData_r[i], dispData_r[i]);
    }
    label* data_mpi = new label[bndNodesSum];
    MPI_Allgatherv(face2NodeNeiTmp.data, face2NodeNeiTmp.startIdx[bndFaces],
        MPI_LABEL, data_mpi, countData_r, dispData_r, MPI_LABEL, MPI_COMM_WORLD);
    label* neighborCellIdx_mpi = new label[bndFacesSum+nprocs];
#endif
    for (int i = 0; i < bndFacesSum+nprocs; ++i) { neighborCellIdx_mpi[i] = -1; }
    // if(rank==2)
    {
        for (int i = 0; i < nprocs; ++i)
        {
            if(rank==i) continue;
            // par_std_out_("rank: %d, start from: %d, read %d\n", i, dispIdx_r[i], countIdx_r[i]);
            label* startIdx = &startIdx_mpi[dispIdx_r[i]];
            for (int j = 0; j < countIdx_r[i]-1; ++j)
            {
                // par_std_out_("The %dth element: ", j);
                Array<label> face2NodeTmp;
                for (int k = startIdx[j]; k < startIdx[j+1]; ++k)
                {
                    // par_std_out_("%d, ", data_mpi[dispData_r[i]+k]);
                    face2NodeTmp.push_back(data_mpi[dispData_r[i]+k]);

                }
                Array<label> tmp;
                tmp.assign(face2NodeTmp.begin(),face2NodeTmp.end());
                sort(tmp.begin(),tmp.end());
                label idx = findSortedArray(face2NodeBndOwn, tmp, 0, face2NodeBndOwn.size()-1);
                // if(rank==1 && idx!=-1) par_std_out_("%d, %d\n", idx, face2CellBndOwnTmp[idx]);
                // // label idx = findArray(bndFaceArr, face2NodeTmp);
                // // label idx = -1;
                // if(idx!=-1) neighborCellIdx_mpi[dispIdx_r[i]+j] = face2CellBndOwnTmp[idx];
                // else neighborCellIdx_mpi[dispIdx_r[i]+j]=-1;

                if(idx!=-1)
                {
                    face2NodeOwn.push_back(face2NodeTmp);
                    face2CellOwn.push_back(face2CellBndOwnTmp[idx]);
                }
                // if(idx==-1) par_std_out_("not found\n");
                // else par_std_out_("%d\n", idx);
            }
            // par_std_out_("\n");
        }
    }

    Array<Array<label> > face2CellOwnArr;
    for (int i = 0; i < face2CellOwn.size(); ++i)
    {
        Array<label> tmp;
        tmp.push_back(face2CellOwn[i]);
        face2CellOwnArr.push_back(tmp);
    }

    // 根据节点个数确定网格面类型，并将网格面分开
    Array<label> faceNodeNum;
    for (int i = 0; i < face2NodeOwn.size(); ++i)
    {
        bool isExist = false;
        for (int j = 0; j < faceNodeNum.size(); ++j)
        {
            if(face2NodeOwn[i].size()==faceNodeNum[j])
            {
                isExist=true;
                break;
            }
        }
        if(!isExist)
        { 
            faceNodeNum.push_back(face2NodeOwn[i].size());
        }
    }
    // 根据网格面类型对网格面拓扑进行重排，使相同类型网格面连续存储
    Array<Array<Array<label> > > face2NodeWithType(faceNodeNum.size());
    Array<Array<Array<label> > > face2CellWithType(faceNodeNum.size());
    for (int i = 0; i < face2NodeOwn.size(); ++i)
    {
        bool isExist = false;
        for (int j = 0; j < faceNodeNum.size(); ++j)
        {
            if(face2NodeOwn[i].size()==faceNodeNum[j])
            {
                isExist=true;
                face2NodeWithType[j].push_back(face2NodeOwn[i]);
                face2CellWithType[j].push_back(face2CellOwnArr[i]);
            }
        }
        if(!isExist) Terminate("sort the faces according to the type", "unrecognized type");
    }
    // par_std_out_("There are %d types of faces, and we have %d faces\n", faceNodeNum.size(), face2NodeInn.size());
    face2NodeOwn.clear();
    face2CellOwnArr.clear();
    for (int i = 0; i < faceNodeNum.size(); ++i)
    {
        face2NodeOwn.insert(face2NodeOwn.end(), face2NodeWithType[i].begin(),
            face2NodeWithType[i].end());
        face2CellOwnArr.insert(face2CellOwnArr.end(),
            face2CellWithType[i].begin(), face2CellWithType[i].end());
    }
    this->getTopology().setFace2Node(face2NodeOwn);
    this->getTopology().setFace2Cell(face2CellOwnArr);
    // par_std_out_("faceNum: %d, %d\n", this->getTopology().getFacesNum(), face2NodeOwn.size());
    // if(rank==0)
    // {
    //     for (int i = 0; i < face2CellOwn.size(); ++i)
    //     {
    //         par_std_out_("The %dth face belongs to element %d: ", i, face2CellOwn[i]);
    //         for (int j = 0; j < face2NodeOwn[i].size(); ++j)
    //         {
    //             par_std_out_("%d, ", face2NodeOwn[i][j]);
    //         }
    //         par_std_out_("\n");
    //     }
    // }
    // if(rank==0)
    // {
    //     for (int i = 0; i < face2NodeBnd.size(); ++i)
    //     {
    //         par_std_out_("cell: %d, type: %d\n", i, this->getTopology().getCellType()[i]);
    //     }
    // }
    // printf("%d\n", this->.getTopology().getCellsNum());
    this->getTopology().setCell2Node(face2NodeOwn);
    // printf("%d\n", face2NodeOwn.size());
    // label* cellNum_mpi = new label[nprocs];
    // label cellNum_local = face2NodeOwn.size();
    // par_std_out_("rank: %d, cellNum: %d\n", rank, cellNum_local);
    // MPI_Allgather(&cellNum_local, 1, MPI_INT, cellNum_mpi, 1, MPI_LABEL, MPI_COMM_WORLD);
    // for (int i = 0; i < nprocs-1; ++i)
    // {
    //     if(rank==0) par_std_out_("%d\n", cellNum_mpi[i]);
    //     cellNum_mpi[i+1] += cellNum_mpi[i];
    // }
    // this->getTopology().setCellStartId(cellNum_mpi[rank]);
    DELETE_POINTER(neighborCellIdx_mpi);
    DELETE_POINTER(data_mpi);
    DELETE_POINTER(startIdx_mpi);
    DELETE_POINTER(bndFaces_mpi);
    DELETE_POINTER(isInner);
}

void Boundary::writeMesh(const char* filePtr)
{
    // par_std_out_("This is boundary!!!!!\n");
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    int iFile, nBases;
    int iBase=1, iZone=1;
    char basename[20];

    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
    if(cgp_open(filePtr, CG_MODE_MODIFY, &iFile))
        Terminate("readBaseInfo", cg_get_error())

    // 读取已有Section网格单元编号
    int nSecs;
    if(cg_nsections(iFile, iBase, iZone, &nSecs))
        Terminate("readNSections", cg_get_error());
    // par_std_out_("nSecs: %d\n", nSecs);
    int iSec;
    cgsize_t start, end;
    for (int iSec = 1; iSec <= nSecs; ++iSec)
    {
        char secName[20];
        ElementType_t type;
        int nBnd, parentFlag;
        if(cg_section_read(iFile, iBase, iZone, iSec, secName, 
            &type, &start, &end, &nBnd, &parentFlag))          
            Terminate("readSectionInfo", cg_get_error());
    }
    // write connectivity
    ArrayArray<label> conn = this->getTopology().getCell2Node();
    Array<label> cellType = this->getTopology().getCellType();
    label *cellStartId = new label[numProcs+1];
    MPI_Allgather(&conn.num, 1, MPI_LABEL, &cellStartId[1], 1, MPI_LABEL, MPI_COMM_WORLD);
    cellStartId[0] = end;
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
    // printf("%d, %d\n", end, conn.num);
    ElementType_t eleType = (ElementType_t)cellType[0];
    if(cgp_section_write(iFile, iBase, iZone, typeToWord(eleType), eleType, end+1, cellStartId[numProcs],
        0, &iSec))
        Terminate("writeSecInfo", cg_get_error());
    par_std_out_("rank %d write from %d to %d\n", rank, cellStartId[rank]+1, cellStartId[rank+1]);
    cgsize_t *data = (cgsize_t*)conn.data;
    if(cgp_elements_write_data(iFile, iBase, iZone, iSec, cellStartId[rank]+1,
        cellStartId[rank+1], data))
        Terminate("writeSecConn", cg_get_error());

    if(cgp_close(iFile))
        Terminate("closeCGNSFile",cg_get_error());

    DELETE_POINTER(cellStartId);
}

} // end namespace HSF