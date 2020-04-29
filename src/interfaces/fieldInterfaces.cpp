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
 * @File: fieldInterfaces.cpp
 * @Author: Hanfeng
 * @Email:
 * @Date:   2019-11-9 10:52:48
 * @Last Modified by:   Hanfeng
 * @Last Modified time: 2019-11-30 15:22:43
 */

/*
 * @brief:
 */

#include "fieldInterfaces.hpp"

#include "interface.hpp"

using namespace HSF;

#define REGION regs[0]

struct struct_labelField HSF::get_label_field_(const char *setType,
                                               const char *fieldName)
{
  struct struct_labelField tmp;

  Field<label> &fieldI = REGION.getField<label>(setType, fieldName);
  tmp.data = fieldI.getLocalData();
  tmp.ndim = fieldI.getDim();
  tmp.locSize = fieldI.getSize();
  tmp.nbrSize = fieldI.getNbrSize();

  return tmp;
}

struct struct_scalarField HSF::get_scalar_field_(const char *setType,
                                                 const char *fieldName)
{
  struct struct_scalarField tmp;

  Field<scalar> &fieldI = REGION.getField<scalar>(setType, fieldName);
  tmp.data = fieldI.getLocalData();
  tmp.ndim = fieldI.getDim();
  tmp.locSize = fieldI.getSize();
  tmp.nbrSize = fieldI.getNbrSize();

  return tmp;
}

void HSF::add_label_field_(const char *setType,
                           const char *fieldName,
                           label *fPtr,
                           label *ndim)
{
  label size = REGION.getMesh().getTopology().getSize(setType);
  Table<Word, Table<Word, Patch *> *> &patchTab = REGION.getPatchTab();
  Field<label> *fnew = new Field<label>(setType, *ndim, size, fPtr, patchTab);
  REGION.addField<label>(fieldName, fnew);
}

void HSF::add_scalar_field_(const char *setType,
                            const char *fieldName,
                            scalar *fPtr,
                            label *ndim)
{
  label size = REGION.getMesh().getTopology().getSize(setType);
  Table<Word, Table<Word, Patch *> *> &patchTab = REGION.getPatchTab();
  Field<scalar> *fnew = new Field<scalar>(setType, *ndim, size, fPtr, patchTab);
  REGION.addField<scalar>(fieldName, fnew);
}

void HSF::start_exchange_label_field_(const char *setType,
                                      const char *fieldName)
{
  Field<label> &fieldI = REGION.getField<label>(setType, fieldName);
  fieldI.initSend();
}

void HSF::finish_exchange_label_field_(const char *setType,
                                       const char *fieldName)
{
  Field<label> &fieldI = REGION.getField<label>(setType, fieldName);
  fieldI.checkSendStatus();
}

void HSF::start_exchange_scalar_field_(const char *setType,
                                       const char *fieldName)
{
  Field<scalar> &fieldI = REGION.getField<scalar>(setType, fieldName);
  fieldI.initSend();
}

void HSF::finish_exchange_scalar_field_(const char *setType,
                                        const char *fieldName)
{
  Field<scalar> &fieldI = REGION.getField<scalar>(setType, fieldName);
  fieldI.checkSendStatus();
}
