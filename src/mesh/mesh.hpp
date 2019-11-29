/**
* @file: mesh.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-24 09:25:44
* @last Modified by:   lenovo
* @last Modified time: 2019-11-28 10:27:09
*/
#ifndef MESH_HPP
#define MESH_HPP

#include "utilities.hpp"
#include "section.hpp"
#include "topology.hpp"
#include "nodes.hpp"

namespace HSF
{

/**
* @brief Mesh
*/
class Mesh
{
private:
	Topology topo_; ///< topology
	
	Nodes nodes_; ///< Coordinates of Nodes
	
	Nodes ownNodes_; ///< Coordinates of nodes owned by this process
	
	Table<label, label> coordMap_; ///< map between the absolute index and the local index
	
	Array<Section> secs_; ///< section information for CGNS file

	label meshType_; ///< the type of mesh: boundary or inner mesh
	
	label nodeNum_; /// count of nodes
	
	label eleNum_; /// count of elements
	/**
	* @brief read mesh file with CGNS format
	*/
	void readCGNSFile(const char* filePtr);
	/**
	* @brief read mesh file with CGNS format, parallel version
	*/
	void readCGNSFilePar(const char* filePtr);
	/**
	* @brief write mesh file with CGNS format, parallel version
	*/
	void writeCGNSFilePar(const char* filePtr);
	/**
	* @brief initialize mesh file with CGNS format, parallel version
	*/
	void initCGNSFilePar(const char* filePtr);
public:
	/**
	* @brief default constructor
	*/
	Mesh()
	{
		this->meshType_ = Inner;
	};
	/**
	* @brief deconstructor
	*/
	virtual ~Mesh(){};
	/**
	* @brief read mesh and construct topology
	*/
	void readMesh(const char* filePtr)
	{
		readCGNSFilePar(filePtr);
		topo_.constructTopology(this->secs_);
	};
	/**
	* @brief initialize mesh and construct topology
	*/
	void initMesh(const char* filePtr)
	{
		// writeCGNSFile(filePtr);
		initCGNSFilePar(filePtr);
		// topo_ = new Topology(this->secs_);
	};
	/**
	* @brief write mesh and construct topology
	*/
	void writeMesh(const char* filePtr)
	{
		// writeCGNSFile(filePtr);
		writeCGNSFilePar(filePtr);
		// topo_ = new Topology(this->secs_);
	};
	/**
	* @brief get class Topology
	*/
	Topology& getTopology() {return this->topo_;};

	/**
	* @brief get coordinates of Nodes without load balance
	*/
	Nodes& getNodes() {return this->nodes_;};

	/**
	* @brief get coordinates of nodes owned by this process
	*/
	Nodes& getOwnNodes() {return this->ownNodes_;};

	/**
	* @brief get the collections of section in CGNS file
	*/
	Array<Section>& getSections() {return this->secs_; };

	/**
	* @brief set the load balance result to topology
	* @param[in] cell2Node the topology between elements and nodes
	* @param[in] cellType the elements type
	*/
	void setLoadBalancerResult(ArrayArray<label>& cell2Node,
		Array<label> cellType)
	{
		this->topo_.setCell2Node(cell2Node);
		this->topo_.setCellType(cellType);
	};

	void setMeshType(label meshType) {this->meshType_ = meshType;};

	/**
	* @brief translate the element type to string
	*/
	char* typeToString(ElementType_t eleType);

	/**
	* @brief fetch the coordinates of nodes owned by this process
	*/
	void fetchNodes(char* filePtr);

	/**
	* @brief get the map between the absolute index and the local index
	*/
	Table<label, label>& getCoordMap() {return this->coordMap_;};
	
};

} // end namespace HSF


#endif