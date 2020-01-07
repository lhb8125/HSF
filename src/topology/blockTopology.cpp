/**
* @file: blockTopology.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2020-01-06 16:12:11
* @last Modified by:   lenovo
* @last Modified time: 2020-01-07 16:49:27
*/

#include <algorithm>
#include "blockTopology.hpp"
#include "section.hpp"

namespace HSF
{

BlockTopology::BlockTopology(Topology& topo)
{
	printf("hello world\n");
	Array<label> cellMap = this->reorderCellTopo(topo);
}

BlockTopology::~BlockTopology()
{

}

}

Array<label> BlockTopology::reorderCellTopo(Topology& topo)
{
	const ArrayArray<label> cell2Cell = topo.getCell2Cell();
	const Array<label> cellType       = topo.getCellType();
	int blockNum = 0;
	this->cellType_.clear();
	vector<label>::iterator it;
	Array<label> cellNumInBlk;
	cellNumInBlk.push_back(0);

	// 确定网格类型数目
	for (int i = 0; i < cellType.size(); ++i)
	{
		it = find(cellType_.begin(), cellType_.end(), cellType[i]);
		if(it!=cellType_.end())
		{
			cellNumInBlk[it-cellType_.begin()+1]++;
		}
		if(it==cellType_.end())
		{
			cellType_.push_back(cellType[i]);
			cellNumInBlk.push_back(1);
			ElementType_t type = (ElementType_t)cellType[i];
			printf("add cell type: %s\n", Section::typeToWord(type));
			blockNum++;
		}
	}

	for (int i = 0; i < cellNumInBlk.size(); ++i)
	{
		// printf("%d\n", cellNumInBlk[i]);
		cellNumInBlk[i+1] += cellNumInBlk[i];
	}

	Array<Array<Array<label> > > cell2CellBlk(blockNum);
	Array<label> cellMap;

	// 将不同类型网格分装到不同容器中
	for (int i = 0; i < cellType.size(); ++i)
	{
		it = find(cellType_.begin(), cellType_.end(), cellType[i]);
		if(it!=cellType_.end())
		{
			// printf("%d,%d\n", i,it-cellType_.begin());
			Array<label> tmp;
			for (int j = cell2Cell.startIdx[i]; j < cell2Cell.startIdx[i+1]; ++j)
			{
				tmp.push_back(cell2Cell.data[j]);
			}
			label blockIdx = it-cellType_.begin();
			cellMap.push_back(cell2CellBlk[blockIdx].size()+cellNumInBlk[blockIdx]);
			cell2CellBlk[blockIdx].push_back(tmp);
		}
	}

	// 将不同类型网格合并到一起
	Array<Array<label> > cell2CellBlkNew;
	for (int i = 0; i < cell2CellBlk.size(); ++i)
	{
		cell2CellBlkNew.insert(cell2CellBlkNew.begin(),
			cell2CellBlk[i].begin(), cell2CellBlk[i].end());
	}

	// 根据重排前后的映射调整cell2cell的值
	for (int i = 0; i < cell2CellBlkNew.size(); ++i)
	{
		printf("The %dth element: ", i);
		for (int j = 0; j < cell2CellBlkNew[i].size(); ++j)
		{
			cell2CellBlkNew[i][j] = cellMap[cell2CellBlkNew[i][j]];
			printf("%d, ", cell2CellBlkNew[i][j]);
		}
		printf("\n");
	}

	// cell2cell
	transformArray(this->cell2Cell_, cell2CellBlkNew);

	this->cellBlockStartIdx_.assign(cellNumInBlk.begin(), cellNumInBlk.end());

	// cell2node
	const ArrayArray<label> cell2Node = topo.getCell2Node();

	this->cell2Node_.num = cell2Node.num;
	this->cell2Node_.startIdx = new label[cell2Node.num+1];
	this->cell2Node_.startIdx[0] = 0;
	for (int i = 0; i < cell2Node.size(); ++i)
	{
		this->cell2Node_.startIdx[cellMap[i]+1]
			= cell2Node.startIdx[i+1]-cell2Node.startIdx[i];
	}
	for (int i = 0; i < cell2Node.size(); ++i)
	{
		this->cell2Node_.startIdx[i+1] += this->cell2Node_.startIdx[i];
	}

	this->cell2Node_.data
		= new label[this->cell2Node_.startIdx[cell2Node.num]];
	for (int i = 0; i < cell2Node.size(); ++i)
	{
		int startIdx = this->cell2Node_.startIdx[cellMap[i]];
		int bonus = 0;
		for (int j = cell2Node.startIdx[i]; j < cell2Node.startIdx[i+1]; ++j)
		{
			this->cell2Node_.data[startIdx+bonus] = cell2Node.data[j];
			bonus++;
		}
	}

	return cellMap;
}