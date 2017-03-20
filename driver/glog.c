#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "glog.h"

SceUID global_log_fd = -1;

int global_ref_ctx = 0; //very simple but probably will work

SceUID g_globalLogMutex = -1; //looks like we need to use mutex because of multithreading

void init_global_log()
{
   g_globalLogMutex = ksceKernelCreateMutex("GlobalLogMutex", 0, 0, 0);
}

void deinit_global_log()
{
  ksceKernelDeleteMutex(g_globalLogMutex);
  g_globalLogMutex = -1;
}

void FILE_GLOBAL_WRITE_LEN(char* msg)
{
  ksceKernelLockMutex(g_globalLogMutex, 1, 0);

  if(global_log_fd >= 0) 
    ksceIoWrite(global_log_fd, msg, strlen(msg));

  ksceKernelUnlockMutex(g_globalLogMutex, 1);
}

void open_global_log()
{
  ksceKernelLockMutex(g_globalLogMutex, 1, 0);

  if(global_log_fd == -1 && global_ref_ctx == 0)
  {
    global_log_fd = ksceIoOpen("ux0:dump/psvemmc_dump.bin", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);
  }

  global_ref_ctx++;

  ksceKernelUnlockMutex(g_globalLogMutex, 1);
}

void close_global_log()
{
  ksceKernelLockMutex(g_globalLogMutex, 1, 0);

  global_ref_ctx--;

  if(global_log_fd >= 0 && global_ref_ctx == 0)
  {
    ksceIoClose(global_log_fd);
    global_log_fd = -1;
  }

  ksceKernelUnlockMutex(g_globalLogMutex, 1);
} 
