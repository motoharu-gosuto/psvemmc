#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "sdstor_log.h"

SceUID sdstor_dev_fs_log_fd = -1;

int sdstor_ref_ctx = 0; //very simple but probably will work

SceUID g_sdstorLogMutex = -1; //looks like we need to use mutex because of multithreading

void init_sdstor_log()
{
   g_sdstorLogMutex = ksceKernelCreateMutex("SdstorLogMutex", 0, 0, 0);
}

void deinit_sdstor_log()
{
  ksceKernelDeleteMutex(g_sdstorLogMutex);
  g_sdstorLogMutex = -1;
}

void FILE_SDSTOR_WRITE_LEN(char* msg)
{
  ksceKernelLockMutex(g_sdstorLogMutex, 1, 0);

  if(sdstor_dev_fs_log_fd >= 0) 
    ksceIoWrite(sdstor_dev_fs_log_fd, msg, strlen(msg));

  ksceKernelUnlockMutex(g_sdstorLogMutex, 1);
}

void open_sdstor_dev_fs_log()
{
  ksceKernelLockMutex(g_sdstorLogMutex, 1, 0);

  if(sdstor_dev_fs_log_fd == -1 && sdstor_ref_ctx == 0)
  {
    sdstor_dev_fs_log_fd = ksceIoOpen("ux0:dump/sdstor_dev_fs.txt", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);  
  }

  sdstor_ref_ctx++;

  ksceKernelUnlockMutex(g_sdstorLogMutex, 1);
}

void close_sdstor_dev_fs_log()
{
  ksceKernelLockMutex(g_sdstorLogMutex, 1, 0);

  sdstor_ref_ctx--;

  if(sdstor_dev_fs_log_fd >= 0 && sdstor_ref_ctx == 0)
  {
    ksceIoClose(sdstor_dev_fs_log_fd);
    sdstor_dev_fs_log_fd = -1;
  }

  ksceKernelUnlockMutex(g_sdstorLogMutex, 1);
} 
