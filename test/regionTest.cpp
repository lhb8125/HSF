/*
The MIT License

Copyright (c) 2019 Hanfeng GU <hanfenggu@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
* @File: regionTest.cpp
* @Author: Hanfeng GU
* @Email:
* @Date:   2019-11-12 17:04:56
* @Last Modified by:   Hanfeng
* @Last Modified time: 2019-11-28 14:16:14
*/

/*
* @brief:
*/


#include "region.hpp"
#include "mpiWrapper.hpp"
#include "fieldFiled.hpp"

using namespace HSF;

int main(int argc, char const *argv[])
{
	MPI_Init(NULL, NULL);
	int nProcs, myProcNo;

	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myProcNo);

    int startNo;

    label face2Cell1[16], face2Cell2[16];
    int faceSize = 0;
    int cellSize = 0;

    if(myProcNo == 0)
    {
    	startNo = 0;
    	cellSize = 8;

    	//- In total: 11 faces, related to 2 neighbors
    	//- neighbor Processor: 1 (5 faces)
    	//- neighbor Processor: 2 (6 faces)
    	faceSize = 11;
    	face2Cell1[0]  = 1;
    	face2Cell1[1]  = 2;
    	face2Cell1[2]  = 3;
    	face2Cell1[3]  = 0;
    	face2Cell1[4]  = 5;

    	face2Cell1[5]  = 7;
    	face2Cell1[6]  = 3;
    	face2Cell1[7]  = 6;
    	face2Cell1[8]  = 5;
    	face2Cell1[9]  = 4;
    	face2Cell1[10] = 0;

    	face2Cell2[0]  = 9;
    	face2Cell2[1]  = 19;
    	face2Cell2[2]  = 21;
    	face2Cell2[3]  = 8;
    	face2Cell2[4]  = 17;

    	face2Cell2[5]  = 17;
    	face2Cell2[6]  = 10;
    	face2Cell2[7]  = 13;
    	face2Cell2[8]  = 23;
    	face2Cell2[9]  = 18;
    	face2Cell2[10] = 10;
    }
    else if(myProcNo == 1)
    {
    	startNo = 8;
    	cellSize = 9;

    	//- In total: 10 faces, related to 2 neighbors
    	//- neighbor Processor: 0 (5 faces)
    	//- neighbor Processor: 2 (5 faces)
    	faceSize = 10;
    	face2Cell1[0]  = 10;
    	face2Cell1[1]  = 9;
    	face2Cell1[2]  = 13;
    	face2Cell1[3]  = 15;
    	face2Cell1[4]  = 8;

    	face2Cell1[5]  = 8;
    	face2Cell1[6]  = 11;
    	face2Cell1[7]  = 9;
    	face2Cell1[8]  = 10;
    	face2Cell1[9]  = 10;

    	face2Cell2[0]  = 3;
    	face2Cell2[1]  = 18;
    	face2Cell2[2]  = 6;
    	face2Cell2[3]  = 19;
    	face2Cell2[4]  = 0;

    	face2Cell2[5]  = 18;
    	face2Cell2[6]  = 19;
    	face2Cell2[7]  = 1;
    	face2Cell2[8]  = 0;
    	face2Cell2[9]  = 19;
    }
    else if(myProcNo == 2)
    {
    	startNo = 17;
    	cellSize = 7;

    	//- In total: 16 faces, related to 3 neighbors
    	//- neighbor Processor: 0 (6 faces)
    	//- neighbor Processor: 1 (5 faces)
    	//- neighbor Processor: 3 (5 faces)
    	faceSize = 16;
    	face2Cell1[0]  = 23;
    	face2Cell1[1]  = 19;
    	face2Cell1[2]  = 17;
    	face2Cell1[3]  = 19;
    	face2Cell1[4]  = 18;

    	face2Cell1[5]  = 22;
    	face2Cell1[6]  = 17;
    	face2Cell1[7]  = 20;
    	face2Cell1[8]  = 21;
    	face2Cell1[9]  = 21;

    	face2Cell1[10] = 19;
    	face2Cell1[11] = 19;
    	face2Cell1[12] = 18;
    	face2Cell1[13] = 18;
    	face2Cell1[14] = 23;
    	face2Cell1[15] = 22;

    	face2Cell2[0]  = 5;
    	face2Cell2[1]  = 11;
    	face2Cell2[2]  = 7;
    	face2Cell2[3]  = 10;
    	face2Cell2[4]  = 4;

    	face2Cell2[5]  = 26;
    	face2Cell2[6]  = 5;
    	face2Cell2[7]  = 29;
    	face2Cell2[8]  = 3;
    	face2Cell2[9]  = 27;

    	face2Cell2[10] = 2;
    	face2Cell2[11] = 15;
    	face2Cell2[12] = 9;
    	face2Cell2[13] = 8;
    	face2Cell2[14] = 28;
    	face2Cell2[15] = 29;
    }
    else if(myProcNo == 3)
    {
    	startNo = 24;
    	cellSize = 6;

    	//- In total: 5 faces, related to 1 neighbors
    	//- neighbor Processor: 2 (5 faces)
    	faceSize = 5;
    	face2Cell1[0]  = 27;
    	face2Cell1[1]  = 29;
    	face2Cell1[2]  = 28;
    	face2Cell1[3]  = 29;
    	face2Cell1[4]  = 26;

    	face2Cell2[0]  = 21;
    	face2Cell2[1]  = 22;
    	face2Cell2[2]  = 23;
    	face2Cell2[3]  = 20;
    	face2Cell2[4]  = 22;
    }

    Array<Array<label> > face2Cell;

    for(int i=0; i<faceSize; ++i)
    {
    	Array<label> v;
    	v.push_back(face2Cell1[i]);
    	v.push_back(face2Cell2[i]);
    	face2Cell.push_back(v);
    }


    Region reg;

    reg.createInterFaces(face2Cell, cellSize);

    label cellIDs[cellSize];
    for(int i=0; i<cellSize; ++i)
    {
    	cellIDs[i] = i;
    }

    Field<label> ff("face", 1, cellSize, cellIDs);

    reg.addField<label>("ID", &ff);

    labelField& ff2 = reg.getField<label>("face", "ID");

    label* localData = ff2.getLocalData();

    // if(myProcNo == 0)
    // for(int i=0; i<cellSize; ++i)
    // {
    // 	cout << "at i = " << i << ", val = " << localData[i] << endl;
    // }

    // ff2.initSend();
    // ff2.checkSendStatus();

    reg.initField<label>("face", "ID");


    Table<Word, label*>* nbrDataPtr = ff2.getNbrData();
    Table<Word, label*>& nbrData = *nbrDataPtr;
    Table<Word, label*>::iterator it = nbrData.begin();
    Table<Word, Patch*>& patches = *ff2.getPatchTab();

    if(myProcNo == 2)
    for(it=nbrData.begin(); it!=nbrData.end(); ++it)
    {
    	cout << "name is " << it->first << ": " << endl;
    	label* nbrData1 = it->second;
    	label fsize = (*patches[it->first]).getSize();
    	for(int i=0; i<fsize; ++i)
    	{
    		cout << "at i = " << i << ", val = " << nbrData1[i] << endl;
    	}
    }

    reg.deleteField<label>("face", "ID");

    labelField& ff3 = reg.getField<label>("face", "ID");

    FieldField<label> ff4;

    ff4.addField("ID", &ff);

    ff4.getField("ID");

    ff4.deleteField("ID");

	return 0;
}

