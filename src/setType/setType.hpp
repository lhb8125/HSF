#ifndef HSF_SETTYPE_HPP
#define HSF_SETTYPE_HPP

#include "utilities.h"

class SetType
{
public:
    SetType();
    ~SetType();
    
};

class Cell : public SetType
{
public:
    Cell();
    ~Cell();
    
};

class Face : public SetType
{
public:
    Face();
    ~Face();
    
};

#endif
