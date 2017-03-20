#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <module.h>

#include "stacktrace.h"

#include "glog.h"
#include "mtable.h"

char sprintfBuffer[256];

int get_current_thread_info(SceKernelThreadInfo* t_info)
{
  SceUID thid = sceKernelGetThreadId();
  
  sceClibMemset(t_info, 0, sizeof(SceKernelThreadInfo));
  t_info->size = sizeof(SceKernelThreadInfo);
 
  int gtiRes = sceKernelGetThreadInfo(thid, t_info);
  return gtiRes;
  
  return -1;
}

int print_current_thread_info_global_base(SceKernelThreadInfo* t_info)
{
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "(global) process: %08x thread: %s\nstack: %p stackSize: %08x\n", t_info->processId, t_info->name, t_info->stack, t_info->stackSize);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  
  return 0;
}

int print_current_thread_info_global()
{
  SceKernelThreadInfo t_info;
  get_current_thread_info(&t_info);
  
  print_current_thread_info_global_base(&t_info);
  
  return 0;
}

int stacktrace_global(volatile int* stackPtr, char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose, uint32_t* addresses, uint32_t* addressNum)
{
  *addressNum = 0;

  //for(int i = 0; stackPtr < (volatile int*)t_info.stack; i++)
  
  int traceSize = stackSize + sizeof(SceKernelThreadInfo) / 4;
  
  for(int i = 0; i < traceSize; i++)
  {
    int curValue = *stackPtr;
    int segidx = find_in_segments(curValue);
    if(segidx >= 0)
    {
      *addresses = curValue;
      addresses++;
      
      (*addressNum)++;
      
      if(verbose == 1)
      {
        open_global_log();
        {
          sceClibSnprintf(sprintfBuffer, 256, "(global) %08x: %08x %s %d %08x %08x\n", (unsigned int)stackPtr, curValue, g_segList[segidx].moduleName, g_segList[segidx].seg, g_segList[segidx].range.start, (curValue - g_segList[segidx].range.start));
          FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
        }
        close_global_log();
      }
      else
      {
        if(g_segList[segidx].seg == segIndexSearch)
        {
          if(sceClibStrcmp(moduleNameSearch, g_segList[segidx].moduleName) == 0)
          {
            open_global_log();
            {
              sceClibSnprintf(sprintfBuffer, 256, "(global) %08x: %08x %s %d %08x %08x\n", (unsigned int)stackPtr, curValue, g_segList[segidx].moduleName, g_segList[segidx].seg, g_segList[segidx].range.start, (curValue - g_segList[segidx].range.start));
              FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
            }
            close_global_log();
          }
        }
      }    
    }

    stackPtr++;
  }

  return 0;
}

int stacktrace_from_here_global(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose, uint32_t* addresses, uint32_t* addressNum)
{
  //must be specified volatile or optimizer will do what it likes

  //I use this variables just for marking to see that stack data that I get is adequate
  volatile int mark0 = 0xA0A0A0A0;
  /*
  volatile int mark1 = 0x05050505;
  volatile int mark2 = 0x37373737;
  */
  
  /*
  SceKernelThreadInfo t_info;
  get_current_thread_info(&t_info);
  */

  //------------------------
  //unless modules are reloaded, which is most likely not happening, we can do it once during this module load
  //construct_module_range_table();
  //sort_segment_table();
  //print_segment_table();

  volatile int* stackPtr = &mark0;

  stacktrace_global(stackPtr, moduleNameSearch, segIndexSearch, stackSize, verbose, addresses, addressNum);

  return 0;
}