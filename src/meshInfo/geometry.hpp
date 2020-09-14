/*
 * @Author: your name
 * @Date: 2020-09-10 14:13:16
 * @LastEditTime: 2020-09-10 14:13:42
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /work_code/home/export/online3/amd_dev1/miaozh/work_code/unstructured_frame/src/meshInfo/geometry.hpp
 */
/**
* @file: compute.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-11-28 10:39:09
* @last Modified by:   lenovo
* @last Modified time: 2019-12-13 17:06:43
*/
#ifndef HSF_GEOMETRY_HPP
#define HSF_GEOMETRY_HPP

#include <cmath>
#include "utilities.h"

namespace HSF
{

// face area
scalar calculateQUADArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z);

scalar calculateTRIArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z);

scalar calculateFaceArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes);

// face normal vector
void calculateQUADNormVec(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, scalar* normVec);

void calculateTRINormVec(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, scalar* normVec);

void calculateFaceNormVec(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes, scalar* normVec);

// face center coordinate
void calculateFaceCenter(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes, scalar* center);

// cell volume
scalar calculateCellVol(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes);

scalar calculateHEXAVol(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z);

scalar calculateTETRAVol(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z);

// cell center coordinate
void calculateCellCenter(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes, scalar* center);
} // namespace HSF

#endif
