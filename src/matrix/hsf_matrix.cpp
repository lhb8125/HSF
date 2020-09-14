/*
 * @Author: zhmiao
 * @Date: 2020-09-03 09:36:18
 * @LastEditTime: 2020-09-08 10:23:55
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /work_code/unstructured_frame/src/matrix/hsf_matrix.cpp
 */
#include "hsf_matrix.hpp"

namespace HSF
{


// 默认的构造函数
template <typename T>
Matrix<T>::Matrix()
{
}

// 构造函数 
template <typename T>
Matrix<T>::Matrix(Field<T> &upper, Field<T> &lower, Field<T> &diag)
        : upper_(upper), lower_(lower), diag_(diag)
{
    if((&(upper_->getCommunicator()) != &(lower_->getCommunicator())) || (&(upper_->getCommunicator()) != &(diag_->getCommunicator())) )
    {
        Terminate("upper_ and lower_  , diag_ ","are not the same commcator");
    }
    this->commcator_=&(upper_->getCommunicator());
}

// 析构函数
template <typename T>
Matrix<T>::~Matrix()
{   
}


} //namespace HSF