#ifndef FUNPTR_SLAVE_H
#define FUNPTR_SLAVE_H

typedef struct{
    int a;
}DataSet;

typedef int label;
typedef double scalar;

scalar* getArrayFromField(char* fieldName);
label* getDimFromField(char* fieldName);
label  getSizeFromField(DataSet *dataSet_edge, DataSet *dataSet_vertex);

#endif