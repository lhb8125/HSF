/**
* @file: nodes.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-14 16:47:22
* @last Modified by:   lenovo
* @last Modified time: 2019-11-29 10:54:30
*/
#include <cstdio>
#include "nodes.hpp"

namespace HSF
{

/*
* @brief get the count of nodes
*/
Label Nodes::size()
{
	return this->x_.size();
}

Label Nodes::getStart()
{
	return this->start_;
}

Label Nodes::getEnd()
{
	return this->end_;
}

void Nodes::setStart(const Label start)
{
	this->start_ = start;
}

void Nodes::setEnd(const Label end)
{
	this->end_ = end;
}

/**
* @brief default constructor
*/
Nodes::Nodes()
{
	refCount_ = new RefCounted();
}

/**
* @brief default deconstructor
*/
Nodes::~Nodes()
{
	this->x_.clear();
	this->y_.clear();
	this->z_.clear();
	if(refCount_->DecRefCount()<=0)
	{
		// printf("%d\n", refCount_->GetRefCount());
		DELETE_POINTER(refCount_);
	}
}

Nodes::Nodes(const Nodes& node)
{
	this->refCount_ = node.refCount_;
	this->xyz_ = node.xyz_;
	this->x_   = node.x_;
	this->y_   = node.y_;
	this->z_   = node.z_;
	this->refCount_->incRefCount();
}

/*
* @param x Coordinate X
* @param y Coordinate Y
* @param z Coordinate Z
*/
Nodes::Nodes(Array<Scalar>& x, Array<Scalar>& y, Array<Scalar>& z)
{
	refCount_ = new RefCounted();
	Label num = x.size();
	for (int i = 0; i < num; ++i)
	{
		x_.push_back(x[i]);
		y_.push_back(y[i]);
		z_.push_back(z[i]);
	}
	xyz_.num = num;
	xyz_.startIdx = new Label[num+1];
	xyz_.startIdx[0] = 0;
	xyz_.data = new Scalar[num*3];
	for (int i = 0; i < num; ++i)
	{
		xyz_.startIdx[i+1] = xyz_.startIdx[i]+3;
		xyz_.data[i*3+0] = x_[i];
		xyz_.data[i*3+1] = y_[i];
		xyz_.data[i*3+2] = z_[i];
	}
}
/*
* @param x Coordinate X
* @param y Coordinate Y
* @param z Coordinate Z
*/
Nodes::Nodes(Scalar* x, Scalar* y, Scalar* z, Label num)
{
	refCount_ = new RefCounted();
	for (int i = 0; i < num; ++i)
	{
		x_.push_back(x[i]);
		y_.push_back(y[i]);
		z_.push_back(z[i]);
	}
	xyz_.num = num;
	xyz_.startIdx = new Label[num+1];
	xyz_.startIdx[0] = 0;
	xyz_.data = new Scalar[num*3];
	for (int i = 0; i < num; ++i)
	{
		xyz_.startIdx[i+1] = xyz_.startIdx[i]+3;
		xyz_.data[i*3+0] = x_[i];
		xyz_.data[i*3+1] = y_[i];
		xyz_.data[i*3+2] = z_[i];
	}
}

void Nodes::copy(Nodes* nodes)
{
	int num = nodes->size();
	Array<Scalar> x = nodes->getX();
	Array<Scalar> y = nodes->getY();
	Array<Scalar> z = nodes->getZ();
	for (int i = 0; i < num; ++i)
	{
		x_.push_back(x[i]);
		y_.push_back(y[i]);
		z_.push_back(z[i]);

	}
	xyz_.num = num;
	xyz_.startIdx = new Label[num+1];
	xyz_.startIdx[0] = 0;
	xyz_.data = new Scalar[num*3];
	for (int i = 0; i < num; ++i)
	{
		xyz_.startIdx[i+1] = xyz_.startIdx[i]+3;
		xyz_.data[i*3+0] = x_[i];
		xyz_.data[i*3+1] = y_[i];
		xyz_.data[i*3+2] = z_[i];
	}
	this->refCount_ = nodes->refCount_;
	this->refCount_->incRefCount();
}

const Array<Scalar>& Nodes::getX()
{
	return this->x_;
}

const Array<Scalar>& Nodes::getY()
{
	return this->y_;
}

const Array<Scalar>& Nodes::getZ()
{
	return this->z_;
}

const ArrayArray<Scalar>& Nodes::getXYZ()
{
	return this->xyz_;
}

} // end namespace HSF