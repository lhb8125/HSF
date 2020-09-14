/*
 * @Author: your name
 * @Date: 2020-09-09 15:08:36
 * @LastEditTime: 2020-09-09 15:35:28
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /work_code/unstructured_frame/src/interfaces/commInterfaces.cpp
 */

#include "commInterfaces.hpp"


#ifdef __cplusplus
extern "C"
{
#endif

// 获得全局通信域对象
void getglobalcommunicator_(label64 * commPtr)
{
    *(Communicator **) commPtr = & COMM::getGlobalComm();
}

// 获得通信域对象的进程编号和进程大小
void getcommidsize_(label64 *commPtr,label *rank,label *size)
{
    Communicator *commcator = (Communicator *)*commPtr;
    *rank = commcator->getMyId();
    *size = commcator->getMySize();
}

// 通信域进程进行同步
void commbarrier_(label64 *commPtr)
{
    Communicator *commcator = (Communicator *)*commPtr;
    commcator->barrier();
}



#ifdef __cplusplus
}
#endif
