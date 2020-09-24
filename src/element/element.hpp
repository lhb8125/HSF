#ifndef HSF_ELEMENT_HPP
#define HSF_ELEMENT_HPP

#include <math.h>
#include "utilities.h"
#include "hsfMacro.h"
#include "cgnslib.h"

namespace HSF {

template<typename T>
class Element
{
private:
  label32 num_;
  // label32 len_;
  T* data_;

  bool compareScalar(scalar a, scalar b)
  {
    if(fabs((a)-(b))>1e-10)
    {
        if((a)==0)
        {
            if(fabs((b))>1e-10)
            {
                printf("Error, %.8f, %.8f\n",
                            (a), (b));
                return false;
            }
        }
        else if(fabs(((a)-(b))/(a))>1e-10)
        {
            printf("Error, %.8f, %.8f\n",
                        (a), (b));
            return false;
        }
    }
    return true;
  }
public:
  /**
  * @brief default constructor
  */
  Element(){};

  /**
  * @brief construct a element with num random numbers
  */
  Element(label32 num)
  {
    this->num_ = num;
    this->data_ = new T[num];
    for (int i = 0; i < num; ++i)
    {
        this->data_[i] = (rand()%100);
    }
  }

  /**
  * @brief construct a element with a value
  */
  Element(T val, label32 num)
  {
    this->num_ = num;
    this->data_ = new T[num];
    for (int i = 0; i < num; ++i)
    {
        this->data_[i] = val;
    }
  }

  /**
  * @brief copy constructor
  */
  Element(const Element<T>& ele)
  {
    this->num_ = ele.num;
    this->data_ = new T[this->num_];
    for (int i = 0; i < this->num_; ++i)
    {
        this->data_[i] = ele.data_[i];
    }
  }

  /**
  * @brief constuct a element from an array
  */
  Element(T* data, label32 num)
  {
    this->num_ = num;
    this->data_ = new T[num];
    for (int i = 0; i < num; ++i)
    {
        this->data_[i] = data[i];
    }
  };

  /**
  * @brief default deconstructor
  */
  ~Element(){ DELETE_POINTER(data_);};

  /**
  * @brief return the total memory size of element
  */
  label32 length() { return sizeof(T); }

  /**
  * @brief return the data type T
  */
  DataType_t type() {
    if(typeid(T) == typeid(label))
    {
        if(sizeof(label)==8) return LongInteger;
        else return Integer;
    }
    else if(typeid(T) == typeid(scalar))
    {
        if(sizeof(scalar)==8) return RealDouble;
        else return RealSingle;
    }
  }

  /**
  * @brief change the pointer
  */
  void setData(void* data) { this->data_ = (T*)data;}

  /**
  * @brief get the data pointer
  */
  T* getData() { return this->data_;}

  /**
  * @brief set the number
  */
  void setNum(label32 num) { this->num_ = num; }
  /**
  * @brief get the number
  */
  label32 getNum() { return this->num_; }

  /**
  * @brief set the value of data
  */
  void setVal(T val)
  {
    for (int i = 0; i < this->num_; ++i)
    {
        this->data_[i] = val;
    }
  }

  /**
  * @brief print the element to screen
  */
  void show()
  {
    printf("n: %d--", this->num_);
    for (int i = 0; i < this->num_; ++i)
    {
        if(typeid(T)==typeid(label))
            printf("%d,", this->data_[i]);
        else if(typeid(T)==typeid(scalar))
            printf("%f, ", this->data_[i]);
        else 
            Terminate("setVal", "the data type is not supported");
    }
    printf("\n");
  }

  /**
  * @brief overload operator []
  */
  T& operator[](const label32 i) 
  {
    if(i<this->num_)
        return this->data_[i];
    else
        Terminate("element overload","the index exceed the length of element");
  }

  /**
  * @brief overload operator ==
  */
  bool operator==(const Element<T>& ele)
  {
    if(this->num_!=ele.num_) return false;
    for (int i = 0; i < this->num_; ++i)
    {
        if(typeid(T)==typeid(label))
            if(this->data_[i]!=ele.data_[i]) return false;
        else if(typeid(T)==typeid(scalar))
            if(!compareScalar(this->data_[i], ele.data_[i])) return false;
        else 
            Terminate("setVal", "the data type is not supported");
    }
    return true;
  }

  /**
  * @brief overload operator !=
  */
  bool operator!=(const Element<T>& ele)
  {
    if(this->num_!=ele.num_) return true;
    for (int i = 0; i < this->num_; ++i)
    {
        if(typeid(T)==typeid(label)){
            if(this->data_[i]!=ele.data_[i]) return true;
        }
        else if(typeid(T)==typeid(scalar)){
            if(!compareScalar(this->data_[i], ele.data_[i])) return true;
        }
        else 
            Terminate("setVal", "the data type is not supported");
    }
    return false;
  }
};
}

#endif
