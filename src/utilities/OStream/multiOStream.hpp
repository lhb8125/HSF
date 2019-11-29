/* Copyright (C) 
 * 2019 - Hu Ren, rh890127a@163.com
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */
/**
 * @file MultiOStream.hpp
 * @brief class MultiOStream will linked with multiple files, and output
 * contents to those files simutaneously.
 *
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-13
 */

#include "OStream.hpp"
#include <stdlib.h>

#ifndef HSF_MULTIOSTREAM_HPP
#define HSF_MULTIOSTREAM_HPP

namespace HSF
{

class MultiOStream : 
  public OStream
{
  // core file stream
  vector<ostream*> files_;
  // if it is redirected
  vector<bool> redirected_;
  // original stream buffer holder in case redirected 
  vector<StrBuf*> buffers_;

public:
//--------------------------------------------------------------
// construct & deconstruct
//--------------------------------------------------------------

  // construct empty
  MultiOStream()
    :
    files_(0),redirected_(0), buffers_(0)
  {}

  // construct from file name
  MultiOStream(const string* filename, int num = 1)
    :
    files_(num, NULL), redirected_(num, false), buffers_(num, NULL)
  {
    for(int i = 0; i < num; i++)
      files_[i] = new ofstream(filename[i].c_str());
  }

  // construct from file buffer
  MultiOStream(StrBuf** rbuf, int num = 1)
    :
    files_(num, NULL), redirected_(num, true), buffers_(num, NULL)
  {
    for(int i = 0; i < num ; i++)
    {
      files_[i] = new ofstream();
      buffers_[i] = files_[i]->rdbuf();
      files_[i]->ostream::rdbuf(rbuf[i]);
    }
  }

// No clone or operator= (before C++11) for ostream 
/* 
  // copy constructor
  MultiOStream( const MultiOStream& ref )
    : 
      OStream(ref), files_(0), redirected_(0), buffers_(0)
  {
    // copy data
    size_t size = ref.getFileNum();
    for(size_t i = 0; i < size; i++)
	{
      this->files_.push_back(ref.getRawStream(i)->clone() );
      this->redirected_.push_back(ref.redirected(i) );
      this->buffers_.push_back(ref.getStrBuf(i)->clone() );
    }
  }
  
  // clone
  virtual MultiOStream* clone() { return new MultiOStream(*this) }; 

  // assign operator
  void operator = ( const MultiOStream& ref )
  { 
    // deconstruct the older first to close file
    for(int i = 0; i < files_.size(); i++)
    {
      if( redirected_[i] ) files_[i]->ostream::rdbuf(buffers_[i]);
      if( files_[i]->rdbuf() != cout.rdbuf() && files_[i]->rdbuf() != NULL )
        ((ofstream*) files_[i] )->close();
      delete files_[i];
    }
    // clear containers
    files_.resize(0);
    redirected_.resize(0);
    buffers_.resize(0);
    // copy data
    size_t size = ref.getFileNum();
    for(size_t i = 0; i < size; i++)
	{
      this->files_.push_back(ref.getRawStream(i)->clone() );
      this->redirected_.push_back(ref.redirected(i) );
      this->buffers_.push_back(ref.getStrBuf(i)->clone() );
    }
  }
*/


  // deconstruct
  virtual ~MultiOStream()
  {
    for(int i = 0; i < files_.size(); i++)
    {
      if( redirected_[i] ) files_[i]->ostream::rdbuf(buffers_[i]);
      if( files_[i]->rdbuf() != cout.rdbuf() && files_[i]->rdbuf() != NULL )
        ((ofstream*) files_[i] )->close();
      delete files_[i];
    }
  }

//--------------------------------------------------------------
// redirecting & access
//--------------------------------------------------------------
  virtual int redirect(StrBuf* rbuf, int pos = 0)
  {
    if(pos >= files_.size())
    {
      cerr<<__FILE__<<" + "<<__LINE__<<": "<<endl
        <<__FUNCTION__<<": "<<endl
        <<"Error: manipulation on an undefined object!"<<endl;
      exit( -1 );
    }

    if(redirected_[pos]) files_[pos]->ostream::rdbuf(rbuf);
    else 
    {
      redirected_[pos] = true;
      buffers_[pos] = files_[pos]->rdbuf();
      files_[pos]->ostream::rdbuf(rbuf);
    }
  }

  virtual int reset(int pos = 0)
  {
    if(pos >= files_.size())
    {
      cerr<<__FILE__<<" + "<<__LINE__<<": "<<endl
        <<__FUNCTION__<<": "<<endl
        <<"Error: manipulation on an undefined object!"<<endl;
      exit( -1 );
    }
    
    if(redirected_[pos]) 
    {
      files_[pos]->ostream::rdbuf(buffers_[pos]);
      redirected_[pos] = false;
    }
  }

  virtual const ostream* getRawStream(int pos = 0) { return files_[pos]; }
  
  virtual StrBuf* getStrBuf(int pos = 0) { return files_[pos]->rdbuf(); }
  
  virtual bool redirected(int pos = 0) { return redirected_[pos]; }
 
  virtual size_t getFileNum() { return files_.size(); } 

  // add new file
  virtual int addFile(const string& filename)
  {
    files_.push_back(NULL);
    *(files_.end() - 1 )= new ofstream(filename.c_str());
    redirected_.push_back(false);
    buffers_.push_back(NULL);
  }

  // add new buffer
  virtual int addBuffer(StrBuf* buf)
  {
    files_.push_back(NULL);
    files_[files_.size() - 1 ] = new ofstream();
    redirected_.push_back(true);
    buffers_.push_back(files_[files_.size() - 1 ]->rdbuf() );
    files_[files_.size() - 1 ]->ostream::rdbuf(buf);
  }

  // erase last file
  virtual int closeLast()
  {
    if(files_.size() > 0 ) 
    {
      int pos = files_.size() - 1;
      if( redirected_[pos] ) files_[pos]->ostream::rdbuf(buffers_[pos]);
      if( files_[pos]->rdbuf() != cout.rdbuf() && 
          files_[pos]->rdbuf() != NULL )
        ((ofstream*) files_[pos] )->close();
      delete files_[pos];
      files_.pop_back();
      buffers_.pop_back();
      redirected_.pop_back();
    }
    else return 0;
  }

//--------------------------------------------------------------
// streaming operator
//--------------------------------------------------------------
  virtual MultiOStream & operator<<(char chrt)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<chrt;
    return (MultiOStream &) *this;
  }

  virtual MultiOStream & operator<<(string str)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<str;
    return (MultiOStream &) *this;
  }

  virtual MultiOStream & operator<<(int64_t val)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<val;
    return (MultiOStream &) *this;
  }

  virtual MultiOStream & operator<<(int32_t val)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<val;
    return (MultiOStream &) *this;
  }

  virtual MultiOStream & operator<<(unsigned long val)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<val;
    return (MultiOStream &) *this;
  }

  virtual MultiOStream & operator<<(unsigned int val)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<val;
    return (MultiOStream &) *this;
  }

  virtual MultiOStream & operator<<(double val)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<val;
    return (MultiOStream &) *this;
  }
  
  virtual MultiOStream & operator<<(float val)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<val;
    return (MultiOStream &) *this;
  }

  /**
   * @brief operator<<, interface accept OsOp type parameters 
   * @param[in] opt, represent parameter like "ENDL" and "FLUSH".
   * @return 
   */
  virtual MultiOStream & operator<<(OsOp opt)
  {
    for(int i = 0; i < files_.size(); i++ )
      *(this->files_[i])<<opt;
    return (MultiOStream &) *this;
  }

};



}// namespace HSF



#endif // HSF_MULTIOSTREAM_HPP
