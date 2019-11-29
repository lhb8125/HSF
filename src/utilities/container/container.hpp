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
 * @file container.hpp
 * @brief, basic container warrper and useful array manipulations
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-16
 */

#include "usingCpp.hpp"

#ifndef HSF_CONTAINER_HPP
#define HSF_CONTAINER_HPP

namespace HSF
{
// wrap container to HSF
#define Word string
#define Array vector
#define List set
#define Table map
#define MultiList multiset
#define MultiTable multimap

// conversion for Word
#define toWord to_string
#define w2f stof
#define w2d stod
#define w2ld stold
#define w2i stoi
#define w2l stol
#define w2ll stoll

/**
 * @brief sort, Sort an single array that contains data can be compared
 * @tparam D data type that can be compared with "<"
 * @param[inout] data the data array
 * @return 
 */
template<class D>
int sort(Array<D>& data )
{
  MultiList<D> table;

  size_t size = data.size();
  for(size_t i = 0; i < size; i++) table.insert(data[i]);

  size_t count = 0;
  for(typename MultiList<D>::iterator iter = table.begin(); iter != table.end(); iter++)
  {
    data[count] = *iter;
    count++;
  }

  if( count != size )
  {
    cerr<<__FILE__<<" +"<<__LINE__<<":"<<endl<<__FUNCTION__<<":"<<endl;
    cerr<<"Error: Sorted data is not consistent with original size!"<<endl;
    exit(-1);
  }
}

/**
 * @brief sort, Sort an single array that contains data can be compared with
 * corresponding weights.
 * @tparam D data type
 * @tparam W weight type that can be compared with "<"
 * @param[inout] data the data array
 * @param[in] weights the weight array
 * @return 
 */
template<class D, class W>
int sort(Array<D>& data, const Array<W>& weights = Array<W>(0) )
{
  MultiTable<W,D> table;

  size_t size = data.size();
  for(size_t i = 0; i < size; i++) table.insert( 
      pair<W,D>(weights[i], data[i])
      );

  size_t count = 0;
  for(typename MultiTable<W,D>::iterator iter = table.begin(); 
      iter != table.end(); iter++)
  {
    data[count] = iter->second;
    count++;
  }

  if( count != size )
  {
    cerr<<__FILE__<<" +"<<__LINE__<<":"<<endl<<__FUNCTION__<<":"<<endl;
    cerr<<"Error: Sorted data is not consistent with original size!"<<endl;
    exit(-1);
  }
}

template<class W>
int ascend(const void* a, const void* b)
{
  cout<<"comparing "<<* (W*) a <<" and "<<* (W*) b<<endl;
  if( * (W*) a < * (W*) b ) return -1;
  else if( * (W*) b < * (W*) a ) return 1;
  else return 0;
}
template<class W>
int descend(const void* a, const void* b)
{
  cout<<"comparing "<<* (W*) a <<" and "<<* (W*) b<<endl;
  if( * (W*) a > * (W*) b ) return -1;
  else if( * (W*) b < * (W*) a ) return 1;
  else return 0;
}
typedef int ComparePtr(const void* , const void* );
/**
 * @brief biSearch, search the array with binary searching
 * @tparam W the data type that can be compared with "<"
 * @param[in] data data array sorted in ascending order
 * @param[in] value the target value to search for
 * @return the position target value located; return data.size() if not found.
 */
template<class W>
size_t biSearch( const Array<W>& data , W value, ComparePtr comp = ascend<W>)
{
  //typename Array<W>::iterator iter;
  //iter = std::lower_bound(data.begin(), data.end(), value);
  //return (size_t) (iter - data.begin() );
  size_t num = data.size();
  size_t size = sizeof(W);
  W* posi = (W*) bsearch(&value, &(data[0]), num, size, comp);
  //cout<<"find value: "<<*posi<<endl;
  return (W*)posi - (W*)&(data[0]);
}


} // namespace HSF

#endif //HSF_CONTAINER_HPP
