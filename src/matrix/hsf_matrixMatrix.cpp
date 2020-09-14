/*
 * @Author: your name
 * @Date: 2020-09-03 14:33:19
 * @LastEditTime: 2020-09-08 14:04:11
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /work_code/unstructured_frame/src/matrix/hsf_matrixMatrix.cpp
 */
#include "hsf_matrixMatrix.hpp"

namespace HSF
{
    

// 默认构造函数
template <typename T>
matrixMatrix<T>::matrixMatrix()
{    
}

// 构造函数
template <typename T >
matrixMatrix<T>::matrixMatrix(const Array<Matrix<T> *> & matrix,const Array<Word> &matrixName)
{
    if( matrix.size() != matrixName.size() )
        Terminate("The matrix and the matrixName", " array are inconsistent in size\n");
    
    data_.clear();
    comm_.clear();
    for(int i = 0;i<matrix.size();++i){ 
        data_[matrixName[i]] = matrix[i];
        comm_[matrixName[i]] = &matrix[i]->getCommunicator();
    }
}

// 析构函数
template <typename T >
matrixMatrix<T>::~matrixMatrix()
{
}


// 插入矩阵函数
template <typename T>
void matrixMatrix<T>::insertMatirx(const Word &name , const Matrix<T> * matrix)
{
    if(data_.count(name))
    {
        std::cout<<"name: "<<name<<std::endl;
        Terminate("matrix_"," already has an object named \n");
    }

    data_[name] = matrix;
}

// 插入数组矩阵和对应的名字
template <typename T >
void matrixMatrix<T>::insertMatirx(const Array<Matrix<T> *> &matrix , const Array<Word> & matrixName)
{
    if( matrix.size() != matrixName.size() )
        Terminate("The matrix and the matrixName ","array are inconsistent in size\n");
    for(int i = 0;i<matrix.size();++i){ 
        if(data_.count(matrixName[i]))
        {
            std::cout<<"name: "<<matrixName[i]<<std::endl;
            Terminate("matrix_ ","already has an object named \n");
        }
        data_[matrixName[i]] = matrix[i];
        comm_[matrixName[i]] = &matrix[i]->getCommunicator();
    }

}


// 设置类的名字 
template <typename T>
void matrixMatrix<T>::setClassName(const Word & className)
{
    className_ = className;
}

// 获得对应的矩阵对象
template <typename T>
void matrixMatrix<T>::getMatrix(const Word &matrixName , Matrix<T> *matrix)
{
    if(!data_.count(matrixName))
        Terminate("matrix_"," has not an object named \n");

    return data_[matrixName];
}

}// namespace HSF