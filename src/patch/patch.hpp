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
 * @File: patch.hpp
 * @author: Hanfeng GU
 * @Email:
 * @Date:   2019-10-11 10:09:18
 * @Last Modified by:   Hanfeng GU
 * @Last Modified time: 2019-11-08 11:26:39
 */

#ifndef PATCH_HPP
#define PATCH_HPP

#include "base.hpp"
#include "container.hpp"

namespace HSF
{
/**
 * @brief      This class describes a patch.
 */
class Patch
{
private:
  /**
   * @param 边界（face/node）大小
   */
  label size_;

  /**
   * @param 发送大小，如果压缩发送，则sendSize_ <=
   * size_，如果不是，则两者大小一样
   */
  label sendSize_;

  /**
   * @param 接收大小，如果压缩发送，则recvSize_ <=
   * size_，如果不是，则两者大小一样
   */
  label recvSize_;

  /**
   * @param send addressing to topo array.
   */
  label *addressing_;

  /**
   * @param patch type.
   */
  Word setType_;

  /**
   * @param the neighbor processor ID.
   */
  label nbrProcID_;

public:
  /**
   * @brief      Constructs a new instance.
   */
  Patch();

  /**
   * @brief      Constructs a new instance.
   */
  Patch(label size, label *addressing, label nbrProcID);

  /**
   * @brief      Destroys the object.
   */
  ~Patch();

  /**
   * @brief      获得发送个数
   * @return     压缩过的发送个数
   */
  inline label getSendSize() { return sendSize_; }

  /**
   * @brief      获得接收个数
   * @return     压缩过的接收个数
   */
  inline label getRecvSize() { return recvSize_; }

  /**
   * @brief      Gets IDs_ address.
   * @return     IDs_ address.
   */
  inline label *getAddressing()
  {
    return addressing_;
  }

  /**
   * @brief      Gets the patch type.
   * @return     The type.
   */
  inline Word getType() { return setType_; }

  /**
   * @brief      Gets the neighbor processor id.
   * @return     The neighbor processor ID.
   */
  inline label getNbrProcID() { return nbrProcID_; }

  /**
   * @brief      设定压缩发送和接收信息
   */
  void setSendRecvCompressed(label sendSize, label recvSize, label *sendMap);
};

}  // namespace HSF

#endif  //- end Patch_hpp
