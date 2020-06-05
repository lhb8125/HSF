/*
* @author: liu hongbin
* @brief: vector interfaces
* @email: lhb8134@foxmail.com
* @date:   2020-02-22 12:09:40
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-22 14:48:06
*/

#ifndef VECTORINTERFACES_HPP
#define VECTORINTERFACES_HPP

#include "mpi.h"
// #include "voidtor.hpp"

// using namespace HSF;

// #define void Vector<double>
#define HSF_COMM MPI_COMM_WORLD

/**
* @brief 给串行向量v开辟空间
* @param[in]  v 待开辟空间向量
* @param[in]  n 向量大小
*/
void VecConstrSeq (void* v, int n);

/**
* @brief 给并行向量v开辟空间
* @param[in]  v 待开辟空间向量
* @param[in]  n 向量大小
* @param[in]  nbghosts ghost元素个数
* @param[in]  ghosts ghost元素的全局位置
*/
void VecConstrPar (void* v, int n, int nbghosts, int* ghosts);

/**
* @brief 释放向量空间
*/
void VecDestr (void* v);

/**
* @brief 给向量指定位置赋值
*/
void VecSetCmp (void* v, int ind, double value);

/**
* @brief 给向量所有位置赋相同的值
*/
void VecSetAllCmp (void* v, double value);

/**
* @brief 获得向量指定位置的值，当前进程上
* @param[in]  v 取值向量
* @param[in]  ind 取值位置
* @return  返回向量指定位置的值
*/
double VecGetCmp (void* v, int ind);


/**
* @brief Obtains the local ghosted representation of a parallel vector
* @param[in]  vg the global vector
* @param[out]  vl the local (ghosted) representation, NULL if g is not ghosted
*/
void VecGhostGetLocalForm(void* vg, void* vl);

/**
* @brief Restores the local ghosted representation of a parallel vector obtained with V_GhostGetLocalForm()
* @param[in]  vg the global vector
* @param[out]  vl the local (ghosted) representation
*/
void VecGhostRestoreLocalForm(void* vg, void* vl);

/**
* @brief 启动向量ghost部分更新
* @param[in]  v 被更新向量
*/
void VecGhostUpdate(void* v);

/**
* @brief 复制向量
* @param[in]  vfrom 被复制向量
* @param[out]  vto 结果向量
*/
void VecCopy(void* vfrom, void* vto);

/**
* @brief 计算向量第二范数
* @param[in]  v 被求向量
* @param[out]  norm2 v的第二范数
*/
void VecNorm2(void* v, double *norm2);

/**
* @brief w = alpha * x + y
* @param[out]  w 结果向量
*/
void VecWAXPY(void* w, double alpha, void* x, void* y);

/**
* @brief y = alpha * x + y
* @param[out]  y 结果向量
*/
void VecAXPY(void* y, double alpha, void* x);

/**
* @brief 点乘 w = x*y
* @param[out]  w 点乘后返回的结果向量
*/
void VecPointwiseMult(void* w, void* x, void* y);

/**
* @brief 获取向量的最大值及其位置
* @param[in]  x 目标向量
* @param[out]  loc 最大值位置
* @param[out]  val 最大值
*/
void VecMax(void* v, int *loc, double *val);

/**
* @brief 向量写入到文件
*/
void VecWrite(void* x);

#endif