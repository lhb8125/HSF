/**
* @file: mesh.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-24 09:25:44
* @last Modified by:   lenovo
* @last Modified time: 2020-01-09 11:20:48
*/
#ifndef MESH_HPP
#define MESH_HPP

#include "utilities.hpp"
#include "section.hpp"
#include "topology.hpp"
#include "blockTopology.hpp"
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

	BlockTopology blockTopo_;
	
	Nodes nodes_; ///< Coordinates of Nodes

	Array<label> nodeStartIdx_; ///< start index of nodes reading from CGNS file

	Array<label> nodeEndIdx_; ///< end index of nodes reading from CGNS file

	Array<label> nodeNumLocal_; ///< end index of nodes reading from CGNS file

	Array<label> nodeNumGlobal_; ///< the count of overall nodes of CGNS file

	Array<label> eleNumGlobal_; ///< the count of overall elements of CGNS file

	/// map from real nodes in zone to nodes in CGNS
	// Array<Array<label64> > node_local_2_global_; 

	// map from nodes in CGNS to real nodes in zone
	// Array<Array<label64> > node_global_2_local_;

	Array<char*> zoneName_; ///< zone name in CGNS file

	/// map between the nodes at the interfaces of zones
	std::map<label64, label64> zc_node_map_; 

	
	Nodes *ownNodes_; ///< Coordinates of nodes owned by this process
	
	Table<label, label> coordMap_; ///< map between the absolute index and the local index
	
	Array<Section> secs_; ///< section information for CGNS file

	label meshType_; ///< the type of mesh: boundary or inner mesh
	
	// label nodeNum_; ///< count of nodes
	
	label eleNum_; ///< count of elements
	/**
	* @brief read mesh file with CGNS format
	*/
	void readCGNSFile(const char* filePtr);
	/**
	* @brief read only one zone of CGNS file
	*/
	void readOneZone(const int iFile, const int iBase, const int iZone);
	/**
	* @brief read zone connectivity
	* @param[in] filePtr CGNS file
	* @param[out] zc_pnts the nodes of own zone at the interfaces
	* @param[out] zc_donor_pnts the nodes of neighbor zone at the interfaces
	* @param[out] zc_name the name of neighbor zone at the interfaces
	* @param[out] nodes the points of the nodes at the interfaces
	*/
	void readZoneConnectivity(const char* filePtr,
	    Array<Array<Array<label64> > >& zc_pnts,
	    Array<Array<Array<label64> > >& zc_donor_pnts,
	    Array<Array<char*> >& zc_name,
	    Array<Array<Array<Array<label64> > > >& nodes);

	/**
	* @brief merge the interfaces nodes, save the nodes of prior zone
	*/
	void mergeInterfaceNodes(Array<Array<Array<label64> > >& zc_pnts,
	    Array<Array<Array<label64> > >& zc_donor_pnts,
    	Array<Array<char*> >& zc_name,
    	Array<Array<Array<Array<label64> > > >& nodes);

	/**
	* @brief find the points of the corresponding node.
	*/
	vector<label64>& findNeighborNodes(Array<Array<Array<label64> > >& zc_pnts,
    	int iZone, label64 owner_pnt, Array<Array<Array<Array<label64> > > >& nodes);

	void removeInterfaceNodes(Array<Array<Array<label64> > >& zc_pnts);
	/**
	* @brief read mesh file with CGNS format, parallel version
	*/
	void readCGNSFilePar(const char* filePtr, int fileIdx);
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
		this->eleNum_ = 0;
	};
	/**
	* @brief deconstructor
	*/
	virtual ~Mesh(){};
	/**
	* @brief read mesh and construct topology
	*/
	void readMesh(const Array<char*> filePtr)
	{
		// readCGNSFilePar(filePtr[0], 0);
		// readCGNSFilePar(filePtr[1], 1);
		for (int i = 0; i < filePtr.size(); ++i)
		{
			readCGNSFilePar(filePtr[i], i);
		}
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
	* @brief get block topology
	*/
	BlockTopology& getBlockTopology() {return this->blockTopo_;};

	/**
	* @brief get coordinates of Nodes without load balance
	*/
	Nodes& getNodes() {return this->nodes_;};

	/**
	* @brief get coordinates of nodes owned by this process
	*/
	Nodes& getOwnNodes() {return *this->ownNodes_;};

	/**
	* @brief get the count of elements of each file
	*/
	Array<label> getNodeNumGlobal() {return this->nodeNumGlobal_;};

	/**
	* @brief get the collections of section in CGNS file
	*/
	Array<Section>& getSections() {return this->secs_; };

	/**
	* @brief erase some sections in CGNS file
	*/
	void eraseSections(Array<bool>& erase)
	{
    	for (int i = 0; i < this->secs_.size(); ++i)
	    {
    	    if(erase[i]) this->secs_.erase(this->secs_.begin()+i);
    	}
	};

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
	* @brief fetch the coordinates of nodes owned by this process
	*/
	// void fetchNodes(Array<char*> fileArr);
	void fetchNodes(char* filename);

	/**
	* @brief get the map between the absolute index and the local index
	*/
	Table<label, label>& getCoordMap() {return this->coordMap_;};

	/**
	* @brief generate the block topology
	*/
	void generateBlockTopo() {blockTopo_.constructBlockTopology(topo_);};

	Array<label64>& getEleNumGlobal(){return this->eleNumGlobal_;};


};

} // end namespace HSF


#endif