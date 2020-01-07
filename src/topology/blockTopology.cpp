/**
* @file: blockTopology.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2020-01-06 16:12:11
* @last Modified by:   lenovo
* @last Modified time: 2020-01-07 09:16:07
*/

#include <algorithm>
#include "blockTopology.hpp"
#include "section.hpp"

namespace HSF
{

BlockTopology::BlockTopology(Topology& topo)
{
	printf("hello world\n");
	const ArrayArray<label> cell2Cell = topo.getCell2Cell();
	const Array<label> cellType       = topo.getCellType();
	int blockNum = 0;
	this->cellType_.clear();
	vector<label>::iterator it;
	for (int i = 0; i < cellType.size(); ++i)
	{
		it = find(cellType_.begin(), cellType_.end(), cellType[i]);
		if(it==cellType_.end())
		{
			cellType_.push_back(cellType[i]);
			ElementType_t type = (ElementType_t)cellType[i];
			printf("add cell type: %s\n", Section::typeToWord(type));
		}
	}
	// Array<Array<label> > cell2CellBlk()
}

BlockTopology::~BlockTopology()
{

}

}
