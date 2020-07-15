#ifndef KERNEL_SLAVE_H
#define KERNEL_SLAVE_H

typedef struct{
    int a;
}DataSet;

typedef struct
{
    int a;
    double b;
    long c;
}S;

typedef int label;
typedef double scalar;

scalar* getArrayFromField(char* fieldName);
label* getDimFromField(char* fieldName);
label  getSizeFromField(DataSet *dataSet_edge, DataSet *dataSet_vertex);
void* getConstFromDataSet(label i);


#endif
