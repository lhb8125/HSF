/**
* @file: section.hpp
* @author: Liu Hongbin
* @Date:   2019-10-14 10:05:10
* @Last Modified by:   lenovo
* @Last Modified time: 2019-10-14 14:14:36
*/
#ifndef SECTION_H
#define SECTION_H
#include "cgnslib.h"
#include "utilities.hpp"

namespace HSF
{

/**
* @brief Elements with same type are stored in one section
*/
class Section
{
public:
	char*  name; ///< section name
	
	ElementType_t  type; ///< element type of the section
	
	Label  iStart; ///< global start index of the section
	
	Label  iEnd; ///< global end index of the section
	
	Label  num; ///< count of elements of the section

	Label  nBnd;

	Label* conn; ///< connectivity of the section
	/**
	* @brief counts of nodes for each element type
	* @param[in] eleType element type
	* @return counts of nodes
	*/
	static Label nodesNumForEle(const Label eleType);
	/**
	* @brief counts of faces for each element type
	* @param[in] eleType element type
	* @return counts of faces
	*/
	static Label facesNumForEle(const Label eleType);
	/**
	* @brief counts of edges for each element type
	* @param[in] eleType element type
	* @return counts of edges
	*/
	static Label edgesNumForEle(const Label eleType);
	/**
	* @brief the connectivity of faces and nodes for each element type
	* @param[in] conn connectivity of elements and nodes
	* @param[in] eleType element type
	* @param[in] idx the index of faces of the elements
	* @return the connectivity of faces and nodes
	*/
	static Array<Label> faceNodesForEle(Label* conn, const Label eleType, const Label idx);
	/**
	* @brief the connectivity of edges and nodes for each element type
	* @param[in] conn connectivity of elements and nodes
	* @param[in] eleType element type
	* @param[in] idx the index of edges of the elements
	* @return the connectivity of edges and nodes
	*/
	static Array<Label> edgeNodesForEle(Label* conn, const Label eleType, const Label idx);
	/**
	* @brief whether the section belongs to the entity through the elements type 
	* @param[in] secType section type
	* @param[in] meshType_ mesh type
	* @return whether the section belongs to the entity
	*/
	static bool compareEleType(const Label secType, const Label meshType_);
};

struct BCSection
{
	char name[40]; ///< section name

	BCType_t type; ///< boundary condition type

	cgsize_t nBCElems; ///< the count of boundary elements

	GridLocation_t location; ///< the location of boundary condition

	PointSetType_t ptsetType[1]; ///< the type of set of boundary elements

	cgsize_t* BCElems; ///< list or range of boundary elements
};

} // end namespace HSF

#endif