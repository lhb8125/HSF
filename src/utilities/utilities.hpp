/**
* @file: utilities.hpp
* @author: Liu Hongbin
* @brief: some useful utilities
* @date:   2019-10-08 15:12:44
* @last Modified by:   lenovo
* @last Modified time: 2019-11-29 10:27:18
*/
#ifndef UTILITIES_HPP
#define UTILITIES_HPP


#include <iostream>
#include <vector>
#include <algorithm>
#include "mpi.h"
#include "base.hpp"
#include "container.hpp"
#include "basicFunction.h"
#include "basicFunction.hpp"
#include "usingCpp.hpp"
#include "OStream.hpp"
#include "multiOStream.hpp"
#include "dummyOStream.hpp"
#include "communicator.hpp"
#include "communicationManager.hpp"
#include "mpiWrapper.hpp"
#include "utilInterfaces.h"

namespace HSF
{

// #define Array std::vector
#define String std::string
//#define Label label
//#define Scalar scalar
typedef label Label;
typedef scalar Scalar;

#define Inner 0
#define Boco 1

#define Terminate(location, content) {printf("Location: %s, error message: %s, file: %s, line: %d\n", location, content, __FILE__, __LINE__); exit(-1);}

#if(Label==long)
#define MPI_LABEL MPI_LONG
#else
#define MPI_LABEL MPI_INT
#endif

// #define DEBUG_METIS
// #define DEBUG

/**
* @brief count the reference pointers
*/
class RefCounted
{
private:
	int count_; ///< the count of reference pointers
public:
	/**
	* @brief constructor
	*/
	RefCounted():count_(1){};
	/**
	* @brief get the count
	*/
	int GetRefCount() const {return count_;}
	/**
	* @brief add a reference pointer
	*/
	void incRefCount() {count_++;}
	/**
	* @brief substract a reference pointer
	* @return the count of pointers
	*/
	int DecRefCount() {return --count_;}
};


template<class T> bool compareArray(Array<T>& a, Array<T>& b);

template <class T>
class ArrayArray : public RefCounted
{
public:
	Label  num; ///< size of structs
	Label* startIdx; ///< start index of structs
	T*     data; ///< structs
	RefCounted* refCount; /// count of reference pointers
	/**
	* @brief constructor
	*/
	ArrayArray()
	{
		refCount = new RefCounted();
		startIdx = NULL;
		data = NULL;
		// par_std_out_("constructor: %d\n", refCount->GetRefCount());
	};
	/**
	* @brief copy constructor
	*/
	ArrayArray(const ArrayArray<T> &arr)
	{
		this->num = arr.num;
		this->startIdx = arr.startIdx;
		this->data = arr.data;
		this->refCount = arr.refCount;
		refCount->incRefCount();
		// par_std_out_("copy constructor: %d\n", refCount->GetRefCount());
	};
	/**
	* @brief operator overload: =
	*/
	ArrayArray<T> operator=(const ArrayArray& arr)
	{
		ArrayArray<T> tmp;
		tmp.num = arr.num;
		tmp.startIdx = arr.startIdx;
		tmp.data = arr.data;
		tmp.refCount = arr.refCount;
		tmp.refCount->incRefCount();
		return tmp;
	};
	/**
	* @brief deconstructor
	*/
	~ArrayArray()
	{
		// std::cout<<this->GetRefCount()<<std::endl;
		// par_std_out_("deconstructor: %d\n", refCount->GetRefCount());
		if(refCount->DecRefCount()<=0)
		{
			// if(startIdx) delete[] startIdx;
			// if(data) delete[] data;
			DELETE_POINTER(data);
			DELETE_POINTER(startIdx);
			DELETE_POINTER(refCount);
		} else
		{
			startIdx = NULL;
			data = NULL;
			refCount = NULL;
		}
	};
	/**
	* @brief get the size of structs
	* @return the size of structs
	*/
	Label size() const {return num;};
	/**
	* @brief print the class to screen
	*/
	void display()
	{
		for (int i = 0; i < num; ++i)
		{
			std::cout<<"Item: "<<i<<" (";
			for (int j = startIdx[i]; j < startIdx[i+1]; ++j)
			{
				std::cout<<data[j];
				if(j<startIdx[i+1]-1) std::cout<<",";
			}
			std::cout<<")"<<std::endl;
		}
	}

};

/**
* @brief divide the data into two parts, 
*        the values of left part is smaller than pivot
*        the values of right part is larger than pivot
* @param[in] arr the unpartitioned data
* @param[in] l the lower bound
* @param[in] r the upper bound
* @return the index of partition boundary
*/
template<class T>
int partition(Array<Array<T> >& arr, int l , int r)
{
	int k=l,pivot = arr[r][0];
	for (int i = l; i < r; ++i)
	{
		if(arr[i][0]<=pivot)
		{
			arr[i].swap(arr[k]);
			k++;
		}
	}
	arr[k].swap(arr[r]);
	return k;
}

/**
* @brief quick sort algorithm for Array<Array<T>>
* @param[in][out] arr unsorted array
* @param[in] l the lower bound
* @param[in] r the upper bound
*/
template<class T>
void quicksortArray(Array<Array<T> >& arr, int l, int r)
{
	if(l<r)
	{
		int pivot = partition(arr, l, r);
		quicksortArray(arr, l, pivot-1);
		quicksortArray(arr, pivot+1, r);
	}
}

/**
* @brief eliminate the duplicate elements
*        and pick the unique ones and divide them into two parts
* @param[in][out] original array
* @return Label[0]: size of the unique one
*         Label[1]: size of the duplicate one
*/
template<class T>
Label* filterArray(Array<Array<T> >& arr)
{
	int num = arr.size();
	quicksortArray(arr, 0, num-1);
	// for(int i=0;i<tmp;i++) printf("%d\n", arr[0][i]);
	sort(arr[0].begin(), arr[0].end());
	int eraseNum = 0;
	Array<Array<T> > bndArr,innArr;
	for (int i = 0; i < num; ++i)
	{
		sort(arr[i].begin(), arr[i].end());
	}
	int end = 0;
	bool *isInner = new bool[num];
	for (int i = 0; i < num; ++i) { isInner[i] = false; }
	while(end < num)
	{
		// printf("%dth elements in %d\n", end, num);
		// printf("%d, %d\n", end, num);
		if(isInner[end]) {end++; continue;}
		// for (int i = end+1; i < num; ++i)
		int i = end+1;
		while(i<num && arr[i][0]==arr[end][0])
		{
			if(compareArray(arr[i],arr[end]))
			{
				isInner[i] = true;
				isInner[end] = true;
				innArr.push_back(arr[end]);
				break;
			}
			i++;
		}
		if(!isInner[end]) bndArr.push_back(arr[end]);
		end++;
	}
	// printf("old Array Num: %d, new Array Num: %d\n", arr.size(), newArr.size());
	arr.clear();
	arr.insert(arr.end(), bndArr.begin(), bndArr.end());
	arr.insert(arr.end(), innArr.begin(), innArr.end());
	printf("arr: %d, bndArr: %d, innArr: %d\n", arr.size(), bndArr.size(), innArr.size());
	Label *faceNum = new Label[2];
	faceNum[0] = bndArr.size();
	faceNum[1] = innArr.size();
	return faceNum;
};

/**
* @brief compare the two arrays if they are equal
* @param[in] a array A
* @param[in] b array B
* @return equal or not
*/
template<class T>
bool compareArray(Array<T>& a, Array<T>& b)
{
	int num_a = a.size();
	int num_b = b.size();
	num_a = std::min(num_a, num_b);
	// if(num_a!=num_b) return false;
	for (int i = 0; i < num_a; ++i)
	{
		// printf("%d, %d\n", a[i],b[i]);
		if(a[i]!=b[i]) return false;
	}
	return true;
};


/**
* @brief transform the Array<Array<>> to ArrayArray
* @param[in] arr array with Array<Array<>> format
* @param[out] res array with ArrayArray format
*/
template<class T>
void transformArray(const Array<Array<T> >& arr, ArrayArray<T>& res)
{
	int cellNum = arr.size();
	res.num = cellNum;
	res.startIdx = new Label[cellNum+1];
	res.startIdx[0] = 0;
	for (int i = 0; i < cellNum; ++i)
	{
		res.startIdx[i+1] = res.startIdx[i]+arr[i].size();	
	}
	// printf("cellNum: %d, nodeNum: %d\n", cellNum, res.startIdx[cellNum]);
	res.data = new T[res.startIdx[cellNum]];
	for (int i = 0; i < cellNum; ++i)
	{
		int k=0;
		for (int j = res.startIdx[i]; j < res.startIdx[i+1]; ++j)
		{
			res.data[j] = arr[i][k];
			k++;
		}
	}
};

/**
* @brief transform the ArrayArray to Array<Array<>>
* @param[in] arr array with ArrayArray format
* @param[out] res array with Array<Array<>> format
*/
template<class T>
void transformArray(const ArrayArray<T>& arr, Array<Array<T> >& res)
{
	int cellNum = arr.size();
	res.resize(cellNum);
	// printf("%d\n", cellNum);
	for (int i = 0; i < cellNum; ++i)
	{
		// printf("%d, %d\n", arr.startIdx[i], arr.startIdx[i+1]);
		for (int j = arr.startIdx[i]; j < arr.startIdx[i+1]; ++j)
		{
			res[i].push_back(arr.data[j]);
		}
	}
};

/**
* @brief find if the value exists in the array
* @param[in] arr array array
* @param[in] value array
* @return the index of entry
*/
template<class T>
Label findArray(Array<Array<T> >& arr, Array<T>& value)
{
	int num = arr.size();
	// printf("%d\n", num);
	// printf("*****************************************************\n");
	for (int i = 0; i < num; ++i)
	{
		// for (int j = 0; j < value.size(); ++j)
		// {
			// printf("(%d, %d)", arr[i][j], value[j]);
		// }
		// printf("\n");
		if(compareArray(arr[i], value)) {return i;};
	}
	// printf("*****************************************************\n");
	return -1;
}

/**
* @brief find if the value exists in the sorted array
* @param[in] arr sorted array array
* @param[in] value array
* @param[in] l the lower bound
* @param[in] r the upper bound
* @return the index of entry
*/
template<class T>
Label findSortedArray(Array<Array<T> >& arr, Array<T>& value, Label l, Label r)
{
	// Label num = std::min(arr.size(),value.size());
	Label num = arr.size();
	bool isFinded = false;
	Label m;
	while(l<r)
	{
		m = (l+r)/2;
		if(l==m)
		{
			if(arr[r][0]==value[0]) {isFinded=true; m=r;}
			break;
		}
		if(arr[m][0]<value[0]) l=m;
		else if(arr[m][0]>value[0]) r=m;
		else {isFinded = true; break;}
	}
	if(!isFinded) return -1;
	int i=m;
	while(i<num && arr[i][0]==value[0])
	{
		if(compareArray(arr[i], value)) return i;
		i++;
	}
	i=m;
	while(i>=0 && arr[i][0]==value[0])
	{
		if(compareArray(arr[i], value)) return i;
		i--;
	}
	// // printf("%d\n", num);
	// // printf("*****************************************************\n");
	// for (int i = 0; i < num; ++i)
	// {
	// 	// printf("%d, %d\n", i, num);
	// 	if(arr[i][0]!=value[0]) continue;
	// 	// if(compareArray(arr[i], value)) {return i;};
	// }
	// // printf("*****************************************************\n");
	return -1;
}

} // end namespace HSF

#endif
