#include "utilityInterfaces.h"
#include <stdarg.h>
#include "utilityExceptions.h"
#include "utilityUsingCpp.hpp"
#include "utilityBasicFunction.h"
#include "utilityContainer.hpp"
#include "OStream.hpp"
#include "multiOStream.hpp"
#include "dummyOStream.hpp"
#include "communicator.hpp"
#include "communicationManager.hpp"
#include "mpiWrapper.hpp"

using namespace UTILITY;
using namespace HSF;


/*****************************************utility初始化*****************************************/
void init_utility()
{
  COMM::init(NULL, NULL, "MPI");
  signal(SIGSEGV, handler);
}

/*****************************************进程间数据交换****************************************/
// 获取进程号及全局通信规模
void get_pid( int * pid )
{
  *pid  = COMM::getGlobalId();
}

void get_commsize( int * commSize )
{
  *commSize = COMM::getGlobalSize();
}



// 规约整形变量序列
// para1 [inout] label类型变量
// para2 [in] 变量个数
// para3 [in] 操作类型
void all_reduce_labels(label* data, const label count, unsigned int op)
{
  vector<label> sdata(data, data+count);

  CommData datatype;
  if( sizeof(label)  == 4 ) datatype = COMM_INT32_T;
  else if( sizeof(label)  == 8 ) datatype = COMM_INT64_T;
  else 
  {
    PERR<<" Invalid LABEL type! "<<ENDL;
    exit(-1);
  }
  
  Communicator& gComm = COMM::getGlobalComm();
  gComm.allReduce("all_reduce_labels_", &(sdata[0]), data, count, datatype, 
      op);
  gComm.finishTask("all_reduce_labels_");
}

// 规约浮点变量序列
// para1 [inout] scalar类型变量
// para2 [in] 变量个数
// para3 [in] 操作类型
void all_reduce_scalars(scalar* data, const label count , unsigned int op)
{
  vector<scalar> sdata(data, data+count);

  CommData datatype;
  if( sizeof(scalar)  == 4 ) datatype = COMM_FLOAT;
  else if( sizeof(scalar)  == 8 ) datatype = COMM_DOUBLE;
  else 
  {
    PERR<<" Invalid SCALAR type! "<<ENDL;
    exit(-1);
  }
  
  Communicator& gComm = COMM::getGlobalComm();
  gComm.allReduce("all_reduce_scalars_", &(sdata[0]), data, count, datatype, 
      op);
  gComm.finishTask("all_reduce_scalars_");
}

// 主进程广播整形序列
// para1 [inout] 变量序列
// para2 [in] 变量个数
void bcast_labels(label* data, const label count)
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.bcast("bcast_labels_", data, sizeof(label)*(count) );
  gComm.finishTask("bcast_labels_");
}

// 主进程广播浮点型序列
// para1 [inout] 变量序列
// para2 [in] 变量个数
void bcast_scalars(scalar* data, const label count)
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.bcast("bcast_scalars_", data, sizeof(scalar)*(count) );
  gComm.finishTask("bcast_scalars_");
}

// 主进程收集整形序列
// para1 [in] 发送序列
// para2 [inout] 接收序列
// para3 [in] 变量个数
void gather_labels(label* sdata, label* rdata, const label count )
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.gather("gather_labels_", sdata, sizeof(label)*(count), rdata, 
     sizeof(label)*(count));
  gComm.finishTask("gather_labels_");
}

// 主进程收集浮点序列
// para1 [in] 发送序列
// para2 [inout] 接收序列
// para3 [in] 变量个数
void gather_scalars(scalar* sdata, scalar* rdata, const label count)
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.gather("gather_scalars_", sdata, sizeof(scalar)*(count), rdata, 
     sizeof(scalar)*(count));
  gComm.finishTask("gather_scalars_");
}

void extreme_labels_in_procs(const char* flag, label* data, label* result,
  const label count)
{
  int pid  = COMM::getGlobalId();
  int commSize = COMM::getGlobalSize();
  int num = count*COMM::getGlobalSize();
  label *rdata;
  // printf("%s\n", flag);
  rdata = new label[num];
  // gather_labels_(data, rdata, count);
  MPI_Gather(data, count, COMM_LABEL, rdata, count, COMM_LABEL, 0, MPI_COMM_WORLD);
  if(pid==0)
  {
    // printf("%d\n", *count);
    for (int i = 0; i < count; ++i)
    {
      result[i] = rdata[i*commSize];
      for (int j = 1; j < commSize; ++j)
      {
        // printf("%d,%d,%d,%d\n", i,j,result[i],rdata[i*commSize+j]);
        if(strcmp(flag,"MAX")==0) result[i] = MAX(result[i],rdata[i*commSize+j]);
        else if(strcmp(flag,"MIN")==0) result[i] = MIN(result[i],rdata[i*commSize+j]);
        else
          Terminate("extreme_labels_in_procs_","unknown flag");
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(result, count, COMM_LABEL, 0, MPI_COMM_WORLD);
  // bcast_labels_(result, count);
  DELETE_POINTER(rdata);
}

void extreme_scalars_in_procs(const char* flag, scalar* data, scalar* result,
  const label count)
{
  int pid  = COMM::getGlobalId();
  int commSize = COMM::getGlobalSize();
  int num = count*COMM::getGlobalSize();
  scalar *rdata;
  rdata = new scalar[num];
  // gather_labels_(data, rdata, count);
  MPI_Gather(data, count, COMM_SCALAR, rdata, count, COMM_SCALAR, 0, MPI_COMM_WORLD);
  if(pid==0)
  {
    // printf("%d\n", *count);
    for (int i = 0; i < count; ++i)
    {
      result[i] = rdata[i*commSize];
      for (int j = 1; j < commSize; ++j)
      {
        if(strcmp(flag,"MAX")==0) result[i] = MAX(result[i],rdata[i*commSize+j]);
        else if(strcmp(flag,"MIN")==0) result[i] = MIN(result[i],rdata[i*commSize+j]);
        else
          Terminate("extreme_labels_in_procs_","unknown flag");
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(result, count, COMM_SCALAR, 0, MPI_COMM_WORLD);
  // bcast_labels_(result, count);
  DELETE_POINTER(rdata);
}

/*******************************************标准输出*******************************************/
// #include "stdarg.h"
// 所有进程输出到特定文件
// para1-... [in] 变量名，不定参数个数
void par_std_out(const char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  POUT<<pbuffer<<FLUSH;
}
void par_std_out_(const char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  POUT<<pbuffer<<FLUSH;
}

// 指定进程号输出到屏幕
// para1 [in] 进程编号
// para2-... [in] 变量名，不定参数个数
void proc_std_out(const label pid, const char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  SOUT(pid)<<pbuffer<<FLUSH;
}
void proc_std_out_(const label pid, const char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  SOUT(pid)<<pbuffer<<FLUSH;
}

// 主核输出到屏幕
// para1-... [in] 变量名，不定参数个数
void master_std_out(char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  COUT<<pbuffer<<FLUSH;
}
void master_std_out_(char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  COUT<<pbuffer<<FLUSH;
}

// Fortran 字符串输出
// void fort_cout(const char* str, const int* len)
// {
//   COUT<<Word(str, str+*len)<<ENDL;
// }
// void fort_pout(const char* str, const int* len)
// {
//   POUT<<Word(str, str+*len)<<ENDL;
// }
// void fort_sout(const int* pid, const char* str, const int* len)
// {
//   SOUT(*pid)<<Word(str, str+*len)<<ENDL;
// }

