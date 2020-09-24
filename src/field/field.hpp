/*
The MIT License

Copyright (c) 2019 Hanfeng GU <hanfenggu@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/**
 * @file: field.hpp
 * @author: Hanfeng GU
 * @Email:  hanfenggu@gmail.com
 * @Date:   2019-09-18 16:03:45
 * @Last Modified by:   Hanfeng GU
 * @Last Modified time: 2019-11-14 09:16:47
 */

#ifndef HSF_Field_hpp
#define HSF_Field_hpp

#include "patch.hpp"
#include "utilities.h"
// #include "utilInterfaces.h"
#include "utilityInterfaces.h"
#include "setType.hpp"

namespace HSF
{
template<typename SetType, typename Element>
class Field
{
private:
  label locSize_;
  label nbrSize_;
  Element *ele_;
  char *data_;
  Table<Word, char *> *sendBufferPtr_;    ///< 进程间通信发送buffer
  MPI_Request *sendRecvRequests_;      ///< mpi非阻塞通信句柄
  Table<Word, Patch *> *patchTabPtr_;  ///< 通信拓扑
  Array<Word> sendTaskName_;
  Array<Word> recvTaskName_;

  Communicator *commcator_;

public:
  /**
   * @brief 构造函数
   */
  Field();

  /**
   * @brief 构造函数，已知类型、维度、大小、数组指针
   * @param[in] setType 数据类型，可能是cell、face、node等
   * @param[in] ndim 数据维度，一维、二维、三维
   * @param[in] n 数据段（结构体）个数，所以真实数据个数是 n*ndim
   * @param[in] dataPtr 数组的起始地址
   */
  Field(label n, Element *dataPtr, Communicator &other_comm);

  /**
   * @brief 构造函数，已知类型、维度、大小、数组指针、进程分块信息
   * @param[in] setType 数据类型，可能是cell、face、node等
   *@param[in] ndim 数据维度，一维、二维、三维
   *@param[in] n 数据段（结构体）个数，所以真实数据个数是 n*ndim
   *@param[in] dataPtr 数组的起始地址
   *@param[in] patchTab，region下存的patch信息
   */
  Field(label n,
        Element *dataPtr,
        Table<Word, Table<Word, Patch *> *> &patchTab,
        Communicator &other_comm);

  /**
   * @brief 析构函数
   */
  ~Field();

  /**
  * @brief 重载[]
  */
  inline Element& operator[](const int i){ return ele_[i]; }

  /**
  * @brief 获取通信子
  */
  Communicator & getCommunicator(){ return *commcator_;};

  /**
   * @brief 获取本地单元场维度
   * @return  本地单元场维度
   */
  inline label getSize() { return locSize_; }

  /**
   * @brief      Gets the type.
   * @return     The type.
   */
  inline Word getType() 
  {
    if(typeid(SetType)==typeid(Cell))
      return "cell";
    else if(typeid(SetType)==typeid(Face))
      return "face";
    else
      Terminate("getType", "the setType is not supported");
  }

  /**
  * @brief get the dim
  */
  inline label32 getDim()
  {
    return ele_->getNum();
  }

  /**
   * @brief      Gets the data.
   * @return     The local data.
   */
  inline char *getLocalData() {
    // printf("pointer: %p\n", data_);
      return data_; 
  }

  /**
   * @brief      Gets the element.
   * @return     The local element.
   */
  inline Element *getLocalElement() {
      return ele_; 
  }

  /**
   * @brief      Gets the neighbor processor data.
   * @return     The neighbor processor data.
   */
  inline char *getNbrData() { return &(data_[locSize_ * (*ele_).getNum()]); }

  /**
   * @brief      Gets the neighbor processor element.
   * @return     The neighbor processor element.
   */
  inline Element *getNbrElement() { return &(ele_[locSize_]); }

  /**
   * @brief 获取ghost单元场维度
   * @return ghost单元场维度
   */
  inline label getNbrSize() { return nbrSize_; }

  /**
   * @brief 赋值进程通信拓扑
   * @param[in] ptr 本进程通信拓扑
   */
  inline void setPatchTab(Table<Word, Patch *> *ptr) { patchTabPtr_ = ptr; }

  /**
   * @brief 获取进程通信拓扑
   * @return 本进程通信拓扑
   */
  inline Table<Word, Patch *> *getPatchTab() { return patchTabPtr_; }

  /**
   * @brief      Initializes the send buffer and start iSend and iRecv.
   */
  void initSend();

  /**
   * @brief      check if we have receive the data from neighbor processors
   * @return     if finished, return 1, else return 0
   */
  label checkSendStatus();

  /**
   * @brief      free the memory of communication
   */
  void freeSendRecvBuffer();
};

#include "fieldI.hpp"

}  // namespace HSF
#endif  //- end Field_hpp
