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

#ifndef Field_hpp
#define Field_hpp

#include "patch.hpp"
#include "utilities.hpp"

namespace HSF
{
/**
 * @brief      场
 * @tparam     T     label scalar
 */
template <typename T>
class Field
{
private:
  label ndim_;                         ///< 结构体维度
  label locSize_;                      ///< 本地单元的场维度
  label nbrSize_;                      ///< 与本地单元相邻的场维度
  T *data_;                            ///< 场数据
  Table<Word, T *> *sendBufferPtr_;    ///< 进程间通信发送buffer
  MPI_Request *sendRecvRequests_;      ///< mpi非阻塞通信句柄
  Word setType_;                       ///< 场所在数据集类型
  Table<Word, Patch *> *patchTabPtr_;  ///< 通信拓扑

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
  Field(Word setType, label ndim, label n, T *dataPtr);

  /**
   * @brief 构造函数，已知类型、维度、大小、数组指针、进程分块信息
   * @param[in] setType 数据类型，可能是cell、face、node等
   *@param[in] ndim 数据维度，一维、二维、三维
   *@param[in] n 数据段（结构体）个数，所以真实数据个数是 n*ndim
   *@param[in] dataPtr 数组的起始地址
   *@param[in] patchTab，region下存的patch信息
   */
  Field(Word setType,
        label ndim,
        label n,
        T *dataPtr,
        Table<Word, Table<Word, Patch *> *> &patchTab);

  /**
   * @brief 析构函数
   */
  ~Field();

  /**
   * @brief 获取本地单元场维度
   * @return  本地单元场维度
   */
  inline label getSize() { return locSize_; }

  /**
   * @brief 获取本地单元场结构体维度
   * @return  本地单元场结构体维度
   */
  inline label getDim() { return ndim_; }

  /**
   * @brief      Gets the data.
   * @return     The local data.
   */
  inline T *getLocalData() { return data_; }

  /**
   * @brief      Gets the neighbor processor data.
   * @return     The neighbor processor data.
   */
  inline T *getNbrData() { return &(data_[locSize_ * ndim_]); }

  /**
   * @brief 获取ghost单元场维度
   * @return ghost单元场维度
   */
  inline label getNbrSize() { return nbrSize_; }

  /**
   * @brief      Gets the type.
   * @return     The type.
   */
  inline Word getType() { return setType_; }

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

#define scalarField Field<scalar>
#define labelField Field<label>

#include "fieldI.hpp"

}  // namespace HSF
#endif  //- end Field_hpp
