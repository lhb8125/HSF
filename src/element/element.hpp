#ifndef HSF_ELEMENT_HPP
#define HSF_ELEMENT_HPP

#include "utilities.h"

namespace HSF {

template<typename T>
class Element
{
private:
  label32 num_;
  // label32 len_;
  T* data_;
public:
  Element(){};
  Element(label32 num)
  {
    this->num_ = num;
    this->data_ = new T[num];
    for (int i = 0; i < num; ++i)
    {
        this->data_[i] = (rand()%100);
    }
  }

  Element(T* data, label32 num)
  {
    this->num_ = num;
    this->data_ = new T[num];
    for (int i = 0; i < num; ++i)
    {
        this->data_[i] = data[i];
    }
  };
  ~Element(){};

  label32 length() { return this->num_*sizeof(T); }

  void setData(void* data) { this->data_ = (T*)data;}

  T* getData() { return this->data_;}

  void setNum(label32 num) { this->num_ = num; }
  label32 getNum() { return this->num_; }

  void setVal(T val)
  {
    for (int i = 0; i < this->num_; ++i)
    {
        this->data_[i] = val;
    }
  }

  void show()
  {
    printf("n: %d--", this->num_);
    for (int i = 0; i < this->num_; ++i)
    {
        printf("%d,", this->data_[i]);
    }
    printf("\n");
  }
  
};
}

#endif