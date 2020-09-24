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

template<typename SetType, typename Element>
Field<SetType, Element>::Field()
    : locSize_(0),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL)
{
}

template<typename SetType, typename Element>
Field<SetType, Element>::Field(label n, Element *dataPtr,
    Communicator &other_comm)
    : locSize_(n),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL),
      commcator_(&other_comm)
{
  label32 size = dataPtr->length()*dataPtr->getNum();
  data_ = new char[n*size];
  ele_ = new Element[n];
  for (int i = 0; i < n; ++i)
  {
    memcpy(&data_[i*size], dataPtr[i].getData(), size);
    ele_[i].setData(&data_[i*size]);
    ele_[i].setNum(dataPtr[i].getNum());
  }
}


template<typename SetType, typename Element>
Field<SetType, Element>::Field(label n, Element *dataPtr,
    Table<Word, Table<Word, Patch *> *> &patchTab, Communicator &other_comm)
    : locSize_(n),
      nbrSize_(0),
      data_(NULL),
      sendBufferPtr_(NULL),
      sendRecvRequests_(NULL),
      patchTabPtr_(NULL),
      commcator_(&other_comm)
{
  label sizeAll = 0;
  Word setType = "cell";
  if(typeid(SetType)==typeid(Cell))
    setType = "cell";
  else if(typeid(SetType)==typeid(Face))
    setType = "face";
  else
    Terminate("Field", "the settype is not supported");
  // par_std_out("%s\n",setType);
  Table<Word, Table<Word, Patch *> *>::iterator it = patchTab.find(setType);
  par_std_out("%s\n",setType.c_str());
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
  } else
    par_std_out("No such type patch in region patchTab %s\n", setType.c_str());
  par_std_out("finish get patchtab\n");
  nbrSize_ = sizeAll;


  label32 size = dataPtr->length()*dataPtr->getNum();
  sizeAll += locSize_;
  data_ = new char[sizeAll * size];

  ele_ = new Element[sizeAll];
  for (int i = 0; i < n; ++i)
  {
    memcpy(&data_[i*size], dataPtr[i].getData(), size);
    ele_[i].setData(&data_[i*size]);
    ele_[i].setNum(dataPtr->getNum());
  }
  for (int i = n; i < sizeAll; ++i)
  {
    ele_[i].setData(&data_[i*size]);
    ele_[i].setNum(dataPtr->getNum());
    ele_[i].setVal(0);
  }
}

template<typename SetType, typename Element>
void Field<SetType, Element>::initSend()
{
  //- create
  if (patchTabPtr_)
  {
    //- create
    if (!sendBufferPtr_)
    {
      sendBufferPtr_ = new Table<Word, char *>;
    }

    Table<Word, Patch *> &patches = *patchTabPtr_;
    Table<Word, char *> &sendBuffer = *sendBufferPtr_;
    label nPatches = patches.size();

    //- create memory for MPI_Request
    // if (!sendTaskName_ && !recvTaskName_)
    // {
    //   sendTaskName_ = new std::string[nPatches];
    //   recvTaskName_ = new std::string[nPatches];
    // }

    Table<Word, Patch *>::iterator it = patches.begin();

    //- if nbrdata not created
    label32 eleLen = ele_->length()*ele_->getNum();
    if (nbrSize_ <= 0 && COMM::getGlobalSize() > 1)
    {
      Terminate("initSend", "reconstruct data");
      nbrSize_ = 0;
      for (it = patches.begin(); it != patches.end(); ++it)
      {
        Patch &patchI = *(it->second);
        nbrSize_ += patchI.getRecvSize();
      }

      char *dataOld = data_;
      data_ = new char[(locSize_ + nbrSize_) * eleLen];
      memcpy(data_, dataOld, locSize_ * eleLen);
      DELETE_POINTER(dataOld);
    }

    char *recvArrayPtr = &(data_[locSize_ * eleLen]);
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
        sendBuffer[patchName] = new char[sendSize * eleLen];
      }

      char *patchI_sendBuffer = sendBuffer[patchName];
      char *patchI_recvBuffer = recvArrayPtr;

      label *patchI_addressing = patchI.getAddressing();

      for (label j = 0; j < sendSize; ++j)
      {
        memcpy(&patchI_sendBuffer[j*eleLen],
          &data_[patchI_addressing[j]*eleLen],
          eleLen);
      }

      char ch[128];

      sprintf(
          ch, "Send_%05d_Recv_%05d", this->commcator_->getMyId(), patchI.getNbrProcID());
      sendTaskName_.push_back(ch);
      sprintf(
          ch, "Send_%05d_Recv_%05d", patchI.getNbrProcID(), this->commcator_->getMyId());
      recvTaskName_.push_back(ch);

      this->commcator_->send(sendTaskName_[i],
                             patchI_sendBuffer,
                             sendSize * eleLen,
                             patchI.getNbrProcID());

      this->commcator_->recv(recvTaskName_[i],
                            patchI_recvBuffer,
                            recvSize * eleLen,
                            patchI.getNbrProcID());

      recvArrayPtr += recvSize * eleLen;
    }
  }
}

template<typename SetType, typename Element>
label Field<SetType, Element>::checkSendStatus()
{
  if (sendTaskName_.size()>0 && recvTaskName_.size()>0)
  {
    Table<Word, Patch *> &patches = *patchTabPtr_;
    label nPatches = patches.size();
    for(int i = 0;i < nPatches;++i){
      this->commcator_->finishTask(recvTaskName_[i]);
      this->commcator_->finishTask(sendTaskName_[i]);
    }
    sendTaskName_.clear();
    recvTaskName_.clear();
  }
  return 1;
}

template<typename SetType, typename Element>
void Field<SetType, Element>::freeSendRecvBuffer()
{
  //- free sendBufferPtr_
  if (sendBufferPtr_)
  {
    Table<Word, char *> &sendBuffer = *sendBufferPtr_;

    typename Table<Word, char *>::iterator it = sendBuffer.begin();

    for (it = sendBuffer.begin(); it != sendBuffer.end(); ++it)
    {
      DELETE_POINTER(it->second);
    }
    DELETE_OBJECT_POINTER(sendBufferPtr_);
  }
}

template<typename SetType, typename Element>
Field<SetType, Element>::~Field()
{
  patchTabPtr_ = NULL;
  commcator_   = NULL;
  // par_std_out("deconstruct Field\n");
  // if(data_)DELETE_POINTER(data_);
  // freeSendRecvBuffer();
  // if(sendTaskName_) DELETE_POINTER(sendTaskName_);
  // if(recvTaskName_) DELETE_POINTER(recvTaskName_);
  // if(ele_) DELETE_POINTER(ele_);
}
