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

SceUID sdstor_dev_fs_log_fd;

void open_sdstor_dev_fs_log()
{
  sdstor_dev_fs_log_fd = ksceIoOpen("ux0:dump/sdstor_dev_fs.txt", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);  
}

void close_sdstor_dev_fs_log()
{
  if(sdstor_dev_fs_log_fd >= 0)
    ksceIoClose(sdstor_dev_fs_log_fd);
} 
