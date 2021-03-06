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
 * @file basicFunction.h
 * @brief basic functionalities that would be used universally
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-09
 */

#include "stdint.h"
#include "stdlib.h"

#ifdef __cplusplus
#include <cstring>
#include "utilityUsingCpp.hpp"
#else
#include <string.h>
#endif

#ifndef UTILITY_BASICFUNCTION_H
#define UTILITY_BASICFUNCTION_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief delete pointer
*/
#ifdef __cplusplus

#define DELETE_POINTER(ptr) if(ptr) \
{ \
    delete [] (ptr); \
    ptr = NULL; \
}

#else
#define DELETE_POINTER(ptr) if(ptr) \
{ \
    delete (ptr); \
    ptr = NULL; \
}
#endif

#define DELETE_OBJECT_POINTER(ptr) if(ptr) \
{ \
    delete ptr; \
    ptr = NULL; \
}

// #define SOL_DIM 3

#define CHAR_DIM 200

#define forAll(i, length) for (label i = 0; i < length; ++i)

//--------------------------------------------------------------
// algorithm macros
//--------------------------------------------------------------

#define MAX(a, b) ( ( (a) > (b) ) ? (a) : (b) )
#define MIN(a, b) ( ( (a) < (b) ) ? (a) : (b) )


//--------------------------------------------------------------
// Hash conversion
//--------------------------------------------------------------

typedef uint64_t Hash;
typedef uint32_t Hash32;

/**
* @brief A simple hash function from Robert Sedgwicks Algorithms in C book. I've added
         some simple optimizations to the algorithm in order to speed up its hashing 
         process.
         Reference "http://www.partow.net/programming/hashfunctions"
* @param[in] str input string
* @return Hash value of string.
*/
inline Hash str2Hash( const char* str)
{
  Hash b    = 378551;
  Hash a    = 63689;
  Hash hash = 0;
  Hash i    = 0;
  
  size_t length = strlen((char*) str);

  for (i = 0; i < length; ++str, ++i)
  {
    hash = hash * a + (*str);
    a    = a * b;
  }

  return hash;
}

/**
* @brief A simple hash function from Robert Sedgwicks Algorithms in C book. I've added
         some simple optimizations to the algorithm in order to speed up its hashing 
         process.
         Reference "http://www.partow.net/programming/hashfunctions"
* @param[in] str input string
* @return Hash value of string.
*/
inline Hash32 str2Hash32( const char * str)
{
  Hash32 b    = 378551;
  Hash32 a    = 63689;
  Hash32 hash = 0;
  Hash32 i    = 0;

  size_t length = strlen((char*) str);

  for (i = 0; i < length; ++str, ++i)
  {
    hash = hash * a + (*str);
    a    = a * b;
  }

  return hash;
}


//--------------------------------------------------------------
// file and directory operations
//--------------------------------------------------------------
#include "sys/types.h" 
#include "sys/stat.h"
#include "unistd.h"
#include "dirent.h"

/**
 * @brief Check file status
 * @param[in] _Filename file name
 * @param[in] _AccessMode access mode
          In file "unistd.h", there are
          #define R_OK 4, readable or not
          #define W_OK 2, writable or not
          #define X_OK 1, excutable or not   
          #define F_OK 0, exit or not
  * @return error code
 */
int access(const char* _Filename, int _AccessMode);

/*
 * Creat directory
 * in file "sys/types.h"
 * #define  S_IRWXU 0000700, RWX mask for owner
 */
int mkdir(const char* _path, mode_t mode);

/*
 * Delete directory
 * in file "dirent.h"
 */
int rmdir(const char *_Path);

/*
 * Rename directory
 * in file "stdlib.h"
 */
int rename ( const char * oldname, const char * newname ) __THROW;

/**
 * @brief Remove a directory recursively
 * @param[in] path absolute directory path
 */
void rmtree(const char path[]);

/**
 * @brief create a new directory and move the older one to xxx.old
 * @param[in] path absolute directory path
 * @return error code
 */
int remakeDir(const char* path);

#ifdef __cplusplus
}
#endif

// c++ overload hash
#ifdef __cplusplus

namespace UTILITY
{

  // convert string to Hash (int64_t)
  inline Hash strToHash(const string& str)
  {
    return str2Hash(str.c_str() );
  }
  
  // convert string to Hash32 (int32_t)
  inline Hash32 strToHash32(const string& str)
  {
    return str2Hash32(str.c_str() );
  }
  
  // remake an directory and move the older one to xx.old
  inline int remakeDirectory(const string& name)
  {
    return remakeDir(name.c_str());
  }

} // namespace UTILITY

#endif

#endif // HSF_BASICFUNCTION_H




