
#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "thread_test.h"
#include "glog.h"
#include "stacktrace.h"

char sprintfBuffer[256];

//Read User read-only Thread and Process ID Register
int print_TPIDRURO()
{
  int TPIDRURO = 0;
  
  asm volatile ("mrc p15, 0, %0, c13, c0, 3" : "=r" (TPIDRURO)); //Read TPIDRURO into Rt
  
  {
    open_global_log();
    snprintf(sprintfBuffer, 256, "TPIDRURO %x\n", TPIDRURO);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    close_global_log();
  }
  
  return 0;
}

int TestThread(SceSize args, void *argp)
{
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("message from thread\n");
  close_global_log();
  
  //get_currentThreadId();
  //print_TPIDRURO();
  
  print_current_thread_info_global();
  
  return 0;  
}

int print_thread_info()
{
  SceUID newThid = ksceKernelCreateThread("TestThread", &TestThread, 0x64, 0x1000, 0, 0, 0);
  if(newThid < 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to create thread\n");
    close_global_log();
    return -1;
  }
  
  {
    open_global_log();
    snprintf(sprintfBuffer, 256, "created thread %x\n", newThid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    close_global_log();
  }
  
  int ret = ksceKernelStartThread(newThid, 0, 0);

  //----------------
  
  int waitRet = 0;
  ksceKernelWaitThreadEnd(newThid, &waitRet, 0);
  
  int delret = ksceKernelDeleteThread(newThid);
  if(delret < 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to delete thread\n");
    close_global_log();
  }
  
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("deleted thread\n");
  close_global_log();
  
  //get_currentThreadId();
  //print_TPIDRURO();
    
  return 0;
} 
