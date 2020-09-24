#ifndef HSF_SCALAR_HPP
#define HSF_SCALAR_HPP

#include "element.hpp"

namespace HSF {

template<typename T>
class ScalarEle : public Element<T>
{
private:

public:
  ScalarEle() : Element<T>(1)
  {};

  ScalarEle(T* data) : Element<T>(data,1)
  {};

  ScalarEle(T val) : Element<T>(val,1)
  {};

  ~ScalarEle(){};
};


template<typename T>
class Vector3DEle : public Element<T>
{
private:

public:
  Vector3DEle() : Element<T>(3)
  {};

  Vector3DEle(T* data) : Element<T>(data,3)
  {};
  ~Vector3DEle(){};
};

} // end namespace HSF

#endif

