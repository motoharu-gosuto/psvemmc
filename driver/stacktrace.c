#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <module.h>

#include "stacktrace.h"

#include "glog.h"
#include "sdstor_log.h"
#include "mtable.h"

char sprintfBuffer[256];

//Read Privileged only Thread and Process ID Register
SceUID get_currentThreadId()
{
  int TPIDRPRW = 0;
  
  asm volatile ("mrc p15, 0, %0, c13, c0, 4" : "=r" (TPIDRPRW)); //Read TPIDRPRW into Rt
  
  /*
  {
    open_global_log();
    snprintf(sprintfBuffer, 256, "TPIDRPRW %x\n", TPIDRPRW);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    close_global_log();
  }
  */
  
  if(TPIDRPRW > 0)
  {
    int* dataPtr = (int*)(((char*)TPIDRPRW) + 0x8);
    int value = *dataPtr;
    
    /*
    {
      open_global_log();
      snprintf(sprintfBuffer, 256, "value %x\n", value);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      close_global_log();
    }
    */
    
    return value;
  }
  
  return -1;
}

typedef int (ksceKernelGetThreadInfo_func)(SceUID thid, SceKernelThreadInfo *info);

SceUID g_threadInfoMutex = -1;

void init_threadInfoMutex()
{
  g_threadInfoMutex = ksceKernelCreateMutex("ThreadInfoMutex", 0, 0, 0);
}

void deinit_threadInfoMutex()
{
  ksceKernelDeleteMutex(g_threadInfoMutex);
  g_threadInfoMutex = -1;
}

int get_current_thread_info(SceKernelThreadInfo* t_info)
{
  ksceKernelLockMutex(g_threadInfoMutex, 1, 0);

  SceUID thid = get_currentThreadId();
  
  memset(t_info, 0, sizeof(SceKernelThreadInfo));
  t_info->size = sizeof(SceKernelThreadInfo);
 
  //need to call function dynamically because it is not defined yet
  
  tai_module_info_t m_info;
  m_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceKernelThreadMgr", &m_info) >= 0) 
  {
    uintptr_t addr = 0;
    //int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 0, 0x5964, &addr);
    int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 0, 0x5965, &addr);
    if(ofstRes == 0)
    {
      /*
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("ready to call ksceKernelGetThreadInfo\n");
      close_global_log();
      */
      
      ksceKernelGetThreadInfo_func* fptr = (ksceKernelGetThreadInfo_func*)addr;
      
      int gtiRes = fptr(thid, t_info);
      /*
      if(gtiRes >= 0)
      {
        open_global_log();
        FILE_GLOBAL_WRITE_LEN("ksceKernelGetThreadInfo success\n");
        close_global_log();
      }
      */
      
      ksceKernelUnlockMutex(g_threadInfoMutex, 1);

      return gtiRes;
    }
  }
  
  ksceKernelUnlockMutex(g_threadInfoMutex, 1);

  return -1;
}

int print_current_thread_info_global()
{
  SceKernelThreadInfo t_info;
  get_current_thread_info(&t_info);
  
  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "(global) process: %08x thread: %s\nstack: %08x stackSize: %08x\n", t_info.processId, t_info.name, t_info.stack, t_info.stackSize);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();
  
  return 0;
}

//TODO: this is copypaste

int print_current_thread_info_sd()
{
  SceKernelThreadInfo t_info;
  get_current_thread_info(&t_info);
  
  open_sdstor_dev_fs_log();
  {
    snprintf(sprintfBuffer, 256, "(sd) process: %08x thread: %s\nstack: %08x stackSize: %08x\n", t_info.processId, t_info.name, t_info.stack, t_info.stackSize);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  
  return 0;
}

int stacktrace_from_here_global(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose)
{
  //must be specified volatile or optimizer will do what it likes

  //I use this variables just for marking to see that stack data that I get is adequate
  volatile int mark0 = 0xA0A0A0A0;
  volatile int mark1 = 0x05050505;
  volatile int mark2 = 0x37373737;
  
  SceKernelThreadInfo t_info;
  get_current_thread_info(&t_info);

  //------------------------
  //unless modules are reloaded, which is most likely not happening, we can do it once during this module load
  //construct_module_range_table();
  //sort_segment_table();
  //print_segment_table();

  volatile int* stackPtr = &mark0;

  //for(int i = 0; stackPtr < (volatile int*)t_info.stack; i++)
  
  int traceSize = stackSize + sizeof(SceKernelThreadInfo) / 4;
  
  for(int i = 0; i < traceSize; i++)
  {
    int curValue = *stackPtr;
    int segidx = find_in_segments(g_segListKernel, SEG_LIST_SIZE, &moduleListIsConstructedKernel, curValue);
    if(segidx >= 0)
    {
      if(verbose == 1)
      {
        open_global_log();
        {
          snprintf(sprintfBuffer, 256, "(global) %08x: %08x %s %d %08x %08x\n", stackPtr, curValue, g_segListKernel[segidx].moduleName, g_segListKernel[segidx].seg, g_segListKernel[segidx].range.start, (curValue - g_segListKernel[segidx].range.start));
          FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        }
        close_global_log();
      }
      else
      {
        if(g_segListKernel[segidx].seg == segIndexSearch)
        {
          if(strcmp(moduleNameSearch, g_segListKernel[segidx].moduleName) == 0)
          {
            open_global_log();
            {
              snprintf(sprintfBuffer, 256, "(global) %08x: %08x %s %d %08x %08x\n", stackPtr, curValue, g_segListKernel[segidx].moduleName, g_segListKernel[segidx].seg, g_segListKernel[segidx].range.start, (curValue - g_segListKernel[segidx].range.start));
              FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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

//TODO: this is copypaste

int stacktrace_from_here_sd(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose)
{
  //must be specified volatile or optimizer will do what it likes

  //I use this variables just for marking to see that stack data that I get is adequate
  volatile int mark0 = 0xA0A0A0A0;
  volatile int mark1 = 0x05050505;
  volatile int mark2 = 0x37373737;
  
  SceKernelThreadInfo t_info;
  get_current_thread_info(&t_info);

  //------------------------
  //unless modules are reloaded, which is most likely not happening, we can do it once during this module load
  //construct_module_range_table();
  //sort_segment_table();
  //print_segment_table();

  volatile int* stackPtr = &mark0;

  //for(int i = 0; stackPtr < (volatile int*)t_info.stack; i++)
  
  int traceSize = stackSize + sizeof(SceKernelThreadInfo) / 4;
  
  for(int i = 0; i < traceSize; i++)
  {
    int curValue = *stackPtr;
    int segidx = find_in_segments(g_segListKernel, SEG_LIST_SIZE, &moduleListIsConstructedKernel, curValue);
    if(segidx >= 0)
    {
      if(verbose == 1)
      {
        open_sdstor_dev_fs_log();
        {
          snprintf(sprintfBuffer, 256, "(sd) %08x: %08x %s %d %08x %08x\n", stackPtr, curValue, g_segListKernel[segidx].moduleName, g_segListKernel[segidx].seg, g_segListKernel[segidx].range.start, (curValue - g_segListKernel[segidx].range.start));
          FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
        }
        close_sdstor_dev_fs_log();
      }
      else
      {
        if(g_segListKernel[segidx].seg == segIndexSearch)
        {
          if(strcmp(moduleNameSearch, g_segListKernel[segidx].moduleName) == 0)
          {
            open_sdstor_dev_fs_log();
            {
              snprintf(sprintfBuffer, 256, "(sd) %08x: %08x %s %d %08x %08x\n", stackPtr, curValue, g_segListKernel[segidx].moduleName, g_segListKernel[segidx].seg, g_segListKernel[segidx].range.start, (curValue - g_segListKernel[segidx].range.start));
              FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
            }
            close_sdstor_dev_fs_log();
          }
        } 
      }    
    }

    stackPtr++;
  }

  return 0;
} 
