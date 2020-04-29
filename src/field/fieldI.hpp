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

/*
 * @File: fieldI.hpp
 * @Author: Hanfeng GU
 * @Email:  hanfenggu@gmail.com
 * @Date:   2019-09-18 16:04:01
 * @Last Modified by:   Hanfeng
 * @Last Modified time: 2019-11-28 14:11:47
 */

/*
 * @brief: Implementation of template functions in Field class
 */

template <typename T>
Field<T>::Field()
    : setType_(NULL),
      ndim_(0),
      locSize_(0),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL)
{
}

template <typename T>
Field<T>::Field(Word setType, label ndim, label n, T *dataPtr)
    : setType_(setType),
      ndim_(ndim),
      locSize_(n),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL)
{
  data_ = new T[n * ndim];
  memcpy(data_, dataPtr, n * ndim * sizeof(T));
}

template <typename T>
Field<T>::Field(Word setType,
                label ndim,
                label n,
                T *dataPtr,
                Table<Word, Table<Word, Patch *> *> &patchTab)
    : setType_(setType),
      ndim_(ndim),
      locSize_(n),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL)
{
  label sizeAll = 0;
  Table<Word, Table<Word, Patch *> *>::iterator it = patchTab.find(setType);
  if (it != patchTab.end())
  {
    patchTabPtr_ = patchTab[setType];
    Table<Word, Patch *> &patches = *patchTabPtr_;
    Table<Word, Patch *>::iterator it2;
    for (it2 = patches.begin(); it2 != patches.end(); ++it2)
    {
      Patch &patchI = *(it2->second);
      sizeAll += patchI.getRecvSize();
    }
    nbrSize_ = sizeAll;
  }
  else
  {
    par_std_out_("No such type patch in region patchTab%s\n", setType.c_str());
  }
  sizeAll += locSize_;
  data_ = new T[sizeAll * ndim_];
  memcpy(data_, dataPtr, n * ndim * sizeof(T));
  forAll(i, nbrSize_)
  {
    forAll(j, ndim)
    {
      data_[(n + i) * ndim + j] = 0;
    }
  }
}

template <typename T>
void Field<T>::initSend()
{
  //- create
  if (patchTabPtr_)
  {
    //- create
    if (!sendBufferPtr_)
    {
      sendBufferPtr_ = new Table<Word, T *>;
    }

    Table<Word, Patch *> &patches = *patchTabPtr_;
    Table<Word, T *> &sendBuffer = *sendBufferPtr_;
    label nPatches = patches.size();

    //- create memory for MPI_Request
    if (!sendRecvRequests_)
    {
      sendRecvRequests_ = new MPI_Request[2 * nPatches];
    }

    Table<Word, Patch *>::iterator it = patches.begin();

    //- if nbrdata not created
    if (nbrSize_ <= 0)
    {
      nbrSize_ = 0;
      for (it = patches.begin(); it != patches.end(); ++it)
      {
        Patch &patchI = *(it->second);
        nbrSize_ += patchI.getRecvSize();
      }

      T *dataOld = data_;
      data_ = new T[(locSize_ + nbrSize_) * ndim_];
      memcpy(data_, dataOld, locSize_ * ndim_ * sizeof(T));
      DELETE_POINTER(dataOld);
    }

    T *recvArrayPtr = &(data_[locSize_ * ndim_]);
    it = patches.begin();
    for (label i = 0; i < nPatches, it != patches.end(); ++i, ++it)
    {
      Patch &patchI = *(it->second);
      label sendSize = patchI.getSendSize();
      label recvSize = patchI.getRecvSize();
      Word patchName = it->first;
      //- here memory created
      if (!sendBuffer[patchName])
      {
        sendBuffer[patchName] = new T[sendSize * ndim_];
      }

      T *patchI_sendBuffer = sendBuffer[patchName];
      T *patchI_recvBuffer = recvArrayPtr;

      label *patchI_addressing = patchI.getAddressing();

      for (label j = 0; j < sendSize; ++j)
      {
        for (label k = 0; k < ndim_; ++k)
        {
          patchI_sendBuffer[j * ndim_ + k] =
              data_[patchI_addressing[j] * ndim_ + k];
        }
      }

      MPI_Isend(patchI_sendBuffer,
                sendSize * ndim_ * sizeof(T),
                MPI_BYTE,
                patchI.getNbrProcID(),
                1,
                MPI_COMM_WORLD,
                &sendRecvRequests_[i]);

      MPI_Irecv(patchI_recvBuffer,
                recvSize * ndim_ * sizeof(T),
                MPI_BYTE,
                patchI.getNbrProcID(),
                1,
                MPI_COMM_WORLD,
                &sendRecvRequests_[i + nPatches]);

      recvArrayPtr += recvSize * ndim_;
    }
  }
}

template <typename T>
label Field<T>::checkSendStatus()
{
  if (sendRecvRequests_)
  {
    Table<Word, Patch *> &patches = *patchTabPtr_;
    label nPatches = patches.size();
    MPI_Waitall(2 * nPatches, &sendRecvRequests_[0], MPI_STATUSES_IGNORE);
    DELETE_POINTER(sendRecvRequests_);
  }
  return 1;
}

template <typename T>
void Field<T>::freeSendRecvBuffer()
{
  //- free sendBufferPtr_
  if (sendBufferPtr_)
  {
    Table<Word, T *> &sendBuffer = *sendBufferPtr_;

    typename Table<Word, T *>::iterator it = sendBuffer.begin();

    for (it = sendBuffer.begin(); it != sendBuffer.end(); ++it)
    {
      DELETE_POINTER(it->second);
    }
    DELETE_OBJECT_POINTER(sendBufferPtr_);
  }
}

template <typename T>
Field<T>::~Field()
{
  DELETE_POINTER(data_);
  freeSendRecvBuffer();
}
