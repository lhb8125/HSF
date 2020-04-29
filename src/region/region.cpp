/**
* @file: region.cpp
* @author: Liu Hongbin
* @brief:
* @date:   2019-10-14 09:17:17
* @last Modified by:   lenovo
* @last Modified time: 2020-01-09 17:17:46
*/
#include <algorithm>
#include "region.hpp"

using namespace std;

namespace HSF
{

bool compare(pair<label, pair<label, label> > a, pair<label, pair<label, label> > b)
{
    if(a.first == b.first)
    {
        if(a.second.first ==  b.second.first)
        {
            return a.second.second <  b.second.second;
        }

        return a.second.first < b.second.first;
    }

    return a.first < b.first;
}


void Region::initBeforeBalance(Array<char*> meshFile)
{
	// strncpy(meshFile_, meshFile, sizeof(meshFile_));
	// this->meshFile_[sizeof(meshFile)-1]='/0';
    meshFile_.assign(meshFile.begin(), meshFile.end());
	par_std_out_("start reading mesh ...\n");
	this->getMesh().readMesh(meshFile);
	par_std_out_("finish reading mesh ...\n");
	MPI_Barrier(MPI_COMM_WORLD);
}

void Region::initAfterBalance()
{
	par_std_out_("start constructing topology ...\n");
	this->getMesh().fetchNodes(this->meshFile_[0]);
    // this->getMesh().fetchNodes(this->meshFile_);
    // this->getMesh().fetchNodes(this->meshFile_[1]);
	this->getMesh().getTopology().constructTopology();
	par_std_out_("finish constructing topology ...\n");
	MPI_Barrier(MPI_COMM_WORLD);

    /// 创建通信对
	par_std_out_("start creating interfaces ...\n");
    label cellNum = this->getMesh().getTopology().getCellsNum();
    Array<Array<label> > faceCells
        = this->getMesh().getTopology().getFace2CellPatch();

    createInterFaces(faceCells, cellNum);

	par_std_out_("finish creating interfaces ...\n");

	par_std_out_("start reading boundary mesh ...\n");
	this->getBoundary().readMesh(this->meshFile_);
    this->getBoundary().readBC(this->meshFile_);
	par_std_out_("finish reading boundary mesh ...\n");
	Topology innerTopo = this->getMesh().getTopology();
	par_std_out_("start constructing boundary topology ...\n");
	this->getBoundary().exchangeBoundaryElements(innerTopo);
	par_std_out_("finish constructing boundary topology ...\n");

    par_std_out_("start initialize mesh information ...\n");
    this->meshInfo_.init(mesh_);
    par_std_out_("finish initialize mesh information ...\n");

    par_std_out_("start generate block topology ...\n");
    this->getMesh().generateBlockTopo();
    // this->getBoundary().generateBlockTopo();
    par_std_out_("finish generate block topology ...\n");

}

void Region::writeMesh(char* meshFile)
{
    par_std_out_("start write inner mesh ...\n");
	this->getMesh().writeMesh(meshFile);
    par_std_out_("finish write inner mesh ...\n");
    par_std_out_("start write boundary mesh ...\n");
	this->getBoundary().writeMesh(meshFile);
    par_std_out_("finish write boundary mesh ...\n");
    par_std_out_("start write boundary condition ...\n");
    this->getBoundary().writeBC(meshFile);
    par_std_out_("finish write boundary condition ...\n");
}

//- TODO: for face only by now
void Region::createInterFaces(Array<Array<label> > &faceCells, label cellNum)
{
  int nProcs, myProcNo;

  MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myProcNo);

  label *partitionInfo = new label[nProcs + 1];

  partitionInfo[0] = 0;
  MPI_Allgather(
      &cellNum, 1, MPI_LABEL, &partitionInfo[1], 1, MPI_LABEL, MPI_COMM_WORLD);

  for (int i = 0; i < nProcs; ++i)
  {
    partitionInfo[i + 1] += partitionInfo[i];
  }

  label nCellsAll = partitionInfo[nProcs];

  if (!patchTabPtr_)
  {
    //- create
    patchTabPtr_ = new Table<Word, Table<Word, Patch *> *>;
  }

  Table<Word, Table<Word, Patch *> *> &allPatchTab = *patchTabPtr_;

  Word patchType = "cell";

  allPatchTab[patchType] = new Table<Word, Patch *>;

  Table<label, label> facesInProcessor;
  Array<pair<label, pair<label, label> > > vec;

  for (label i = 0; i < faceCells.size(); ++i)
  {
    label IDIn, IDOut;
    IDIn = faceCells[i][0];
    IDOut = faceCells[i][1];

    bool FIND = false;
    //- get the assumed neighbor processor ID,
    //- by assuming that all cells are partitioned uniformly
    label procAssume = IDOut / (nCellsAll / nProcs);

    if (IDOut < nCellsAll)
    {
      if (procAssume > nProcs - 1) procAssume = nProcs - 1;

      do
      {
        if (IDOut >= partitionInfo[procAssume + 1])
        {
          procAssume++;
        }
        else if (IDOut < partitionInfo[procAssume])
        {
          procAssume--;
        }
        else
        {
          //- do nothing
          FIND = true;
        }
      } while (!FIND);
    }
    else
    {
      par_std_out_(
          "Error: cell ID exceeds the total cell number: cell ID = %d, total "
          "number = %d!\n",
          IDOut,
          nCellsAll);
    }

    //- smaller IDs are placed in the left
    //- thus all processors will follow the same discipline
    //- and produce the same patch faces order
    if (IDIn > IDOut)
    {
      int temp = IDIn;
      IDIn = IDOut;
      IDOut = temp;
    }

    vec.push_back(make_pair(procAssume, make_pair(IDIn, IDOut)));
  }

  //- sort faces in order of neighbor processors
  std::sort(vec.begin(), vec.end(), compare);

  Table<label, Array<pair<label, label> > > mapCells;

  //- split the faces in order of neighbor processors
  for (label i = 0; i < vec.size(); ++i)
  {
    label NbrProcID = vec[i].first;
    label cellID1 = vec[i].second.first;
    label cellID2 = vec[i].second.second;
    label localCellID;
    label nbrCellID;

    //- find the cellIDs belonging to current processor
    //- and storage them
    if (cellID1 >= partitionInfo[myProcNo] &&
        cellID1 < partitionInfo[myProcNo + 1])
    {
      localCellID = cellID1;
      nbrCellID = cellID2;
    }
    else if (cellID2 >= partitionInfo[myProcNo] &&
             cellID2 < partitionInfo[myProcNo + 1])
    {
      localCellID = cellID2;
      nbrCellID = cellID1;
    }
    //- myProNo + 1 == nProcs
    //- 最右端闭合
    else if (cellID2 == partitionInfo[myProcNo + 1])
    {
      localCellID = cellID2;
      nbrCellID = cellID1;
    }
    else
    {
      par_std_out_(
          "Error: cell is not in the target Processor, please check! At proc = "
          "%d, elements from %d to %d, cell1 = %d, cell2 = %d\n",
          myProcNo,
          partitionInfo[myProcNo],
          partitionInfo[myProcNo + 1],
          cellID1,
          cellID2);
      ERROR_EXIT;
    }

    mapCells[NbrProcID].push_back(make_pair(localCellID, nbrCellID));
  }

  //- compress send and receive
  Table<label, Array<pair<label, label> > >::iterator it;

  //- number of neighbor processors
  label nbrProcSize = mapCells.size();
  //- local/neighbor cell number
  //- 计数累进制
  label *locCellNumProcs = new label[nbrProcSize + 1];
  label *nbrCellNumProcs = new label[nbrProcSize + 1];
  locCellNumProcs[0] = 0;
  nbrCellNumProcs[0] = 0;
  label i = 0;

  //- 按进程分段存储，全局编号，每段内编号不重复，且升序排列
  Array<label> sendMap;
  //- 按进程分段存储，全局编号，编号不重复，升序排列
  Array<label> recvMap;

  for (it = mapCells.begin(), i = 1; it != mapCells.end(); ++it, ++i)
  {
    label faceSize = (it->second).size();
    Array<pair<label, label> > face2CellGlobal = it->second;
    Table<label, Array<label> > locCellIDKey;
    Table<label, Array<label> > nbrCellIDKey;
    for (label j = 0; j < faceSize; ++j)
    {
      locCellIDKey[face2CellGlobal[j].first].push_back(
          face2CellGlobal[j].second);

      nbrCellIDKey[face2CellGlobal[j].second].push_back(
          face2CellGlobal[j].first);
    }
    //- recv size
    nbrCellNumProcs[i] = nbrCellNumProcs[i - 1] + nbrCellIDKey.size();

    //- send size
    locCellNumProcs[i] = locCellNumProcs[i - 1] + locCellIDKey.size();

    Table<label, Array<label> >::iterator itin;
    for (itin = locCellIDKey.begin(); itin != locCellIDKey.end(); ++itin)
    {
      sendMap.push_back(itin->first);
    }

    for (itin = nbrCellIDKey.begin(); itin != nbrCellIDKey.end(); ++itin)
    {
      recvMap.push_back(itin->first);
    }
  }


  //- send index, reference to send buffer
  label sendSizeAll = locCellNumProcs[nbrProcSize];
  label *sendLocID = new label[sendSizeAll];
  if (sendMap.size() != sendSizeAll) ERROR_EXIT;
  for (label j = 0; j < sendSizeAll; ++j)
  {
    sendLocID[j] = sendMap[j] - partitionInfo[myProcNo];
  }

  bool ifUsingCompressedComm = true;

  //- recv index, reference to recv buffer
  label recvSizeAll = nbrCellNumProcs[nbrProcSize];
  label *nbrCellID = new label[recvSizeAll];
  Table<label, label> recvMapLoc;
  if (recvMap.size() != recvSizeAll) ERROR_EXIT;
  for (label j = 0; j < recvSizeAll; ++j)
  {
    recvMapLoc[recvMap[j]] = j + cellNum;
  }

  Array<Array<label> > face2Cell_1;
  Array<Array<label> > cell2Node_1;
  Array<Array<label> > face2Node_1;

  const ArrayArray<label> &face2Cell_0 = mesh_.getTopology().getFace2Cell();
  const ArrayArray<label> &cell2Node_0 = mesh_.getTopology().getCell2Node();
  const ArrayArray<label> &face2Node_0 = mesh_.getTopology().getFace2Node();
  const Array<Array<label> > &face2NodePatch_1 = mesh_.getTopology().getFace2NodePatch();
  const Array<label> &cellType = mesh_.getTopology().getCellType();
  transformArray(face2Cell_0, face2Cell_1);
  transformArray(cell2Node_0, cell2Node_1);
  transformArray(face2Node_0, face2Node_1);


  label innerFaceSize = face2Cell_0.size();
  label umcompressedCount = cellNum;

  for (it = mapCells.begin(); it != mapCells.end(); it++)
  {
    label faceSize = (it->second).size();

    Array<pair<label, label> > face2CellIDsGlobal = it->second;
    label cell1, cell2;
    for (label j = 0; j < faceSize; ++j)
    {
      cell1 = face2CellIDsGlobal[j].first - partitionInfo[myProcNo];

      if (ifUsingCompressedComm)
      {
        cell2 = (recvMapLoc.find(face2CellIDsGlobal[j].second))->second;
      }
      else
      {
        cell2 = umcompressedCount;
        umcompressedCount++;
      }

      Array<label> arrtemp;
      arrtemp.push_back(cell1);
      arrtemp.push_back(cell2);
      face2Cell_1.push_back(arrtemp);

      // bool flag = false;
      // for(int iface=0;iface<faceCells.size();iface++)
      // {
      //   if(faceCells[iface][0]==cell1) flag = true;
      // }
      // if(!flag)
      //   Terminate("createInterFaces","the cell index is not correct");

      // par_std_out_("%d,%d,%d\n",cell1,cell2,cell2Node_0.size());
      if(cell1>cell2Node_1.size())
        Terminate("createInterFaces","the cell in the interfaces exceed the cell num");
      int found = 0;
      Array<label> c2n = cell2Node_1[cell1];
      label faceNumTmp = Section::facesNumForEle(cellType[cell1]);
      for (int jj = 0; jj < faceNumTmp; ++jj)
      {
        Array<label> face2NodeTmp = Section::faceNodesForEle(
          &cell2Node_0.data[cell2Node_0.startIdx[cell1]],
          cellType[cell1], jj);
        Array<label> tmp;
        tmp.assign(face2NodeTmp.begin(), face2NodeTmp.end());
        sort(face2NodeTmp.begin(), face2NodeTmp.end());
        for(int iface=0;iface<face2NodePatch_1.size();iface++)
        {
          if(compareArray(face2NodeTmp, face2NodePatch_1[iface])
            && face2CellIDsGlobal[j].second==faceCells[iface][1])
          {
            found++;
            // par_std_out_("%d,%d\n", face2CellIDsGlobal[j].second,faceCells[iface][1]);
            face2Node_1.push_back(tmp);
          }
        }
      }
      if(found!=1)
      {
        for (int inode = 0; inode < cell2Node_1[cell1].size(); ++inode)
        {
          par_std_out_("%d,", cell2Node_1[cell1][inode]);
        }
        par_std_out_("\n");
        for (int iface = 0; iface < face2NodePatch_1.size(); ++iface)
        {
          par_std_out_("The %dth face:\n", iface);
          for (int inode = 0; inode < face2NodePatch_1[iface].size(); ++inode)
          {
            par_std_out_("%d,", face2NodePatch_1[iface][inode]);
          }
          par_std_out_("\n");
        }
        Terminate("createInterFaces","can not find the corresponding face2node");
      }
    }
  }
  mesh_.getTopology().setFace2Cell(face2Cell_1);
  mesh_.getTopology().setFace2Node(face2Node_1);
  const ArrayArray<label> &face2Cell_2 = mesh_.getTopology().getFace2Cell();

  Table<Word, Patch *> &patches = *allPatchTab[patchType];

  label jj = 0;
  label faceSizeStart = innerFaceSize;
  for (it = mapCells.begin(); it != mapCells.end(); it++)
  {
    label nbrID = it->first;
    label faceSize = (it->second).size();
    label startIndex = face2Cell_2.startIdx[faceSizeStart];

    label* face2cellpart = new label[faceSize];
    for(int kk = 0; kk < faceSize; ++kk)
    {
      face2cellpart[kk] = face2Cell_2.data[startIndex + kk * 2];
    }

    //- construct the patch
    patches[to_string(nbrID)] =
        new Patch(faceSize, &(face2cellpart[0]), nbrID);

    if (ifUsingCompressedComm)
    {
      label sendSize = locCellNumProcs[jj + 1] - locCellNumProcs[jj];
      label recvSize = nbrCellNumProcs[jj + 1] - nbrCellNumProcs[jj];
      patches[to_string(nbrID)]->setSendRecvCompressed(
          sendSize, recvSize, &sendLocID[locCellNumProcs[jj]]);
      ++jj;
    }

    faceSizeStart += faceSize;
    DELETE_POINTER(face2cellpart);
  }

  DELETE_POINTER(partitionInfo);
  DELETE_POINTER(locCellNumProcs);
  DELETE_POINTER(nbrCellNumProcs);
  DELETE_POINTER(sendLocID);
  DELETE_POINTER(nbrCellID);
}


Region::~Region()
{
    //- free interfaces created

    //- delete Table<Word, Table<Word, Patch*>*>*
    if(patchTabPtr_)
    {
        Table<Word, Table<Word, Patch*>*>& allPatchTab = *patchTabPtr_;
        Table<Word, Table<Word, Patch*>*>::iterator it1;

        //- delete Table<Word, Patch*>*
        for(it1= allPatchTab.begin(); it1!=allPatchTab.end(); it1++)
        {
            Table<Word, Patch*>* patchesPtr = it1->second;
            Table<Word, Patch*>& patches = *patchesPtr;
            Table<Word, Patch*>::iterator it2;
            //- delete Patch*
            for(it2=patches.begin(); it2!=patches.end(); it2++)
            {
                Patch* patchIPtr = it2->second;
                DELETE_OBJECT_POINTER(patchIPtr);
            }
            DELETE_OBJECT_POINTER(patchesPtr)
        }
        DELETE_OBJECT_POINTER(patchTabPtr_);
    }
}

} // end namespace HSF