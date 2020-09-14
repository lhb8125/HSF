/*
 * @Author: your name
 * @Date: 2020-09-09 15:02:22
 * @LastEditTime: 2020-09-12 17:17:09
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /work_code/unstructured_frame/src/interfaces/commInterfaces.hpp
 */
#ifndef HSF_COMMINTERFACE_HPP
#define HSF_COMMINTERFACE_HPP

#include "utilities.h"


#ifdef __cplusplus
extern "C"
{
#endif

// 获得全局通信域对象
void getglobalcommunicator_(label64 * commPtr);

// 获得通信域对应的进程编号和进程大小
void getcommidsize_(label64 *commPtr , label *rank,label *size);

// 通信域进程进行同步
void commbarrier_(label64 *commPtr);


#ifdef __cplusplus
}
#endif



#endif
