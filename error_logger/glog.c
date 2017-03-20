#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>

#include <taihen.h>
#include <error.h>

#include <stdio.h>
#include <string.h>

#include "glog.h"

SceUID global_log_fd;

void open_global_log()
{
  global_log_fd = sceIoOpen("ux0:dump/error_logger.txt", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);
  
  if (global_log_fd < 0) 
    LOG("file not found");
}

void close_global_log()
{
  if(global_log_fd >= 0)
    sceIoClose(global_log_fd);
}