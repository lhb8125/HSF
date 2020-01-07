/**
* @file: blockTopology.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2020-01-06 15:57:25
* @last Modified by:   lenovo
* @last Modified time: 2020-01-06 16:36:50
*/

#ifndef BLOCKTOPOLOGY_HPP
#define BLOCKTOPOLOGY_HPP
#include <string.h>
#include "utilities.hpp"
#include "section.hpp"
#include "cgnslib.h"
#include "topology.hpp"

namespace HSF
{

/**
* @brief Topology information of mesh
*/
class BlockTopology
{
private:
	ArrayArray<label> face2Node_; ///< Connectivity between faces and nodes (finish)
	
	Array<Array<label> > face2NodeBnd_; ///< Connectivity between faces and nodes at the boundary (finish)
	
	ArrayArray<label> face2Cell_; ///< Connectivity between faces and cells (finish)
	
	Array<Array<label> > face2CellPatch_; ///< Connectivity between faces and cells at the process boundary (finish)
	
	Array<Array<label> > face2CellBnd_; ///< Connectivity between faces and cells at the boundary (finish)
	
	ArrayArray<label> cell2Cell_; ///< Connectivity between cells and cells (finish)
	
	ArrayArray<label> cell2Node_; ///< Connectivity between cells and nodes (finish)
	
	ArrayArray<label> cell2Face_; ///< Connectivity between cells and faces (finish)
	
	ArrayArray<label> cell2Edge_; ///< Connectivity between cells and edges (finish)
	
	Array<label> cellType_; ///< type of cells
	
	Array<label> faceType_; ///< type of faces

public:
	/**
	* @brief default constructor
	*/
	BlockTopology(Topology& topo);

	/**
	* @brief default deconstructor
	*/
	~BlockTopology();

	/**
	* @brief get the count of cell types over the region
	*/
	label getCellBlockNum(){return this->cell2Cell_.size();};

	/**
	* @brief get the count of face types over the region
	*/
	label getFaceBlockNum(){return this->face2Node_.size();};
};

}

#endif