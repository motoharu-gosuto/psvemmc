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

char sprintfBuffer[256];

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
    snprintf(sprintfBuffer, 256, "vfs_func1: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func1_entered = 0;
  #endif

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
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
    snprintf(sprintfBuffer, 256, "vfs_func3: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func3_entered = 0;
  #endif

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
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
    snprintf(sprintfBuffer, 256, "vfs_func12: %x %s\n", ctx, ((ctx_C175D0*)ctx)->blockDeviceName);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func12_entered = 0;
  #endif

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
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
    snprintf(sprintfBuffer, 256, "vfs_func13: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);

    vfs_func13_args* args = (vfs_func13_args*)ctx;

    snprintf(sprintfBuffer, 256, "dev: %s num: %s res: %x\n", args->blockDeviceName, args->numericName, res);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);
  }
  close_sdstor_dev_fs_log();
  vfs_func13_entered = 0;
  #endif

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
  close_sdstor_dev_fs_log();

  return res;
}

sd_stor_device* get_sd_stor_device_base()
{
  tai_module_info_t m_info;
  m_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &m_info) >= 0) 
  {
    uintptr_t addr = 0;
    int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 1, 0x1558, &addr);
    if(ofstRes == 0)
    {
      sd_stor_device* data = (sd_stor_device*)addr;
      return data;
    }
  }
  return (sd_stor_device*)-1;
}

int get_sd_stor_dev_index_by_address(sd_stor_device* dev)
{
  sd_stor_device* base = get_sd_stor_device_base();
  if(dev == base)
    return 0;
  else if(dev == (base + 1))
    return 1;
  else if(dev == (base + 2))
    return 2;
  else if(dev == (base + 3))
    return 3;
  else if(dev == (base + 4))
    return 4;
  else
    return -1;
}

int print_sd_stor_dev_name_by_address(sd_stor_device* dev)
{
  int index = get_sd_stor_dev_index_by_address(dev);
  switch(index)
  {
    case 0:
    {
      open_sdstor_dev_fs_log();
      FILE_SDSTOR_WRITE_LEN("device: emmc\n");
      close_sdstor_dev_fs_log();
    }
    break;
    case 1:
    {
      open_sdstor_dev_fs_log();
      FILE_SDSTOR_WRITE_LEN("device: game card\n");
      close_sdstor_dev_fs_log();
    }
    break;
    case 2:
    {
      open_sdstor_dev_fs_log();
      FILE_SDSTOR_WRITE_LEN("device: memory card\n");
      close_sdstor_dev_fs_log();
    }
    break;
    case 3:
    {
      open_sdstor_dev_fs_log();
      FILE_SDSTOR_WRITE_LEN("device: unknown\n");
      close_sdstor_dev_fs_log();
    }
    break;
    case 4:
    {
      open_sdstor_dev_fs_log();
      FILE_SDSTOR_WRITE_LEN("device: unknown\n");
      close_sdstor_dev_fs_log();
    }
    break;
    default:
    {
      open_sdstor_dev_fs_log();
      FILE_SDSTOR_WRITE_LEN("device: invalid\n");
      close_sdstor_dev_fs_log();
    }
    break;
  }

  return 0;
}

int print_raw_partition_entry(partition_entry_raw* praw)
{

  return 0;
}

//I do not know how to map numeric name to block device

int print_data_corresponding_to_numeric_partition(char* numericName)
{
  //reverse of 
  //proc_init_mbr_table_init_partition_table_get_partition_entry_C1542C(blockDeviceName, len) - in SceSdstor
  //proc_get_mount_data_C15B80(blockDeviceName, nameLength, mnt) - in SceSdstor

  return 0; 
}

int print_vfs_node(vfs_node* node)
{
  if(node == 0)
  {
    open_sdstor_dev_fs_log();
    FILE_SDSTOR_WRITE_LEN("node is not set\n");
    close_sdstor_dev_fs_log();
    return 0;
  }

  if(node->prev_node == 0)
  {
    open_sdstor_dev_fs_log();
    FILE_SDSTOR_WRITE_LEN("prev_node is not set\n");
    close_sdstor_dev_fs_log();
  }
  else
  {
    open_sdstor_dev_fs_log();
    {
      snprintf(sprintfBuffer, 256, "nodes: %x %x\n", node, node->prev_node->unk_54);
      FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
    }
    close_sdstor_dev_fs_log();
  }    

  vfs_device_info* dev_info = (vfs_device_info*)node->dev_info;
  if(dev_info == 0)
  {
    open_sdstor_dev_fs_log();
    FILE_SDSTOR_WRITE_LEN("dev info is not set\n");
    close_sdstor_dev_fs_log();
    return 0;
  }

  open_sdstor_dev_fs_log();
  {
    snprintf(sprintfBuffer, 256, "dev info: %x %x %x\n", dev_info->partition, dev_info->device, dev_info->unk_8);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();

  partition_entry* partition = dev_info->partition;

  if(partition != 0)
  {
    open_sdstor_dev_fs_log();
    {
      snprintf(sprintfBuffer, 256, "partition: %s\n", partition->numericName);
      FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
    }
    close_sdstor_dev_fs_log();

    print_data_corresponding_to_numeric_partition(partition->numericName);

    partition_entry_raw* praw = partition->partitionEntryPtr;
    if(praw != 0)
    {
      print_raw_partition_entry(praw);
    }
    else
    {
      open_sdstor_dev_fs_log();
      FILE_SDSTOR_WRITE_LEN("partition raw: not set\n");
      close_sdstor_dev_fs_log();
    }
  }
  else
  {
    open_sdstor_dev_fs_log();
    FILE_SDSTOR_WRITE_LEN("partition: not set\n");
    close_sdstor_dev_fs_log();
  }

  if(node->prev_node != 0)
  {
    open_sdstor_dev_fs_log();
    {
      snprintf(sprintfBuffer, 256, "prev node is initialized: %x\n", node->prev_node);
      FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
    }
    close_sdstor_dev_fs_log();
  }

  sd_stor_device* device = dev_info->device;

  if(device != 0)
  {
    print_sd_stor_dev_name_by_address(device);
  }
  else
  {
    open_sdstor_dev_fs_log();
    FILE_SDSTOR_WRITE_LEN("device: not set\n");
    close_sdstor_dev_fs_log();
  }

  return 0;
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
    snprintf(sprintfBuffer, 256, "vfs_node_func1: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);

    /*
    snprintf(sprintfBuffer, 256, "node: %08x\narg1: %08x\narg2: %08x\narg3: %08x\nret:%08x\n", args->node, args->arg1, args->arg2, args->arg3, res);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
    */

    snprintf(sprintfBuffer, 256, "dev: %s mount: %s res: %x\n", args->arg1->blockDevice, args->arg1->unixMount, res);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);
  }
  close_sdstor_dev_fs_log();

  vfs_node_func1_entered = 0;
  #endif

  print_vfs_node(args->node);

  //print_vfs_node(args->node->prev_node);

  /*
  vfs_node* cur_node = args->node;
  while(cur_node != 0)
  {
    print_vfs_node(cur_node);
    cur_node = cur_node->prev_node;
  }
  */

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
  close_sdstor_dev_fs_log();

  dump_sdstor_data();
  dump_sdif_data();
  dump_exfatfs_data();

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
    snprintf(sprintfBuffer, 256, "vfs_node_func3: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func3_entered = 0;
  #endif

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
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
    snprintf(sprintfBuffer, 256, "vfs_node_func4: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);

    snprintf(sprintfBuffer, 256, "node: %x dev: %s arg1: %x arg3: %x res: %x\n", args->node, args->dev->blockDeviceName, *args->new_node, args->arg3, res);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func4_entered = 0;
  #endif

  print_vfs_node(args->node);

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("----\n");
  close_sdstor_dev_fs_log();

  print_vfs_node(*args->new_node);

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
  close_sdstor_dev_fs_log();

  return res;
}

//int vfs_node_func5_entered = 0;

int vfs_node_func5_num = 0;

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
    snprintf(sprintfBuffer, 256, "vfs_node_func5: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  */
  //vfs_node_func5_entered = 0;
  #endif

  //send_message_to_client("vfs_node_func5", sizeof("vfs_node_func5"));
  vfs_node_func5_num++;

  return res;
}

//int vfs_node_func6_entered = 0;

int vfs_node_func6_num = 0;

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
    snprintf(sprintfBuffer, 256, "vfs_node_func6: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  */
  //vfs_node_func6_entered = 0;
  #endif

  //send_message_to_client("vfs_node_func6", sizeof("vfs_node_func6"));
  vfs_node_func6_num++;

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
    snprintf(sprintfBuffer, 256, "vfs_node_func7: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);

    snprintf(sprintfBuffer, 256, "node: %08x arg1: %08x arg2: %08x arg3: %08x arg_0: %08x res:%08x\n", args->node, args->arg1, args->arg2, args->arg3, args->arg_0, res);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);

    //send_message_to_client("vfs_node_func7", sizeof("vfs_node_func7"));
  }
  close_sdstor_dev_fs_log();
  vfs_node_func7_entered = 0;
  #endif

  print_vfs_node(args->node);

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
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
    snprintf(sprintfBuffer, 256, "vfs_node_func9: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);

    snprintf(sprintfBuffer, 256, "node0: %08x node1: %08x dev: %s arg3: %08x res:%08x\n", args->node0, args->node1, args->arg2->blockDeviceName, args->arg3, res);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);

    //======================
    snprintf(sprintfBuffer, 256, "vfs_node_func5_num %x vfs_node_func6_num: %x\n", vfs_node_func5_num, vfs_node_func6_num);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);

    //send_message_to_client(sprintfBuffer, 256);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func9_entered = 0;
  #endif

  print_vfs_node(args->node0);

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("----\n");
  close_sdstor_dev_fs_log();

  print_vfs_node(args->node1);

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
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
    snprintf(sprintfBuffer, 256, "vfs_node_func19: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func19_entered = 0;
  #endif

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
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
    snprintf(sprintfBuffer, 256, "vfs_node_func20: %x\n", ctx);
    FILE_SDSTOR_WRITE_LEN(sprintfBuffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func20_entered = 0;
  #endif

  /*
  stacktrace_from_here_sd("SceIofilemgr", 0, 100, 0);
  print_current_thread_info_sd();
  */

  open_sdstor_dev_fs_log();
  FILE_SDSTOR_WRITE_LEN("\n");
  close_sdstor_dev_fs_log();

  return res;
}