/*
 * @Author: your name
 * @Date: 2020-09-03 13:49:35
 * @LastEditTime: 2020-09-08 14:03:21
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /work_code/unstructured_frame/src/matrix/hsf_matrixMatirx.h
 */

#ifndef HSF_MATRIXMATRIX_HPP
#define HSF_MATRIXMATRIX_HPP

#include "hsf_matrix.hpp"
#include "utilities.h"


namespace HSF
{ 


template <typename T = scalar>
class matrixMatrix
{

private:
    Table<Word , Matrix<T> *> data_;
    Table<Word , Communicator *> comm_;

    static Word className_;

public:
    // 默认构造构造函数
    matrixMatrix();

    //构造函数
    matrixMatrix(const Array<Matrix<T> *> & matirx,const Array<Word> & matrixName);
    
    // 析构函数
    ~matrixMatrix();

    // 插入对应的矩阵函数
    void insertMatirx(const Word &name,const Matrix<T> * matrix);

    // 插入对应的数组矩阵函数
    void insertMatirx(const Array<Matrix<T> *> &matrix , const Array<Word> & matrixName);

    // 设置类的名字
    static void  setClassName(const Word & className);

    // 获得对应的矩阵对象
    void getMatrix(const Word &matrixName , Matrix<T> *matrix);

};// class matrixMatrix



}//  namespace HSF

#endif