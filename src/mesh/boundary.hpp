/**
* @file: boundary.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-26 09:25:10
* @last Modified by:   lhb8125
* @last Modified time: 2020-05-26 21:02:15
*/
#ifndef BOUNDARY_HPP
#define BOUNDARY_HPP

#include "utilities.hpp"
#include "mesh.hpp"

namespace HSF
{

/**
* @brief store the boundary elements
*/
class Boundary : public Mesh
{
private:
	label meshType_; ///< Identification for mesh type
	
	Array<BCSection> BCSecs_; ///< Boundary condition section information for CGNS file

	Array<label> BCType_;
	
	void readBoundaryCondition(const Word filePtr); ///< read boundary condition

	void readFamilyBC(const Word filePtr); ///< read family boundary condition
	
	void writeBoundaryCondition(const Word filePtr); ///< write mesh file with CGNS format

	void initBoundaryConditionType();
public:
	/**
	* @brief default constructor
	*/
	Boundary()
	{
		this->setMeshType(Boco);
	};
	/**
	* @brief deconstructor
	*/
	virtual ~Boundary(){};
	/**
	* @brief exchange boundary faces according to the load balance result
	* @param[in] innerTopo the internal topology between elements and nodes
	*/
	void exchangeBoundaryElements(Topology& innerTopo);
	/**
	* @brief write mesh and construct topology
	* @param[in] filePtr CGNS file name
	*/
	void writeMesh(const Word filePtr);
	/**
	* @brief generate the block topology
	*/
	void generateBlockTopo();
	/**
	* @brief read boundary condition
	*/
	void readBC(const Array<Word> filePtr)
	{
		for (int i = 0; i < filePtr.size(); ++i)
		{
			readBoundaryCondition(filePtr[i]);
			readFamilyBC(filePtr[i]);
		}
		initBoundaryConditionType();
	};

	/**
	* @brief write boundary condition
	*/
	void writeBC(const Word filePtr)
	{
		writeBoundaryCondition(filePtr);
		par_std_out_("writeBC\n");
	}
};

// struct BCSection
// {
// 	char* name;
// 	label type;
// 	label nBCElems;
// 	label* BCElems_;
// }

} // end namespace HSF;

#endif