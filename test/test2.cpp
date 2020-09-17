#include <iostream>
#include <fstream>
#include <string>
#include "utilities.h"
#include <yaml-cpp/yaml.h>
#include <assert.h>
#include <unistd.h>
#include "cstdlib"
#include <mpi.h>
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "cgnslib.h"
#include <iostream>

// 测试目的:  1. 测试功能 utilitis 中创建 多个communicator的功能
//           2. 测试HSF中多个类添加communicator对象的功能


using namespace HSF;

int main(int argc, char **argv)
{
    Parameter para("./config.yaml"); // 获得配置参数
    ControlPara newPara("./config.yaml");

    COMM::init(NULL,NULL,"MPI"); // 初始化MPI环境 (使用全局的通信域)
    Communicator &global_comm = COMM::getGlobalComm();
    Communicator comm, comm1, comm2;
    int rank = global_comm.getMyId();
    int size = global_comm.getMySize();

    if (rank == 0)
    {
        printf("comm1 is NULL %d,comm2 is NULL %d\n", comm1.isCommunicatorNull(), comm2.isCommunicatorNull());
    }
    printf("global rank %d , global size %d\n", rank, size);

    global_comm.barrier();

    int color = 0;
    if (rank % 2 == 0)
        color = 1;

    // global_comm.barrier();
    // 开始创建通信域
    COMM::getCommManager().split(global_comm, comm, color, 2);

    // 对本地进程来说复制相应的通信域

    int comm_rank = comm.getMyId();
    int comm_size = comm.getMySize();

    // printf("comm  rank: %d ,comm size: %d . global rank %d , global size %d  , and color %d \n", comm_rank, comm_size, rank, size, color);

    global_comm.barrier();
    if (color == 1)
    {
        COMM::getCommManager().duplicate(comm, comm1);
    }
    else
    {
        COMM::getCommManager().duplicate(comm, comm2);
    }

    if (rank == 0)
    {
        printf("***comm1 is NULL %d,comm2 is NULL %d\n", comm1.isCommunicatorNull(), comm2.isCommunicatorNull());
    }
    else if (rank == 1)
    {
        printf("***comm1 is NULL %d,comm2 is NULL %d\n", comm1.isCommunicatorNull(), comm2.isCommunicatorNull());
    }
    global_comm.barrier();
    // 声明两个不同的通讯域 分别读入region对象


    Array<Word> mesh_files; 
    Word meshFile,resultFile;
    newPara["domain"]["region"]["0"]["path"].read(meshFile);
    newPara["domain"]["region"]["0"]["resPath"].read(resultFile);
    mesh_files.push_back(meshFile);

    Array<Region> regs;
    Region reg(global_comm);
    regs.push_back(reg);

    LoadBalancer lb(global_comm);

    printf("init Before Balance\n");
    regs[0].initBeforeBalance(mesh_files);
    global_comm.barrier();

    printf("init before balance done! \nbegin load balance ...\n");

    lb.LoadBalancer_3(regs);
    global_comm.barrier();

    printf("Load Balance done \n");
    printf("after balance ...\n");
    regs[0].initAfterBalance();
    global_comm.barrier();
    printf("after balance done, sizeof(label)%d \n",sizeof(label));

    
   

    // Word resultFile; // 输出网格的文件名称
    // newPara["domain"]["region"]["0"]["resPath"].read(resultFile);

    // Word meshFile1;

    // if (rank == 0)
    //     printf("%s,name:%s\n", meshFile.c_str(), meshFile1.c_str());

    // LoadBalancer lb;
    // LoadBalancer lb1;
    // Array<Region> regs;
    // Array<Region> regs1;

    // global_comm.barrier();

    // double t1, t2;

    // Array<std::string> meshfiles;
    // Region reg(global_comm);
    // Region reg1(global_comm);
    // regs.push_back(reg);
    // regs1.push_back(reg1);
    // meshfiles.push_back(meshFile);
    // t1 = MPI_Wtime();
    // printf("使用regs1之前的程序进行计算\n");
    // regs1[0].initBeforeBalance(meshfiles);
    // t2 = MPI_Wtime();
    // printf("网格读入成功\n开始负载均衡,时间花费: %f\n", t2 - t1);

    // t1 = MPI_Wtime();
    // lb.LoadBalancer_3(regs1);
    // t2 = MPI_Wtime();
    // if (global_comm.getMyId() == 0)
    //     printf("parmetis time :%f\n", t2 - t1);
    // printf("负载均衡完成\n开始形成对应的拓扑关系\n");
    // global_comm.barrier();
    // t1 = MPI_Wtime();
    // // regs1[0].initAfterBalance();
    // t2 = MPI_Wtime();
    // printf("完成生成对应的拓扑关系: time %f .\n", t2 - t1);

    // printf("开始读入网格信息\n");
    // t1 = MPI_Wtime();
    // regs[0].initBeforeBalance_1(meshfiles);
    // t2 = MPI_Wtime();
    // printf("读入网格数据成功,时间花费%f . \n", t2 - t1);
    // t1 = MPI_Wtime();
    // // 开始进行负载均衡
    // lb.LoadBalancer_3_3(regs);
    // t2 = MPI_Wtime();
    // if (global_comm.getMyId() == 0)
    //     printf("parmetis time :%f\n", t2 - t1);

    // t1 = MPI_Wtime();
    // regs[0].initAfterBalance_1();
    // t2 = MPI_Wtime();
    // printf("完成生成的拓扑关系: time: %f .\n", t2 - t1);

}