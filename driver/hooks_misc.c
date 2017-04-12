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

#include "glog.h"
#include "sdstor_log.h"
#include "dump.h"
#include "thread_test.h"
#include "net.h"
#include "mtable.h"
#include "stacktrace.h"

#include "hooks_misc.h"

#include "defines.h"

char sprintfBuffer[256];

//========================================

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

SceUID gen_init_1_patch_uid = -1; // patch of zero func in gen_init_1 function
SceUID gen_init_2_patch_uid = -1; // patch of zero func in gen_init_2 function

tai_hook_ref_t gen_read_hook_ref;
SceUID gen_read_hook_id = -1;

tai_hook_ref_t sd_read_hook_ref;
SceUID sd_read_hook_id = -1;

SceUID hs_dis_patch1_uid = -1; //high speed disable patch 1
SceUID hs_dis_patch2_uid = -1; //high speed disable patch 2

SceUID bus_size_patch_uid = -1;

SceUID mbr_init_zero_patch1_uid = -1;
SceUID mbr_init_zero_patch2_uid = -1;

tai_hook_ref_t init_partition_table_hook_ref;
SceUID init_partition_table_hook_id = -1;

tai_hook_ref_t create_device_handle_hook_ref;
SceUID create_device_handle_hook_id = -1;

tai_hook_ref_t send_command_hook_ref;
SceUID send_command_hook_id = -1;

tai_hook_ref_t sceErrorHistoryPostError_hook_ref;
SceUID sceErrorHistoryPostError_hook_id = -1;

tai_hook_ref_t sceErrorHistoryUpdateSequenceInfo_hook_ref;
SceUID sceErrorHistoryUpdateSequenceInfo_hook_id = -1;

tai_hook_ref_t sceErrorGetExternalString_kernel_hook_ref;
SceUID sceErrorGetExternalString_kernel_hook_id = -1;

tai_hook_ref_t sceErrorHistoryGetError_hook_ref;
SceUID sceErrorHistoryGetError_hook_id = -1;

tai_hook_ref_t ksceKernelCreateThread_hook_ref;
SceUID ksceKernelCreateThread_hook_id = -1;

tai_hook_ref_t sceKernelCreateThreadForUser_hook_ref;
SceUID sceKernelCreateThreadForUser_hook_id = -1;

tai_hook_ref_t sceIoOpenForDriver_hook_ref;
SceUID sceIoOpenForDriver_hook_id = -1;

tai_hook_ref_t ksceKernelWaitSema_hook_ref;
SceUID ksceKernelWaitSema_hook_id = -1;

tai_hook_ref_t ksceKernelSignalSema_hook_ref;
SceUID ksceKernelSignalSema_hook_id = -1;

tai_hook_ref_t vshSblAuthMgrVerifySpsfo_hook_ref;
SceUID vshSblAuthMgrVerifySpsfo_hook_id = -1;

tai_hook_ref_t debug_printf_callback_invoke_ref;
SceUID debug_printf_callback_invoke_id = -1;

tai_hook_ref_t gc_22fd5d23_hook_ref;
SceUID gc_22fd5d23_hook_id = -1;

tai_hook_ref_t appmgr_23D642C_hook_ref;
SceUID appmgr_23D642C_hook_id = -1;

tai_hook_ref_t sceAppMgrGameDataMountForDriver_hook_ref;
SceUID sceAppMgrGameDataMountForDriver_hook_id = -1;

//========================================

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
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("------ mmc -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(sprintfBuffer, 256, "idx:%x ctx:%x\n", last_mmc_inits[i].sd_ctx_index, last_mmc_inits[i].ctx);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  FILE_GLOBAL_WRITE_LEN("------ sd  -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(sprintfBuffer, 256, "idx:%x ctx:%x\n", last_sd_inits[i].sd_ctx_index, last_sd_inits[i].ctx);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();
  
  return 0;
}

//========================================

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
    snprintf(sprintfBuffer, 256, "call gc auth res:%x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
    snprintf(sprintfBuffer, 256, "init mmc - idx:%x ctx:%x res:%x\n", sd_ctx_index, *result, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
    snprintf(sprintfBuffer, 256, "init sd - idx:%x ctx:%x res:%x\n", sd_ctx_index, *result, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
    snprintf(sprintfBuffer, 256, "res cmd55_41:%x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
    snprintf(sprintfBuffer, 256, "called gen_init_hook_1 res:%x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();
  
  return res;
}

int gen_init_hook_2(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[1], ctx);
  
  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "called gen_init_hook_2 res:%x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();
  
  return res;
}

int gen_init_hook_3(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[2], ctx);
  
  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "called gen_init_hook_3 res:%x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();
  
  return res;
}

int sysroot_zero_hook()
{
  int res = TAI_CONTINUE(int, sysroot_zero_hook_ref);
  
  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("------------------------\n");
    FILE_GLOBAL_WRITE_LEN("called sysroot_zero_hook\n");
    FILE_GLOBAL_WRITE_LEN("------------------------\n");
  }
  close_global_log();
  
  /*
  stacktrace_from_here_global("SceSdstor", 0, 110, 1);
  print_current_thread_info_global();
  */

  open_global_log();
  FILE_GLOBAL_WRITE_LEN("------------------------\n");
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
    snprintf(sprintfBuffer, 256, "called load_mbr_hook: %x res: %x\n", ctx_index, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
      FILE_GLOBAL_WRITE_LEN("called mnt_pnt_chk_hook: data is invalid\n");
    }
    else
    {
      snprintf(sprintfBuffer, 256, "called mnt_pnt_chk_hook: %s %08x %08x res: %08x\n", blockDeviceName, mountNum, *mountData, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
      FILE_GLOBAL_WRITE_LEN("called mbr_table_init_hook: data is invalid\n");
    }
    else
    {
      snprintf(sprintfBuffer, 256, "called mbr_table_init_hook: %s %08x res: %08x\n", blockDeviceName, mountNum, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
  }
  close_global_log();

  return res;
} 

int readCnt = 0;

int gen_read_hook(char *ctx, char *buffer, int sector, int nSectors)
{
  int res = TAI_CONTINUE(int, gen_read_hook_ref, ctx, buffer, sector, nSectors);
  
  char idx = *(ctx + 0x40);
  char type = *(ctx + 0x41);

  if(idx == 1)
  {
    //if(readCnt < 5) //limit size of log
    //{
      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "called gen_read_hook idx:%x type:%x sector:%x nSectors: %x res:%x\n", idx, type, sector, nSectors, res);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      close_global_log();
    //}
    readCnt++;
  }

  return res;
}

int sdReadCnt = 0;

int sd_read_hook(sd_context_part* ctx, int sector, char* buffer, int nSectors)
{
  int res = TAI_CONTINUE(int, sd_read_hook_ref, ctx, sector, buffer, nSectors);
  
  //fake_read(sector, buffer, nSectors, &res);

  #ifdef ENABLE_SD_PATCHES
  if(nSectors == 1 && sector == 0)
  {
    tai_module_info_t m_info;
    m_info.size = sizeof(tai_module_info_t);
    if(taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &m_info) >= 0) 
    {
      uintptr_t addr = 0;
      int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 1, 0x1CC0, &addr);
      if(ofstRes == 0)
      {
        memcpy((char*)addr, buffer, 0x200); //copy MBR
      }
    }
  }

  if(ksceSdifGetSdContextGlobal(SCE_SDIF_DEV_GAME_CARD) == ctx->gctx_ptr)
  {
    /*
    if(nSectors > 1)
    {
      print_bytes(buffer, 0x200);
    }
    */
  }
  #endif

  /*
  if(nSectors == 1)
  {
    switch(sector)
    {
      case 0:
      {
        print_bytes(buffer, 0x200);
      }
      break;
      case 0x40:
      {
        print_bytes(buffer, 0x200);
      }
      break;
    }
  }
  */

  //if(sdReadCnt < 5) //limit size of log
  //{
      open_global_log();
      {
        if(sector == 0)
        {
          //snprintf(sprintfBuffer, 256, "called sd_read_hook sector: %x nSectors: %x mbr:%s res: %x\n", sector, nSectors, buffer, res);
          snprintf(sprintfBuffer, 256, "called sd_read_hook sector: %x nSectors: %x res: %x\n", sector, nSectors, res);
          FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        }
        else
        {
          snprintf(sprintfBuffer, 256, "called sd_read_hook sector: %x nSectors: %x res: %x\n", sector, nSectors, res);
          FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        }
      }
      close_global_log();
  //}

  sdReadCnt++;

  return res;
}

int init_partition_table_hook(int arg0, sdstor_mbr_ctx* data)
{
  int res = TAI_CONTINUE(int, init_partition_table_hook_ref, arg0, data);

  open_global_log();
  {  
    snprintf(sprintfBuffer, 256, "called init_partition_table_hook arg0: %x mbr_ctx: %x res: %x\n", arg0, data, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();

  return res;
}

int create_device_handle(partition_entry* pentry, int unk1, sd_stor_device_handle** handle)
{
  int res = TAI_CONTINUE(int, create_device_handle_hook_ref, pentry, unk1, handle);

  open_global_log();
  {  
    snprintf(sprintfBuffer, 256, "called create_device_handle partition: %s unk1: %x handle: %x res: %x\n", pentry->numericName, unk1, handle, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();

  return res;
}

//by some unknown reason real sectors on the card start from 0x8000
//TODO: I need to figure out this later
#define ADDRESS_OFFSET 0x8000

int send_command_hook(sd_context_global* ctx, cmd_input* cmd_data1, cmd_input* cmd_data2, int nIter, int num)
{
  #ifdef ENABLE_SD_PATCHES
  if(ksceSdifGetSdContextGlobal(SCE_SDIF_DEV_GAME_CARD) == ctx)
  {
    if(cmd_data1->command == 17 || cmd_data1->command == 18)
    {
      cmd_data1->argument = cmd_data1->argument + ADDRESS_OFFSET; //fixup address. I have no idea why I should do it
    }
  }
  #endif

  int res = TAI_CONTINUE(int, send_command_hook_ref, ctx, cmd_data1, cmd_data2, nIter, num);
 
  //log ONLY game cart becase may get conflict in other case (speculation)
  //anyway we only need game card commands
  if(ksceSdifGetSdContextGlobal(SCE_SDIF_DEV_GAME_CARD) == ctx)
  {
    open_global_log();
    {
      if(cmd_data2 == 0)
      {
        snprintf(sprintfBuffer, 256, "called send_command_hook: CMD%d %08x res: %08x\n", cmd_data1->command, cmd_data1->argument, res);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      else
      {
        snprintf(sprintfBuffer, 256, "called send_command_hook: CMD%d %08x CMD%d %08x res: %08x\n", cmd_data1->command, cmd_data1->argument, cmd_data2->command, cmd_data2->argument, res);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
    }
    close_global_log();
  }

  return res;
}

int sceErrorHistoryPostError_hook(void* src_user)
{
  int res = TAI_CONTINUE(int, sceErrorHistoryPostError_hook_ref, src_user);

  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "called sceErrorHistoryPostError_hook: src_user: %08x res: %08x\n", src_user, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();

  return res;
}

int sceErrorHistoryUpdateSequenceInfo_hook(void* src_user, int unk1)
{
  int res = TAI_CONTINUE(int, sceErrorHistoryUpdateSequenceInfo_hook_ref, src_user, unk1);

  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "called sceErrorHistoryUpdateSequenceInfo_hook: src_user: %08x unk1: %08x res: %08x\n", src_user, unk1, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();

  return res;
}

int sceErrorGetExternalString_kernel_hook(void* dest_user, int unk)
{
  int res = TAI_CONTINUE(int, sceErrorGetExternalString_kernel_hook_ref, dest_user, unk);

  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "called sceErrorGetExternalString_kernel_hook: dest_user: %08x unk: %08x res: %08x\n", dest_user, unk, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();

  stacktrace_from_here_global("", 0, 100, 1);
  print_current_thread_info_global();

  return res;
}

int sceErrorHistoryGetError_hook(int unk0, void* dest_user)
{
  int res = TAI_CONTINUE(int, sceErrorHistoryGetError_hook_ref, unk0, dest_user);

  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "called sceErrorHistoryGetError: dest_user: %08x unk0: %08x res: %08x\n", dest_user, unk0, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();

  return res;
}

int ksceKernelCreateThread_hook_entered = 0;

int ksceKernelCreateThread_hook(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, int cpuAffinityMask, const SceKernelThreadOptParam *option)
{
  int res = TAI_CONTINUE(int, ksceKernelCreateThread_hook_ref, name, entry, initPriority, stackSize, attr, cpuAffinityMask, option);

  /*
  if(ksceKernelCreateThread_hook_entered == 0)
  {
    ksceKernelCreateThread_hook_entered = 1;

    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      snprintf(sprintfBuffer, 256, "called ksceKernelCreateThread: %s res: %08x\n", name, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      FILE_GLOBAL_WRITE_LEN("======================================\n");
    }
    close_global_log();

    ksceKernelCreateThread_hook_entered = 0;
  }
  */

  return res;
}

char g_threadName[256];

int sceKernelCreateThreadForUser_hook_entered = 0; 

int sceKernelCreateThreadForUser_hook(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, int cpuAffinityMask, const SceKernelThreadOptParam *option)
{
  int res = TAI_CONTINUE(int, sceKernelCreateThreadForUser_hook_ref, name, entry, initPriority, stackSize, attr, cpuAffinityMask, option);

  if(sceKernelCreateThreadForUser_hook_entered == 0)
  {
    sceKernelCreateThreadForUser_hook_entered = 1;

    /*
    open_global_log();
    {
      ksceKernelStrncpyUserToKernel(g_threadName, (uintptr_t)name, 256);

      FILE_GLOBAL_WRITE_LEN("======================================\n");
      snprintf(sprintfBuffer, 256, "called sceKernelCreateThreadForUser: %s res: %08x\n", g_threadName, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      FILE_GLOBAL_WRITE_LEN("======================================\n");
    }
    close_global_log();
    */

    sceKernelCreateThreadForUser_hook_entered = 0;
  }

  return res;
}

char g_fileName[1024];

int sceIoOpenForDriver_hook_entered = 0;

SceUID sceIoOpenForDriver_hook(const char *file, int flags, SceMode mode, void *args)
{
  //temporary disable this since log is very heavy

  #ifdef ENABLE_IO_FILE_OPEN_LOG
  if(sceIoOpenForDriver_hook_entered == 0)
  {
    sceIoOpenForDriver_hook_entered = 1;

    open_global_log();
    {
      ksceKernelStrncpyUserToKernel(g_fileName, (uintptr_t)file, 1024);

      FILE_GLOBAL_WRITE_LEN("======================================\n");
      snprintf(sprintfBuffer, 256, "called sceIoOpenForDriver: %s\n", g_fileName);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    close_global_log();

    SceKernelThreadInfo t_info;
    get_current_thread_info(&t_info);

    open_global_log();
    {
      snprintf(sprintfBuffer, 256, "process: %08x thread: %s\nentry: %08x stack: %08x stackSize: %08x\n", t_info.processId, t_info.name, t_info.entry, t_info.stack, t_info.stackSize);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    close_global_log();
    
    int segidx = find_in_segments(g_segListUser, SEG_LIST_SIZE, &moduleListIsConstructedUser, (uintptr_t)t_info.entry);
    if(segidx >= 0)
    {
      open_global_log();
      snprintf(sprintfBuffer, 256, "name: %s base: %08x seg: %d ofst: %08x\n", g_segListUser[segidx].moduleName, g_segListUser[segidx].range.start, g_segListUser[segidx].seg, (uintptr_t)t_info.entry - g_segListUser[segidx].range.start);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      close_global_log();
    }
    else
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("entry not found\n");
      close_global_log();
    }

    open_global_log();
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    close_global_log();
    
    sceIoOpenForDriver_hook_entered = 0;
  }
  #endif

  SceUID res = TAI_CONTINUE(SceUID, sceIoOpenForDriver_hook_ref, file, flags, mode, args);

  return res;
}

int ksceKernelWaitSema_counter = 0;

SceUID SceGameGcPromoteJobQueue_semaid = -1;

int ksceKernelWaitSema_hook(SceUID a_semaid, int a_signal, SceUInt *a_timeout)
{
  int res = TAI_CONTINUE(int, ksceKernelWaitSema_hook_ref, a_semaid, a_signal, a_timeout);

  SceKernelThreadInfo t_info;
  get_current_thread_info(&t_info);

  if(strcmp(t_info.name, "SceGameGcPromoteJobQueue") == 0)
  {
    SceGameGcPromoteJobQueue_semaid = a_semaid;
  }

  /*
  if(strcmp(t_info.name, "SceGameGcPromoteJobQueue") == 0)
  {
    if(ksceKernelWaitSema_counter < 1) //limit log
    {
      open_global_log();
      {
        FILE_GLOBAL_WRITE_LEN("======================================\n");
        snprintf(sprintfBuffer, 256, "called ksceKernelWaitSema: %08x\n", a_semaid);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      close_global_log();

      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "process: %08x thread: %s\nentry: %08x stack: %08x stackSize: %08x\n", t_info.processId, t_info.name, t_info.entry, t_info.stack, t_info.stackSize);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      close_global_log();

      open_global_log();
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      close_global_log();

      ksceKernelWaitSema_counter++;
    }
  }
  */

  return res;
}

int ksceKernelSignalSema_counter = 0;

int ksceKernelSignalSema_hook(SceUID a_semaid, int a_signal)
{
  int res = TAI_CONTINUE(int, ksceKernelSignalSema_hook_ref, a_semaid, a_signal);

  if(SceGameGcPromoteJobQueue_semaid != -1 && SceGameGcPromoteJobQueue_semaid == a_semaid)
  {
    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      snprintf(sprintfBuffer, 256, "called ksceKernelSignalSema: %08x\n", a_semaid);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    close_global_log();

    print_current_thread_info_global();

    open_global_log();
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    close_global_log();
  }

  /*
  if(ksceKernelSignalSema_counter < 5)
  {
    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      snprintf(sprintfBuffer, 256, "called ksceKernelSignalSema: %08x\n", a_semaid);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    close_global_log();

    print_current_thread_info_global();

    open_global_log();
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    close_global_log();

    ksceKernelSignalSema_counter++;
  }
  */

  return res;
}

char g_spsfo_path[1024];

char g_spsfo_dest[1024];

int vshSblAuthMgrVerifySpsfo_hook(char *path_user, char *dest_user, int maxSize, spsfo_opt *opt)
{
  int res = TAI_CONTINUE(int, vshSblAuthMgrVerifySpsfo_hook_ref, path_user, dest_user, maxSize, opt);

  open_global_log();
  {
    ksceKernelStrncpyUserToKernel(g_spsfo_path, (uintptr_t)path_user, 1024);

    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called vshSblAuthMgrVerifySpsfo:\npath: %s size: %08x res: %08x\n", g_spsfo_path, maxSize, res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log();

  //ksceKernelMemcpyUserToKernel(g_spsfo_dest, (uintptr_t)dest_user, maxSize < 1024 ? maxSize : 1024);
  //print_bytes(g_spsfo_dest, maxSize);

  return res;
}

int debug_printf_callback_invoke_hook_called = 0;

int debug_printf_callback_invoke_hook(int unk0, int unk1, int unk2)
{
  if(debug_printf_callback_invoke_hook_called == 0)
  {
    debug_printf_callback_invoke_hook_called = 1;

    //send_message_to_client("prinf", sizeof("prinf"));

    /*
    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      snprintf(sprintfBuffer, 256, "called debug_printf_callback_invoke:\narg0: %08x arg1: %08x arg2: %08x res: %08x\n", unk0, unk1, unk2, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      FILE_GLOBAL_WRITE_LEN("======================================\n");
    }
    close_global_log();
    */

    debug_printf_callback_invoke_hook_called = 0;
  }

  int res = TAI_CONTINUE(int, debug_printf_callback_invoke_ref, unk0, unk1, unk2);

  return res;
}

//this is a hook for SceNpDrm.SceSblGcAuthMgrDrmBBForDriver._imp_22fd5d23
//it does some checksum check on last responce of cmd56 custom initialization
//when error happens - returns 0x8087000A

int gc_22fd5d23_hook(char* buffer)
{
  /*
  tai_module_info_t gc_auth_info;
  gc_auth_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSblGcAuthMgr", &gc_auth_info) >= 0)
  {
    //does not work
    //directly into data segment
    
    //uintptr_t buffer_BDD038_adr = 0;
    //int ofstRes = module_get_offset(KERNEL_PID, gc_auth_info.modid, 1, 0x5038, &buffer_BDD038_adr);
    //if(ofstRes >= 0)
    //{
    //  if(buffer > 0)
    //  {
    //    memcpy((void*)buffer_BDD038_adr, buffer, 0x14);
    //  }
    //}
    
    //does not work
    //derive address from code segment constant
    //uintptr_t dword_CA9144 = 0;
    //int ofstRes = module_get_offset(KERNEL_PID, gc_auth_info.modid, 0, 0x9144, &dword_CA9144);
    //if(ofstRes >= 0)
    //{
    //  if(dword_CA9144 > 0)
    //  {
    //    void* destBuffer = *((void**)(dword_CA9144));
    //    if(destBuffer > 0)
    //    {
    //      if(buffer > 0)
    //      {
    //        memcpy(destBuffer, buffer, 0x14);
    //      }
    //    }
    //  }
    //}
  }
  */

  int res = TAI_CONTINUE(int, gc_22fd5d23_hook_ref, buffer);

  #ifdef ENABLE_SD_PATCHES
  return 0; //force return success
  #else
  return res; 
  #endif
}

int appmgr_23D642C_hook(int unk0, int unk1)
{
  int res = TAI_CONTINUE(int, appmgr_23D642C_hook_ref, unk0, unk1);
 
  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called appmgr_23D642C_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log();  

  return res;
}

int sceAppMgrGameDataMountForDriver_hook(int unk0, int unk1, int unk2, int unk3)
{
  int res = TAI_CONTINUE(int, sceAppMgrGameDataMountForDriver_hook_ref, unk0, unk1, unk2, unk3);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called sceAppMgrGameDataMountForDriver_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log();  

  return res;
}