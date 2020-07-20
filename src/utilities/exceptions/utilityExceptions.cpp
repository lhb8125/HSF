#include "utilityExceptions.h"
#include <dlfcn.h> 
#include <cxxabi.h>
#include <sstream>
#include "utilityOStream.hpp"
#include "utilityMpiWrapper.hpp"


void handler(int sig) {
  void *array[10];
  size_t size;
  char **strings;
  size_t i;
  char buf[1024];
  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  // fprintf(stderr, "Error: signal %d:\n", sig);
  if(sig!=0)
    UTILITY::PERR<<"Error: signal "<<sig<<":\n"<<FLUSH;
  strings = backtrace_symbols (array, size);

  // printf ("Obtained %zd stack frames.\n", size);
  UTILITY::PERR<<"Obtained "<<size<<"stack frames.\n"<<FLUSH;

  std::ostringstream trace_buf;
  for (i = 0; i < size; i++)
  {
    // HSF::PERR<<strings[i]<<"\n"<<FLUSH;
    Dl_info info;
    if (dladdr(array[i], &info) && info.dli_sname) {
        char *demangled = NULL;
        int status = -1;
        if (info.dli_sname[0] == '_')
            demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
        snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
                 i, int(2 + sizeof(void*) * 2), array[i],
                 status == 0 ? demangled :
                 info.dli_sname == 0 ? strings[i] : info.dli_sname,
                 (char *)array[i] - (char *)info.dli_saddr);
        free(demangled);
    } else {
        snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
                 i, int(2 + sizeof(void*) * 2), array[i], strings[i]);
    }
    trace_buf << buf;
  }
  UTILITY::PERR<<trace_buf.str()<<"\n"<<FLUSH;
     // printf ("\n\t%s\n", trace_buf.str().c_str());

  free (strings);
  exit(1);
}

void printFunctionStack()
{
    handler(0);
}

void Terminate(const char* location, const char* content)
{
    // 各进程先将错误信息打印到屏幕
    std::cout
        <<"Process ID: \033[31m"<<UTILITY::COMM::getGlobalId()
        <<"\033[0m, Location: \033[31m"<<location
        <<"\033[0m, error message: \033[31m"<<content
        <<"\033[0m, file: \033[31m"<<__FILE__
        <<"\033[0m, line: \033[31m"<<__LINE__
        <<"\033[0m\n"<<FLUSH;
    // 再将错误信息输出到文件
    UTILITY::POUT
        <<"Location: "<<location
        <<", error message: "<<content
        <<", file: "<<__FILE__
        <<", line: "<<__LINE__
        <<"\n"<<FLUSH;
    printFunctionStack();
    exit(-1);
}

void Warning(const char* location, const char* content)
{
    // 各进程先将错误信息打印到屏幕
    std::cout
        <<"Process ID: \033[31m"<<UTILITY::COMM::getGlobalId()
        <<"\033[0m, Location: \033[31m"<<location
        <<"\033[0m, error message: \033[31m"<<content
        <<"\033[0m, file: \033[31m"<<__FILE__
        <<"\033[0m, line: \033[31m"<<__LINE__
        <<"\033[0m\n"<<FLUSH;
    // 再将错误信息输出到文件
    UTILITY::POUT
        <<"Location: "<<location
        <<", error message: "<<content
        <<", file: "<<__FILE__
        <<", line: "<<__LINE__
        <<"\n"<<FLUSH;
    printFunctionStack();
}

