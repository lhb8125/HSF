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
      sendTaskName_(NULL),
      recvTaskName_(NULL),
      patchTabPtr_(NULL),
      basicEle_(NULL)
{
}

// 复制构造函数 (采用深度拷贝的方式)
// 对于commcator来说 都是共用一个对象 使用指针的方式指向该对象 指针对象不涉及内存释放
template <typename T>
Field<T>::Field(const Field<T> &other_Field)
{
      ndim_ = other_Field.ndim_;
      locSize_ = other_Field.locSize_;
      nbrSize_ = other_Field.nbrSize_;
      setType_ = other_Field.setType_;

      label dataSize = locSize_ + nbrSize_;
      dataSize *= ndim_;

      if(other_Field.commcator_) this->commcator_ = other_Field.commcator_;

      data_ = new T[dataSize];

      memcpy(data_, other_Field.data_, dataSize * sizeof(T));
      if (!other_Field.sendBufferPtr_)
        sendBufferPtr_ = NULL;
      else
      {
        sendBufferPtr_ = new Table<Word, T *>;
        *sendBufferPtr_ = *other_Field.sendBufferPtr_;
      }
      if (!other_Field.patchTabPtr_)
        patchTabPtr_ = NULL;
      else
      {
        patchTabPtr_ = new Table<Word, Patch *>;
        *patchTabPtr_ = *other_Field.patchTabPtr_;
      }

      if ( (!sendTaskName_) && (!recvTaskName_) )
      {
        sendTaskName_ = NULL;
        recvTaskName_ = NULL;
      }
      else
      {
        int nPatches = (*patchTabPtr_).size();
        sendTaskName_ = new std::string[nPatches];
        recvTaskName_ = new std::string[nPatches];
      }

      basicEle_ = new BasicElement<T>[dataSize / ndim_];
      for (int i = 0; i < dataSize / ndim_; ++i)
      {
        basicEle_[i].num = ndim_;
        basicEle_[i].data = &data_[i * ndim_];
      }
}

template <typename T>
Field<T>::Field(Word setType, label ndim, label n, T *dataPtr,Communicator &other_comm)
    : setType_(setType),
      ndim_(ndim),
      locSize_(n),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendTaskName_(NULL),
      recvTaskName_(NULL),
      patchTabPtr_(NULL),
      commcator_(&other_comm)
{
  data_ = new T[n * ndim];
  memcpy(data_, dataPtr, n * ndim * sizeof(T));
  // 初始化结构体指针，不包含ghost
  basicEle_ = new BasicElement<T>[n];
  for (int i = 0; i < n; ++i)
  {
    basicEle_[i].num = ndim;
    basicEle_[i].data = &data_[i*ndim];
  }
}

template <typename T>
Field<T>::Field(Word setType,
                label ndim,
                label n,
                T *dataPtr,
                Table<Word, Table<Word, Patch *> *> &patchTab,Communicator &other_comm)
    : setType_(setType),
      ndim_(ndim),
      locSize_(n),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendTaskName_(NULL),
      recvTaskName_(NULL),
      patchTabPtr_(NULL),
      commcator_(&other_comm)
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
    par_std_out("No such type patch in region patchTab %s\n", setType.c_str());
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
  // 初始化结构体指针，包含ghost
  basicEle_ = new BasicElement<T>[sizeAll];
  for (int i = 0; i < sizeAll; ++i)
  {
    basicEle_[i].num = ndim;
    basicEle_[i].data = &data_[i*ndim];
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
    if (!sendTaskName_ && !recvTaskName_)
    {
      sendTaskName_ = new std::string[nPatches];
      recvTaskName_ = new std::string[nPatches];
    }

    Table<Word, Patch *>::iterator it = patches.begin();

    //- if nbrdata not created
    // printf("%d\n", COMM::getGlobalSize());
    // if (nbrSize_ <= 0)
    if (nbrSize_ <= 0 && COMM::getGlobalSize() > 1)
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

      char ch[128];

      sprintf(
          ch, "Send_%05d_Recv_%05d", this->commcator_->getMyId(), patchI.getNbrProcID());
      sendTaskName_[i] = ch;
      sprintf(
          ch, "Send_%05d_Recv_%05d", patchI.getNbrProcID(), this->commcator_->getMyId());
      recvTaskName_[i] = ch;

      this->commcator_->send(sendTaskName_[i],
                             patchI_sendBuffer,
                             sendSize * ndim_  * sizeof(T),
                             patchI.getNbrProcID());

      this->commcator_->recv(recvTaskName_[i],
                            patchI_recvBuffer,
                            recvSize * ndim_ * sizeof(T),
                            patchI.getNbrProcID());

      recvArrayPtr += recvSize * ndim_;
    }
  }
}

template <typename T>
label Field<T>::checkSendStatus()
{
  if (sendTaskName_ && recvTaskName_)
  {
    Table<Word, Patch *> &patches = *patchTabPtr_;
    label nPatches = patches.size();
    for(int i = 0;i < nPatches;++i){
      this->commcator_->finishTask(recvTaskName_[i]);
      this->commcator_->finishTask(sendTaskName_[i]);
    }
    DELETE_POINTER(sendTaskName_);
    DELETE_POINTER(recvTaskName_);
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
  if(data_)DELETE_POINTER(data_);
  freeSendRecvBuffer();
  if(sendTaskName_) DELETE_POINTER(sendTaskName_);
  if(recvTaskName_) DELETE_POINTER(recvTaskName_);
  if(basicEle_) DELETE_POINTER(basicEle_);
}


template<typename SetType, typename Element>
Field_new<SetType, Element>::Field_new()
    : locSize_(0),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL)
{
}

template<typename SetType, typename Element>
Field_new<SetType, Element>::Field_new(label n, Element *dataPtr)
    : locSize_(n),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL)
{
  label32 dim = *dataPtr.size();
  // data_ = new Element[n];
  // memcpy(data_, dataPtr, n * ndim * sizeof(T));
  // // 初始化结构体指针，不包含ghost
  // basicEle_ = new BasicElement<T>[n];
  // for (int i = 0; i < n; ++i)
  // {
  //   basicEle_[i].num = ndim;
  //   basicEle_[i].data = &data_[i*ndim];
  // }
}

