#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <taihen.h>
#include <module.h>

#include "sector_api.h"

#include "glog.h"
#include "sdstor_log.h"
#include "dump.h"
#include "thread_test.h"
#include "net.h"
#include "mtable.h"
#include "stacktrace.h"

#include "hooks_vfs_ops.h" 

tai_hook_ref_t sdstor_dev_fs_refs[13];
SceUID sdstor_dev_fs_ids[13] = {-1};

//=======================================

#define ENABLE_LOG_VFS_SD_NODE

//=======================================

int vfs_func1_entered = 0;

int vfs_func1(void* ctx) //00C17015
{
  if(vfs_func1_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[0], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func1_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func1: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func1_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_func3_entered = 0;

int vfs_func3(void* ctx) //00C1703D
{
  if(vfs_func3_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[1], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func3_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func3: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func3_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_func12_entered = 0;

int vfs_func12(void* ctx) //00C175D1
{
  if(vfs_func12_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[2], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func12_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func12: %x %s\n", ctx, ((ctx_C175D0*)ctx)->blockDeviceName);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func12_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_func13_entered = 0;

int vfs_func13(void* ctx) //00C17551
{
  if(vfs_func13_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[3], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func13_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func13: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 100);

    snprintf(buffer, 100, "dev: %s res: %x\n", ((ctx_C17550*)ctx)->blockDeviceName, res);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 100);
  }
  close_sdstor_dev_fs_log();
  vfs_func13_entered = 0;
  #endif

  /*
  stacktrace_from_here("SceIofilemgr", 0);
  */
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_node_func1_entered = 0;

int vfs_node_func1(void* ctx) //00C17465
{
  if(vfs_node_func1_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[4], ctx);

  vfs_node_func1_args* args = (vfs_node_func1_args*)ctx;

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func1_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[120];
    snprintf(buffer, 120, "vfs_node_func1: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 120);

    /*
    snprintf(buffer, 120, "node: %08x\narg1: %08x\narg2: %08x\narg3: %08x\nret:%08x\n", args->node, args->arg1, args->arg2, args->arg3, res);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
    */

    snprintf(buffer, 120, "dev: %s mount: %s res: %x\n", args->arg1->blockDevice, args->arg1->unixMount, res);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 120);
  }
  close_sdstor_dev_fs_log();

  vfs_node_func1_entered = 0;
  #endif

  /*
  stacktrace_from_here("SceIofilemgr", 0);
  */
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_node_func3_entered = 0;

int vfs_node_func3(void* ctx) //00C17459
{
  if(vfs_node_func3_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[5], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func3_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func3: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func3_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_node_func4_entered = 0;

int vfs_node_func4(void* ctx) //00C172E1
{
  if(vfs_node_func4_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[6], ctx);

  vfs_node_func4_args* args = (vfs_node_func4_args*)ctx;

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func4_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func4: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 100);

    snprintf(buffer, 100, "node: %x dev: %s arg1: %x arg3: %x res: %x\n", args->node, args->arg2->blockDeviceName, *args->arg1, args->arg3, res);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 100);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func4_entered = 0;
  #endif

  /*
  stacktrace_from_here("SceIofilemgr", 0);
  */
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

//int vfs_node_func5_entered = 0;

int vfs_node_func5(void* ctx) //00C17231 - HOOK DOES NOT WORK when writing to file is done
{
  //if(vfs_node_func5_entered == 1)
  //  return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[7], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  //vfs_node_func5_entered = 1;
  /*
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func5: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  */
  //vfs_node_func5_entered = 0;
  #endif

  return res;
}

//int vfs_node_func6_entered = 0;

int vfs_node_func6(void* ctx) //00C1717D - HOOK DOES NOT WORK when writing to file is done
{
  //if(vfs_node_func6_entered == 1)
  //  return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[8], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  //vfs_node_func6_entered = 1;
  /*
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func6: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  */
  //vfs_node_func6_entered = 0;
  #endif

  return res;
}

int vfs_node_func7_entered = 0;

//this function returns arg2 (size?) on success

int vfs_node_func7(void* ctx) //00C170C5
{
  if(vfs_node_func7_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[9], ctx);

  vfs_node_func7_args* args = (vfs_node_func7_args*)ctx;

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func7_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[120];
    snprintf(buffer, 120, "vfs_node_func7: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 120);

    snprintf(buffer, 120, "node: %08x arg1: %08x arg2: %08x arg3: %08x arg_0: %08x res:%08x\n", args->node, args->arg1, args->arg2, args->arg3, args->arg_0, res);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 120);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func7_entered = 0;
  #endif

  /*
  stacktrace_from_here("SceIofilemgr", 0);
  */
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_node_func9_entered = 0;

int vfs_node_func9(void* ctx) //00C17291
{
  if(vfs_node_func9_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[10], ctx);

  vfs_node_func9_args* args = (vfs_node_func9_args*)ctx;

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func9_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[140];
    snprintf(buffer, 140, "vfs_node_func9: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 140);

    snprintf(buffer, 140, "node0: %08x node1: %08x dev: %s arg3: %08x res:%08x\n", args->node0, args->node1, args->arg2->blockDeviceName, args->arg3, res);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);

    send_message_to_client(buffer, 140);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func9_entered = 0;
  #endif

  /*
  stacktrace_from_here("SceIofilemgr", 0);
  */
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_node_func19_entered = 0;

int vfs_node_func19(void* ctx) //00C171DD
{
  if(vfs_node_func19_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[11], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func19_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func19: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func19_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}

int vfs_node_func20_entered = 0;

int vfs_node_func20(void* ctx) //00C17071
{
  if(vfs_node_func20_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[12], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func20_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func20: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func20_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);
  print_current_thread_info();

  open_sdstor_dev_fs_log();
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return res;
}