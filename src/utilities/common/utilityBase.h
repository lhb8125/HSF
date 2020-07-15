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
* @file: utilityBase.h
* @author: Hongbin Liu
* @email:
* @Date:   2019-10-06 10:07:59
* @Last Modified by:   Hongbin Liu
* @Last Modified time: 2019-11-14 09:50:18
*/

#ifndef UTILITY_UTILITYBASE_HPP
#define UTILITY_UTILITYBASE_HPP

/**
* define long integer type
*/
typedef long int  label64;
/**
* define integer type
*/
typedef int       label32;
/**
* define short integer type
*/
typedef short int label16;

/**
* define short float type
*/
typedef float scalar32;
/**
* define long float type
*/
typedef double scalar64;

// 默认使用长整型
/**
* define integer type according to compiling macro
*/
#if defined(LABEL_INT32)
    typedef int label;
#else
    typedef long int label;
#endif

// 默认使用double精度
#if defined(SCALAR_FLOAT32)
    typedef float scalar;
#else
    typedef double scalar;
#endif



#endif