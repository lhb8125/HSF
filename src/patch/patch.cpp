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
 * @file: patch.cpp
 * @author: Hanfeng GU
 * @Email:
 * @Date:   2019-10-11 10:09:42
 * @Last Modified by:   Hanfeng GU
 * @Last Modified time: 2019-11-08 14:14:25
 */

#include "patch.hpp"
#include "utilities.hpp"

HSF::Patch::Patch(label size, label *IDs, label nbrProcID)
    : size_(size),
      addressing_(NULL),
      nbrProcID_(nbrProcID),
      sendSize_(size),
      recvSize_(size)
{
  addressing_ = new label[size];
  memcpy(addressing_, IDs, size * sizeof(label));
}

HSF::Patch::~Patch() { DELETE_POINTER(addressing_); }

void HSF::Patch::setSendRecvCompressed(label sendSize,
                                       label recvSize,
                                       label *sendMap)
{
  sendSize_ = sendSize;
  recvSize_ = recvSize;

  DELETE_POINTER(addressing_);
  addressing_ = new label[sendSize];
  memcpy(addressing_, sendMap, sendSize * sizeof(label));
}
