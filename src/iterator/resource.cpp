#include "resource.hpp"

template<typename T>
UNAT::ArrayArray<T>& transformArrayFromHSFToUNAT(const HSF::ArrayArray<T> &src)
{
    UNAT::ArrayArray<T> *unat_arr = new UNAT::ArrayArray<T>();
    unat_arr->num = src.num;
    unat_arr->startIdx = new swInt[src.num+1];
    for (int i = 0; i < src.num+1; ++i)
    {
        unat_arr->startIdx[i] = src.startIdx[i];
    }
    unat_arr->data = new T[unat_arr->startIdx[unat_arr->num]];
    for (int i = 0; i < src.num; ++i)
    {
        for (int j = unat_arr->startIdx[i]; j < unat_arr->startIdx[i+1]; ++j)
        {
            unat_arr->data[j] = src.data[j];
        }
    }
    return *unat_arr;
}

void prepareConst(int nPara, ...)
{
    void*   paraList[nPara/2];
    label32 lengthList[nPara/2];

    va_list args;
    va_start(args, &nPara);

    for (int i = 0; i < nPara/2; ++i)
    {
        paraList[i]   = va_arg(args, void*);
        lengthList[i] = va_arg(args, label32);
    }

    va_end(args);

    for (int i = 0; i < nPara/2; ++i)
    {
        addSingleArray(paraData, 1, lengthList[i], 1, (swFloat*)paraList[i]);
    }
}

void prepareField(Region& reg, label32* inoutList, e2v_slaveFunPtr funcPtr, int nPara, ...)
{
    void* paraList[nPara];

    va_list args;
    va_start(args, &nPara);

    for (int i = 0; i < nPara; ++i)
    {
        paraList[i] = va_arg(args, void*);
    }

    va_end(args);

    Arrays dataSet_vertex, dataSet_edge;
    Array<Word> setTypeList;
    label32 vertexWeight=0, edgeWeight=0;
    for (int i = 0; i < nPara; ++i)
    {
        Field<scalar>* field = (Field<scalar>*)paraList[i];
        scalar* var          = field->getLocalData();
        Word setType         = field->getType();
        label32 dim          = field->getDim();
        label64 size         = field->getSize();
        setTypeList.push_back(setType);
        if(setType=="cell")
        {
            vertexWeight++;
          addSingleArray(dataSet_vertex, dim, size, inoutList[i], var);
        }
        else if(setType=="face")
        {
          edgeWeight++;
          addSingleArray(dataSet_edge, dim, size, inoutList[i], var);
        }
        else
        {
          std::cout<<"error!!!"<<endl;
        }
    }
    // 根据数据集类型选择拓扑
    HSF::ArrayArray<label>& topo = reg.getTopology<label>(setTypeList);
    UNAT::ArrayArray<label> UNATTopo = transformArrayFromHSFToUNAT(topo);
    label n = topo.size();
    // 创建UNAT拓扑
    UNAT::Topology* realTopo = UNAT::Topology::constructTopology(UNATTopo, UNAT::LDU);
    // 封装耦合算子
    coupledOperator opt;
    opt.data->edgeData = &dataSet_edge;
    opt.data->vertexData = &dataSet_vertex;
    opt.fun_slave = funcPtr;
    // 生成UNAT迭代器
    UNAT::RowSubsectionIterator rssIter(&opt, *realTopo, &vertexWeight, &edgeWeight, 1, true);
    // 调用UNAT迭代器计算，UNAT函数指针需要进行改造
    rssIter.edge2VertexIteration(&paraData, &opt, 1);

}