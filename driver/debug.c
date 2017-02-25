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

//=================================================

tai_hook_ref_t gc_hook_ref;
SceUID gc_hook_id = -1; //hook of CMD56 init routine in SblGcAuthMgr

tai_hook_ref_t init_mmc_hook_ref;
SceUID init_mmc_hook_id = -1; //hook of mmc init function in Sdif

tai_hook_ref_t init_sd_hook_ref;
SceUID init_sd_hook_id = -1; // hook of sd init function in Sdif

SceUID patch_uids[3]; //these are used to patch number of iterations for CMD55, ACMD41 in Sdif

tai_hook_ref_t gen_init_hook_refs[3];
SceUID gen_init_hook_uids[3]; //these are used to hook generic init functions in SdStor

tai_hook_ref_t load_mbr_hook_ref;
SceUID load_mbr_hook_id = -1;

tai_hook_ref_t  mnt_pnt_chk_hook_ref;
SceUID mnt_pnt_chk_hook_id = -1;

tai_hook_ref_t mbr_table_init_hook_ref;
SceUID mbr_table_init_hook_id = -1;

tai_hook_ref_t cmd55_41_hook_ref; //hook of CMD55, ACMD41 preinit function in Sdif
SceUID cmd55_41_hook_id = -1;

tai_hook_ref_t sysroot_zero_hook_ref;
SceUID sysroot_zero_hook_id = -1;

tai_hook_ref_t sdstor_dev_fs_refs[13];
SceUID sdstor_dev_fs_ids[13] = {-1};

tai_hook_ref_t sceVfsMount_hook_ref;
tai_hook_ref_t sceVfsAddVfs_hook_ref;
tai_hook_ref_t sceVfsUnmount_hook_ref;
tai_hook_ref_t sceVfsDeleteVfs_hook_ref;
tai_hook_ref_t sceVfsGetNewNode_hook_ref;

SceUID sceVfsMount_hook_id = -1;
SceUID sceVfsAddVfs_hook_id = -1;
SceUID sceVfsUnmount_hook_id = -1;
SceUID sceVfsDeleteVfs_hook_id = -1;
SceUID sceVfsGetNewNode_hook_id = -1;
 
//==============================================

int print_bytes(char* bytes, int size)
{ 
  open_sdstor_dev_fs_log();
  for(int i = 0; i < size; i++)
  {
    char buffer[4];
    snprintf(buffer, 4, "%02x ", bytes[i]);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  FILE_WRITE(sdstor_dev_fs_log_fd, "\n");
  close_sdstor_dev_fs_log();

  return 0;
}

//==================================

#pragma pack(push, 1)

typedef struct device_init_info
{
  int sd_ctx_index;
  sd_context_part* ctx;
}device_init_info;

#pragma pack(pop)

#define DEVICE_INFO_SIZE 4

int last_mmc_index = 0;
int last_sd_index = 0;

device_init_info last_mmc_inits[DEVICE_INFO_SIZE];
device_init_info last_sd_inits[DEVICE_INFO_SIZE];

int clear_device_info_arrays()
{
  memset(last_mmc_inits, -1, sizeof(device_init_info) * DEVICE_INFO_SIZE);
  memset(last_sd_inits, -1, sizeof(device_init_info) * DEVICE_INFO_SIZE);
  return 0;
}

int print_device_info_arrays()
{
  char buffer[100];
  
  open_global_log();
  FILE_WRITE(global_log_fd, "------ mmc -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(buffer, 100, "idx:%x ctx:%x\n", last_mmc_inits[i].sd_ctx_index, last_mmc_inits[i].ctx);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  FILE_WRITE(global_log_fd, "------ sd  -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(buffer, 100, "idx:%x ctx:%x\n", last_sd_inits[i].sd_ctx_index, last_sd_inits[i].ctx);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return 0;
}

//-----------

int gc_sd_init(void* args)
{
   sd_context_part* ctx_00BDCBC0 = ksceSdifGetSdContextPartSd(SCE_SDIF_DEV_GAME_CARD);
   if(ctx_00BDCBC0 == 0)
   {
      int res = ksceSdifInitializeSdContextPartSd(SCE_SDIF_DEV_GAME_CARD, &ctx_00BDCBC0);
      if(res != 0)
         return 0x808A0703;
   }
   return 0;
}

int gc_patch(int param0)
{
  /*
  int var_10 = param0;
  return ksceKernelRunWithStack(0x2000, &gc_sd_init, &var_10);
  */
  
  int res = TAI_CONTINUE(int, gc_hook_ref, param0);
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "call gc auth res:%x\n", res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return res;
}

int init_mmc_hook(int sd_ctx_index, sd_context_part** result)
{
  int res = TAI_CONTINUE(int, init_mmc_hook_ref, sd_ctx_index, result);
  
  /*
  int res = 0;
  
  //forward game card initialization to anoher function
  //other initializations should be fowarded to standard function
  
  if(sd_ctx_index == SCE_SDIF_DEV_GAME_CARD)
  {
    res = ksceSdifInitializeSdContextPartSd(sd_ctx_index, result);
  }
  else
  {
    res = TAI_CONTINUE(int, init_mmc_hook_ref, sd_ctx_index, result);
  }
  */
  
  last_mmc_inits[last_mmc_index].sd_ctx_index = sd_ctx_index;
  if(result != 0)
    last_mmc_inits[last_mmc_index].ctx = *result;
  else
    last_mmc_inits[last_mmc_index].ctx = (sd_context_part*)-1;
  
  last_mmc_index++;
  if(last_mmc_index == DEVICE_INFO_SIZE)
    last_mmc_index = 0;
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "init mmc - idx:%x ctx:%x res:%x\n", sd_ctx_index, *result, res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();

  return res;
}

int init_sd_hook(int sd_ctx_index, sd_context_part** result)
{
  int res = TAI_CONTINUE(int, init_sd_hook_ref, sd_ctx_index, result);
  
  last_sd_inits[last_sd_index].sd_ctx_index = sd_ctx_index;
  if(result != 0)
    last_sd_inits[last_sd_index].ctx = *result;
  else
    last_sd_inits[last_sd_index].ctx = (sd_context_part*)-1;
  
  last_sd_index++;
  if(last_sd_index == DEVICE_INFO_SIZE)
    last_sd_index = 0;
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "init sd - idx:%x ctx:%x res:%x\n", sd_ctx_index, *result, res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  //initialize_gc_globals(); //initialize all globals here since it can not be done on boot
  
  return res;
}

int cmd55_41_hook(sd_context_global* ctx)
{
  int res = TAI_CONTINUE(int, cmd55_41_hook_ref, ctx);
  
  /*
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "res cmd55_41:%x\n", res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  */
  
  return res;
}

int gen_init_hook_1(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[0], ctx);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called gen_init_hook_1\n");
  }
  close_global_log();
  
  return res;
}

int gen_init_hook_2(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[1], ctx);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called gen_init_hook_2\n");
  }
  close_global_log();
  
  return res;
}

int gen_init_hook_3(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[2], ctx);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called gen_init_hook_3\n");
  }
  close_global_log();
  
  return res;
}

int sysroot_zero_hook()
{
  int res = TAI_CONTINUE(int, sysroot_zero_hook_ref);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called sysroot_zero_hook\n");
  }
  close_global_log();
  
  return res;
  
  //returning 1 here enables sd init
  //however it breaks existing functionality, including:
  //insertion detection of the card - looks like initilization of card is started upon insertion, however no "please wait" dialog is shown and card is not detected
  //upon suspend and then resume - causes hang of the whole system. touch does not respond, unable to power off, have to take out baterry
  
  return 1; //return 1 instead of hardcoded 0
}

int load_mbr_hook(int ctx_index)
{
  int res = TAI_CONTINUE(int, load_mbr_hook_ref, ctx_index);
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "called load_mbr_hook: %x\n", ctx_index);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return res;
}

int mnt_pnt_chk_hook(char* blockDeviceName, int mountNum, int* mountData)
{
  int res = TAI_CONTINUE(int, mnt_pnt_chk_hook_ref, blockDeviceName, mountNum, mountData);

  open_global_log();
  {
    if(blockDeviceName == 0 || mountData == 0)
    {
      FILE_WRITE(global_log_fd, "called mnt_pnt_chk_hook: data is invalid\n");
    }
    else
    {
      char buffer[200];
      snprintf(buffer, 200, "called mnt_pnt_chk_hook: %s %08x %08x %08x\n", blockDeviceName, mountNum, *mountData, res);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
  }
  close_global_log();

  return res;
}

int mbr_table_init_hook(char* blockDeviceName, int mountNum)
{
  int res = TAI_CONTINUE(int, mbr_table_init_hook_ref, blockDeviceName, mountNum);

  open_global_log();
  {
    if(blockDeviceName == 0)
    {
      FILE_WRITE(global_log_fd, "called mbr_table_init_hook: data is invalid\n");
    }
    else
    {
      char buffer[200];
      snprintf(buffer, 200, "called mbr_table_init_hook: %s %08x %08x\n", blockDeviceName, mountNum, res);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
  }
  close_global_log();

  return res;
}

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

typedef struct ctx_C175D0
{
   int unk_0;
   int unk_4;
   int unk_8;
   
   char* blockDeviceName;
   int nameLength;
   
   int unk_14;
   int unk_18;
} ctx_C175D0;

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

typedef struct ctx_C17550
{
   int unk_0;
   char* blockDeviceName;
   int unk_8;
   int unk_C;
   
   char* dest;
   int len;
   int unk_18;
   
}ctx_C17550;

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

//probably vnodeops
typedef struct node_ops2 // size is 0x74 (29 pointers)
{
  int (*func1)(void* ctx); // sceIoOpenForDriver
  int (*func2)(void* ctx); // sceIoOpenForDriver (another one ?)
  int (*func3)(void* ctx); // ?
  int (*func4)(void* ctx); // ?
  int (*func5)(void* ctx); // sceIoReadForDriver
  int (*func6)(void* ctx); // sceIoWriteForDriver
  int (*func7)(void* ctx); // sceIoLseekForDriver or sceIoLseek32?
  int (*func8)(void* ctx); // sceIoIoctlForDriver
  int (*func9)(void* ctx); // ?
  int (*func10)(void* ctx); // sceIoMkdirForDriver
  int (*func11)(void* ctx); // sceIoRmdirForDriver
  int (*func12)(void* ctx); // sceIoDopenForDriver
  int (*func13)(void* ctx); // sceIoDcloseForDriver
  int (*func14)(void* ctx); // sceIoDreadForDriver
  int (*func15)(void* ctx); // sceIoGetstatForDriver or sceIoGetstatByFdForDriver
  int (*func16)(void* ctx); // sceIoChstatForDriver or sceIoChstatByFdForDriver
  int (*func17)(void* ctx); // sceIoRenameForDriver
  int (*func18)(void* ctx); // not implemented by all
  int (*func19)(void* ctx); // sceIoPreadForDriver
  int (*func20)(void* ctx); // sceIoPwriteForDriver
  int (*func21)(void* ctx); // ?
  int (*func22)(void* ctx); // not referenced
  int (*func23)(void* ctx); // not referenced
  int (*func24)(void* ctx); // sceIoSyncForDriver or sceIoSyncByFdForDriver
  int (*func25)(void* ctx); // sceIoGetstatByFdForDriver
  int (*func26)(void* ctx); // sceIoChstatByFdForDriver
  int (*func27)(void* ctx); // ?
  int (*func28)(void* ctx); // ?
  int (*func29)(void* ctx); // not implemented by all
} node_ops2;

typedef struct vfs_node_unk_48_unk_0
{
  uint32_t unk_0;
  uint32_t unk_4;
  uint32_t unk_8; //not a pointer
  uint32_t unk_C;  //not a pointer

  //can be more bytes

}vfs_node_unk_48_unk_0;

typedef struct vfs_node_unk_48_unk_4_2
{
  //same pointers?
  void* unk_0;
  void* unk_4;

  uint32_t unk_8;
  uint32_t unk_C;
} vfs_node_unk_48_unk_4_2;

typedef struct vfs_node_unk_48_unk_4_1
{
  //same pointers?
  void* unk_0;
  void* unk_4;

  void* unk_8;
  void* unk_C;
} vfs_node_unk_48_unk_4_1;

typedef struct vfs_node_unk_48_unk_4
{
  //same pointers?
  vfs_node_unk_48_unk_4_1* unk_0;
  vfs_node_unk_48_unk_4_1* unk_4;

  uint32_t unk_8; //not a pointer
  vfs_node_unk_48_unk_4_2* unk_C;

  //can be more bytes

}vfs_node_unk_48_unk_4;

typedef struct vfs_node_unk_48
{
  vfs_node_unk_48_unk_0* unk_0;
  vfs_node_unk_48_unk_4* unk_4;

  //can be more bytes
}vfs_node_unk_48;

typedef struct vfs_node_unk_50
{
  //same pointers?
  void* unk_0;
  void* unk_4;

  uint32_t unk_8;
  void* unk_C;

  //can be more bytes
} vfs_node_unk_50;

typedef struct vfs_node_unk_54
{
  uint32_t unk_0;
  uint32_t unk_4;

  uint32_t unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_54;

typedef struct vfs_node_unk_70_1
{
  void* unk_0;
  void* unk_4;

  void* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70_1;

typedef struct vfs_node_unk_70_3
{
  void* unk_0;
  void* unk_4;

  void* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70_3;

typedef struct vfs_node_unk_70_2
{
  uint32_t unk_0;
  uint32_t unk_4;

  uint32_t unk_8;
  vfs_node_unk_70_3* unk_C;

  //can be more bytes
} vfs_node_unk_70_2;

typedef struct vfs_node_unk_70_4
{
  void* unk_0;
  void* unk_4;

  void* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70_4;

typedef struct vfs_node_unk_70
{
  vfs_node_unk_70_1* unk_0;
  vfs_node_unk_70_2* unk_4;

  vfs_node_unk_70_4* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70;

typedef struct vfs_node
{
   uint32_t unk_0;
   uint32_t unk_4; //uid ?, not ptr
   uint32_t unk_8;
   uint32_t unk_C; //uid ?, not ptr

   uint32_t unk_10; //num
   uint32_t unk_14;
   uint32_t unk_18;
   uint32_t unk_1C;

   uint32_t unk_20;
   uint32_t unk_24;
   uint32_t unk_28;
   uint32_t unk_2C;

   uint32_t unk_30;
   uint32_t unk_34;
   uint32_t unk_38;
   uint32_t unk_3C;

   node_ops2 *ops;
   uint32_t unk_44;
   vfs_node_unk_48* unk_48; //ptr ?
   uint32_t unk_4C; // not a pointer

   vfs_node_unk_50* unk_50; //ptr
   vfs_node_unk_54* unk_54; //ptr
   uint32_t unk_58; //num
   uint32_t unk_5C;

   uint32_t unk_60;
   uint32_t unk_64;
   uint32_t unk_68;
   SceUID pool_uid;

   vfs_node_unk_70* unk_70; //ptr
   uint32_t unk_74; //num
   uint32_t unk_78; //num
   uint32_t unk_7C;

   uint32_t unk_80; //ptr ?
   uint32_t unk_84;
   uint32_t unk_88; //num
   uint32_t unk_8C;

   uint32_t unk_90; //num
   uint32_t unk_94; //num
   uint32_t unk_98;
   uint32_t unk_9C;

   uint32_t unk_A0;
   uint32_t unk_A4;
   uint32_t unk_A8;
   uint32_t unk_AC;

   uint32_t unk_B0;
   uint32_t unk_B4;
   uint32_t unk_B8;
   uint32_t unk_BC;

   uint32_t unk_C0;
   uint32_t unk_C4;
   uint32_t unk_C8;
   uint32_t unk_CC;

   uint32_t unk_D0; //num
}vfs_node;

typedef struct vnf1_arg1
{
  char* blockDevice;
  uint32_t nameLength;
  char* unixMount;
}vnf1_arg1;

typedef struct vfs_node_func1_args
{
   struct vfs_node* node;
   struct vnf1_arg1* arg1;
   uint32_t arg2;
   uint32_t arg3;
}vfs_node_func1_args;

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

typedef struct vnf4_arg2
{
   char* blockDeviceName;
   int nameLength;
} vnf4_arg2;

typedef struct vfs_node_func4_args
{
   vfs_node* node;
   int* arg1; //result
   vnf4_arg2* arg2; //mount
   uint32_t arg3;
}vfs_node_func4_args;

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

typedef struct vfs_node_func7_args
{
   struct vfs_node* node;
   void* arg1; //some ptr
   uint32_t arg2; //0x200 or 0 (size?)
   uint32_t arg3; 
   uint32_t arg_0; //0x1 or 0x0
}vfs_node_func7_args;

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

typedef struct vnf9_arg2
{
   char* blockDeviceName;
   int nameLength;

   //can be more bytes
}vnf9_arg2;

typedef struct vfs_node_func9_args
{
   struct vfs_node* node0;
   struct vfs_node* node1;
   struct vnf9_arg2* arg2;
   uint32_t arg3;
}vfs_node_func9_args;

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

//=====================================================

typedef struct node_ops1 // size is 0x34 (13 pointers)
{
  int (*func1)(void* ctx);
  int (*func2)(void* ctx); // ?
  int (*func3)(void* ctx);
  int (*func4)(void* ctx);
  int (*func5)(void* ctx); // not implemented by all
  int (*func6)(void* ctx); // not implemented by all
  int (*func7)(void* ctx);
  int (*func8)(void* ctx); // not implemented by all
  int (*func9)(void* ctx); // called by sceVfsAddVfs
  int (*func10)(void* ctx); // called by sceVfsDeleteVfs
  int (*func11)(void* ctx); // not implemented by all
  int (*func12)(void* ctx); // sceIoDevctlForDriver
  int (*func13)(void* ctx); // ?
} node_ops1;

typedef struct vfs_add_data
{
    struct node_ops1* funcs1;
    const char *name; //max size is 0x20
    int flags; //0x0E
    int unk_C; //0x01 / 0x00 (root, leaf ?)
    
    int unk_10; //0x10
    struct node_ops2* funcs2;
    int unk_18; //0x00
    struct vfs_add_data* next_element; //ptr to next element, confirmed
} vfs_add_data;

typedef struct vfs_block_dev_info //size is 0x14
{
  char* vitaMount;
  char* filesystem; // Some name, I guess it is filesystem
  char* blockDevicePrimary;
  char* blockDeviceSecondary; // can be 0
  uint32_t vshMountId; //must be same as in mount_point_info
}vfs_block_dev_info;

typedef struct vfs_mount_point_info_base
{
  char* unixMount;
  uint32_t unk_4; //zero
  uint32_t devMajor;
  uint32_t devMinor;

  char* filesystem;
  uint32_t unk_14; //zero
  struct vfs_block_dev_info* blockDev1;
  uint32_t unk_1C; //zero
} vfs_mount_point_info_base;

typedef struct vfs_unmount_data
{
    const char *mountpoint;
    int flags;
}vfs_unmount_data;

int sceVfsMount_hook(vfs_mount_point_info_base* data)
{
  int res = TAI_CONTINUE(int, sceVfsMount_hook_ref, data);

  /*
  char msg_buffer[MSG_SIZE];
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsMount");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsAddVfs_hook(vfs_add_data* data)
{
  int res = TAI_CONTINUE(int, sceVfsAddVfs_hook_ref, data);

  /*
  char msg_buffer[MSG_SIZE];
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsAddVfs");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsUnmount_hook(vfs_unmount_data* data)
{
  int res = TAI_CONTINUE(int, sceVfsUnmount_hook_ref, data);

  /*
  char msg_buffer[MSG_SIZE];
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsUnmount");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsDeleteVfs_hook(const char* name, void** deleted_node)
{
  int res = TAI_CONTINUE(int, sceVfsDeleteVfs_hook_ref, name, deleted_node);

  /*
  char msg_buffer[MSG_SIZE];
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsDeleteVfs");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsGetNewNode_hook(void* ctx, node_ops2* ops, int unused, vfs_node** node)
{
  int res = TAI_CONTINUE(int, sceVfsGetNewNode_hook_ref, ctx, ops, unused, node);

  /*
  char msg_buffer[MSG_SIZE];
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsGetNewNode");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

//=====================================================

typedef int(vfs_func)(void* ctx);

vfs_func* sdstor_dev_fs_functions[13] = {
                                         vfs_func1, 
                                         vfs_func3, 
                                         vfs_func12, 
                                         vfs_func13,
                                         vfs_node_func1, 
                                         vfs_node_func3, 
                                         vfs_node_func4, 
                                         vfs_node_func5, 
                                         vfs_node_func6, 
                                         vfs_node_func7, 
                                         vfs_node_func9, 
                                         vfs_node_func19, 
                                         vfs_node_func20
                                        };

uint32_t sdstor_dev_fs_function_offsets[13] = {
                                              0x3014,
                                              0x303C,
                                              0x35D0,
                                              0x3550,

                                              //looks like these functions are too close to each other ? can register only one of them?
                                              0x3464,
                                              0x3458,

                                              0x32E0,
                                              0x3230,
                                              0x317C,
                                              0x30C4,
                                              0x3290,
                                              0x31DC,
                                              0x3070,
                                              };

#define SceSblGcAuthMgrGcAuthForDriver_NID 0xC6627F5E
#define SceSdifForDriver_NID 0x96D306FA
#define SceSysrootForDriver_NID 0x2ED7F97A
#define SceIofilemgrForDriver_NID 0x40FD29C7

int initialize_all_hooks()
{
  // Get tai module info
  tai_module_info_t sdstor_info;
  sdstor_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &sdstor_info) >= 0) 
  {
      gc_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &gc_hook_ref, "SceSdstor", SceSblGcAuthMgrGcAuthForDriver_NID, 0x68781760, gc_patch);
      
      //init_mmc_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &init_mmc_hook_ref, "SceSdstor", SceSdifForDriver_NID, 0x22c82e79, init_mmc_hook);
      //init_sd_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &init_sd_hook_ref, "SceSdstor", SceSdifForDriver_NID, 0xc1271539, init_sd_hook);
      
      gen_init_hook_uids[0] = taiHookFunctionOffsetForKernel(KERNEL_PID, &gen_init_hook_refs[0], sdstor_info.modid, 0, 0x1E50, 1, gen_init_hook_1);
      gen_init_hook_uids[1] = taiHookFunctionOffsetForKernel(KERNEL_PID, &gen_init_hook_refs[1], sdstor_info.modid, 0, 0x22D0, 1, gen_init_hook_2);
      gen_init_hook_uids[2] = taiHookFunctionOffsetForKernel(KERNEL_PID, &gen_init_hook_refs[2], sdstor_info.modid, 0, 0x2708, 1, gen_init_hook_3);
      
      load_mbr_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &load_mbr_hook_ref, sdstor_info.modid, 0, 0xF2C, 1, load_mbr_hook); //0xC14F2C
      mnt_pnt_chk_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &mnt_pnt_chk_hook_ref, sdstor_info.modid, 0, 0x1B80, 1, mnt_pnt_chk_hook); //0xc15B80
      mbr_table_init_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &mbr_table_init_hook_ref, sdstor_info.modid, 0, 0x142C, 1, mbr_table_init_hook); //0xc1542C

      for(int f = 0; f < 13; f++)
      {
        sdstor_dev_fs_ids[f] = taiHookFunctionOffsetForKernel(KERNEL_PID, &sdstor_dev_fs_refs[f], sdstor_info.modid, 0, sdstor_dev_fs_function_offsets[f], 1, sdstor_dev_fs_functions[f]);
      }
  }
  
  tai_module_info_t sdif_info;
  sdif_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdif", &sdif_info) >= 0)
  {
    init_mmc_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &init_mmc_hook_ref, "SceSdif", SceSdifForDriver_NID, 0x22c82e79, init_mmc_hook);
    init_sd_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &init_sd_hook_ref, "SceSdif", SceSdifForDriver_NID, 0xc1271539, init_sd_hook);
    
    char iterations[1] = {20};
      
    patch_uids[0] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x349A, iterations, 1); //patch MOVS R2, 5 to MOVS R2, 10
    patch_uids[1] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x34B8, iterations, 1); //patch MOVS R2, 5 to MOVS R2, 10
    patch_uids[2] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x359A, iterations, 1); //patch MOVS R2, 5 to MOVS R2, 10
    
    cmd55_41_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &cmd55_41_hook_ref, sdif_info.modid, 0, 0x35E8, 1, cmd55_41_hook);
  }
  
  tai_module_info_t sysroot_info;
  sysroot_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSysmem", &sysroot_info) >= 0)
  {
    //sysroot_zero_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sysroot_zero_hook_ref, "SceSysmem", SceSysrootForDriver_NID, 0xf804f761, sysroot_zero_hook);
    
    //by some reason only import hook worked
    sysroot_zero_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sysroot_zero_hook_ref, "SceSdstor", SceSysrootForDriver_NID, 0xf804f761, sysroot_zero_hook);
  }

  tai_module_info_t iofilemgr_info;
  iofilemgr_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceIofilemgr", &iofilemgr_info) >= 0)
  {
    sceVfsMount_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsMount_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xB62DE9A6, sceVfsMount_hook);
    sceVfsAddVfs_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsAddVfs_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x673D2FCD, sceVfsAddVfs_hook);

    //by some reason this hook fails with TAI_ERROR_HOOK_ERROR
    sceVfsUnmount_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsUnmount_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x9C7E7B76, sceVfsUnmount_hook);

    sceVfsDeleteVfs_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsDeleteVfs_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x9CBFA725, sceVfsDeleteVfs_hook);
    sceVfsGetNewNode_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsGetNewNode_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xD60B5C63, sceVfsGetNewNode_hook);
  }
  
  open_global_log();
  if(gc_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set gc init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gc init hook: %x\n", gc_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(init_mmc_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mmc init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set mmc init hook: %x\n", init_mmc_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(init_sd_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sd init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sd init hook: %x\n", init_sd_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(cmd55_41_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set cmd55_41_hook_id hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set cmd55_41_hook_id hook: %x\n", cmd55_41_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(gen_init_hook_uids[0] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 1 hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gen init 1 hook: %x\n", gen_init_hook_uids[0]);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(gen_init_hook_uids[1] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 2 hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gen init 2 hook: %x\n", gen_init_hook_uids[1]);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(gen_init_hook_uids[2] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 3 hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gen init 3 hook: %x\n", gen_init_hook_uids[2]);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(sysroot_zero_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sysroot zero hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sysroot zero hook: %x\n", sysroot_zero_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(load_mbr_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set load mbr hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set load mbr hook: %x\n", load_mbr_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  if(mnt_pnt_chk_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mnt ptr chk hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set mnt ptr chk hook: %x\n", mnt_pnt_chk_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  if(mbr_table_init_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mbr table init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set mbr table init hook: %x\n", mbr_table_init_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  for(int f = 0; f < 13; f++)
  {
    if(sdstor_dev_fs_ids[f] >= 0)
    {
      char buffer[100];
      snprintf(buffer, 100, "set sdstor_dev_fs function %d hook\n", (f + 1));
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
    else
    {
      char buffer[100];
      snprintf(buffer, 100, "failed to set sdstor_dev_fs function %d hook: %x\n", (f + 1), sdstor_dev_fs_ids[f]);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
  }

  if(sceVfsMount_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsMount hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sceVfsMount hook: %x\n", sceVfsMount_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  if(sceVfsAddVfs_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsAddVfs hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sceVfsAddVfs hook: %x\n", sceVfsAddVfs_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  if(sceVfsUnmount_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsUnmount hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sceVfsUnmount hook: %x\n", sceVfsUnmount_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  if(sceVfsDeleteVfs_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsDeleteVfs hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sceVfsDeleteVfs hook: %x\n", sceVfsDeleteVfs_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(sceVfsGetNewNode_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsGetNewNode hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sceVfsGetNewNode hook: %x\n", sceVfsGetNewNode_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  close_global_log();
  
  return 0;
}

int deinitialize_all_hooks()
{
  if (gc_hook_id >= 0)
    taiHookReleaseForKernel(gc_hook_id, gc_hook_ref);
  
  if(init_mmc_hook_id >= 0)
    taiHookReleaseForKernel(init_mmc_hook_id, init_mmc_hook_ref);
  
  if(init_sd_hook_id >= 0)
    taiHookReleaseForKernel(init_sd_hook_id, init_sd_hook_ref);

  if (patch_uids[0] >= 0)
    taiInjectReleaseForKernel(patch_uids[0]);
  
  if (patch_uids[1] >= 0)
    taiInjectReleaseForKernel(patch_uids[1]);
  
  if (patch_uids[2] >= 0)
    taiInjectReleaseForKernel(patch_uids[2]);
  
  if (cmd55_41_hook_id >= 0)
    taiHookReleaseForKernel(cmd55_41_hook_id, cmd55_41_hook_ref);
  
  if(gen_init_hook_uids[0] >= 0)
    taiHookReleaseForKernel(gen_init_hook_uids[0], gen_init_hook_refs[0]);
    
  if(gen_init_hook_uids[1] >= 0)
    taiHookReleaseForKernel(gen_init_hook_uids[1], gen_init_hook_refs[1]);
    
  if(gen_init_hook_uids[2] >= 0)
    taiHookReleaseForKernel(gen_init_hook_uids[2], gen_init_hook_refs[2]);
  
  if(load_mbr_hook_id >= 0)
    taiHookReleaseForKernel(load_mbr_hook_id, load_mbr_hook_ref);

  if(mnt_pnt_chk_hook_id >= 0)
    taiHookReleaseForKernel(mnt_pnt_chk_hook_id, mnt_pnt_chk_hook_ref);

  if(mbr_table_init_hook_id >= 0)
    taiHookReleaseForKernel(mbr_table_init_hook_id, mbr_table_init_hook_ref);

  if(sysroot_zero_hook_id >= 0)
    taiHookReleaseForKernel(sysroot_zero_hook_id, sysroot_zero_hook_ref);

  for(int f = 0; f < 13; f++)
  {
    if(sdstor_dev_fs_ids[f] >= 0)
      taiHookReleaseForKernel(sdstor_dev_fs_ids[f], sdstor_dev_fs_refs[f]);
  }
  
  if(sceVfsMount_hook_id >= 0)
    taiHookReleaseForKernel(sceVfsMount_hook_id, sceVfsMount_hook_ref);

  if(sceVfsAddVfs_hook_id >= 0)
    taiHookReleaseForKernel(sceVfsAddVfs_hook_id, sceVfsAddVfs_hook_ref);

  if(sceVfsUnmount_hook_id >= 0)
    taiHookReleaseForKernel(sceVfsUnmount_hook_id, sceVfsUnmount_hook_ref);
  
  if(sceVfsDeleteVfs_hook_id >= 0)
    taiHookReleaseForKernel(sceVfsDeleteVfs_hook_id, sceVfsDeleteVfs_hook_ref);

  if(sceVfsGetNewNode_hook_id >= 0)
    taiHookReleaseForKernel(sceVfsGetNewNode_hook_id, sceVfsGetNewNode_hook_ref);

  return 0;
}

int print_ctx(int sd_index)
{
  sd_context_global* gctx = ksceSdifGetSdContextGlobal(sd_index);
  uint32_t idx = gctx->ctx_data.dev_type_idx;
  sd_context_part* pctx = gctx->ctx_data.ctx;
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "type:%x ctx:%x\n", idx, pctx);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return 0;
}

int initialize_gc_sd()
{
  sd_context_part* ctx_00BDCBC0 = ksceSdifGetSdContextPartSd(SCE_SDIF_DEV_GAME_CARD);
  if(ctx_00BDCBC0 == 0)
  {
    int res = ksceSdifInitializeSdContextPartSd(SCE_SDIF_DEV_GAME_CARD, &ctx_00BDCBC0);
    
    open_global_log();
    {
      char buffer[100];
      snprintf(buffer, 100, "res:%x \n", res);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
    close_global_log();
  }
  
  //print_ctx(SCE_SDIF_DEV_EMMC);
  print_ctx(SCE_SDIF_DEV_GAME_CARD);
  //print_ctx(SCE_SDIF_DEV_WLAN_BT);
  
  return 0;
}
