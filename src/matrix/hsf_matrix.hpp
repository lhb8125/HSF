/*
 * @Author: zhmiao
 * @Date: 2020-09-03 09:22:27
 * @LastEditTime: 2020-09-08 10:22:48
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /work_code/unstructured_frame/src/matrix/hsf_matrix.hpp
 */
#ifndef HSF_MATRIX_HPP
#define HSF_MATRIX_HPP
#include "field.hpp"
#include "utilities.h"
namespace HSF
{

template <typename T>
class Matrix
{
private:
    Field<T> upper_, lower_, diag_;
    Communicator * commcator_;

public:
    // 默认的构造函数
    Matrix();

    // 构造函数
    Matrix(Field<T> &upper, Field<T> &lower, Field<T> &diag);

    // 析构函数
    ~Matrix();

    Communicator & getCommunicator(){return *commcator_;};

}; // class matrix



}// namespace HSF

#endif