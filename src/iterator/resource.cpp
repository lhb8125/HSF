#include "funPtr_slave.hpp"

void prepareField(label32* inoutList, char* funcName, int nPara, ...)
{
    void* paraList[nPara];

    va_list args;
    va_start(args, &nPara);

    for (int i = 0; i < nPara; ++i)
    {
        paraList[i] = va_arg(args, void*);
    }

    va_end(args);

    // DataSet dataSet_vertex, dataSet_edge;
    for (int i = 0; i < nPara; ++i)
    {
        Field<scalar>* field = (Field<scalar>*)paraList[i];
        scalar* var          = field->getLocalData();
        Word setType         = field->getType();
        label32 dim          = field->getDim();
        label64 size         = field->getSize();
        // if(setType=="cell")
        //     addSingleArray(dataSet_vertex, dim, size, inoutList[i], var);
        // else if(setType=="face")
        //     addSingleArray(dataSet_edge, dim, size, inoutList[i], var);
        // else
        //     std::cout<<"error!!!"<<endl;
    }
    // // 根据数据集类型选择拓扑
    // ArrayArray<label>& topo = chooseTopology(setTypeList);
    // // 创建UNAT拓扑
    // Topology* UNATTopo = constructTopology(topo, LDU);
    // // 生成UNAT迭代器
    // UNAT::RSSIterator rssIter(UNATTopo, LDU);
    // // 封装耦合算子
    // CoupledOperator opt;
    // opt.data.edgeData = &dataSet_edge;
    // opt.data.vertexData = &dataSet_vertex;
    // opt.fun_slave = funcName;
    // // 调用UNAT迭代器计算，UNAT函数指针需要进行改造
    // rssIter.iteration(NULL, &opt, 1);

}