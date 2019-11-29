/**
* @file: nodes.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-09 15:08:19
* @last Modified by:   lenovo
* @last Modified time: 2019-11-27 15:33:57
*/
#ifndef NODES_HPP
#define NODES_HPP
#include "utilities.hpp"

namespace HSF
{

/**
* @brief Coordinates of nodes
*/
class Nodes
{
private:
	
	ArrayArray<Scalar> xyz_; ///< AoS layout of coordinates
	
	Array<Scalar> x_; ///< Coordinate X
	
	Array<Scalar> y_; ///< Coordinate Y
	
	Array<Scalar> z_; ///< Coordinate Z
	
	Label start_; ///< the global start index of nodes
	
	Label end_; ///< the global end index of nodes

public:
	RefCounted *refCount_; ///< the count of reference pointers
	/**
	* @brief default constructor
	*/
	Nodes();
	/**
	* @brief constructor
	* @param[in] xyz AoS layout of coordinates
	*/
	Nodes(ArrayArray<Scalar>& xyz);
	/** 
	* @brief constructor
	* @param[in] x Coordinate X
	* @param[in] y Coordinate Y
	* @param[in] z Coordinate Z
	*/
	Nodes(Array<Scalar>& x, Array<Scalar>& y, Array<Scalar>& z);
	/** 
	* @brief constructor
	* @param[in] x Coordinate X
	* @param[in] y Coordinate Y
	* @param[in] z Coordinate Z
	*/
	Nodes(Scalar* x, Scalar* y, Scalar* z, Label num);
	/**
	* @brief copy constructor
	*/
	Nodes(const Nodes& node);
	/**
	* @brief deconstructor
	*/
	~Nodes();
	/**
	* @brief get the count of nodes
	* @return the count of nodes
	*/
	Label size();
	/**
	* @brief copy function
	*/
	void copy(Nodes* nodes);

	/**
	* @brief get X coordinates
	*/
	const Array<Scalar>& getX();

	/**
	* @brief get Y coordinates
	*/
	const Array<Scalar>& getY();

	/**
	* @brief get Z coordinates
	*/
	const Array<Scalar>& getZ();

	/**
	* @brief get XYZ coordinates
	*/
	const ArrayArray<Scalar>& getXYZ();

	/**
	* @brief get the global start index of nodes
	* @return global start index of nodes
	*/
	Label getStart();

	/**
	* @brief get the global end index of nodes
	* @return global end index of nodes
	*/
	Label getEnd();

	/**
	* @brief set the global start index of nodes
	* @param[in] start global start index of nodes
	*/
	void setStart(const Label start);

	/**
	* @brief set the global end index of nodes
	* @param[in] end global end index of nodes
	*/
	void setEnd(const Label end);
};

} // end namespace HSF

#endif