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

#include "hooks_misc.h"

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

SceUID gen_init_2_patch_uid; // patch of zero func in gen_init_2 function

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
  FILE_WRITE(global_log_fd, "------ mmc -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(sprintfBuffer, 256, "idx:%x ctx:%x\n", last_mmc_inits[i].sd_ctx_index, last_mmc_inits[i].ctx);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  FILE_WRITE(global_log_fd, "------ sd  -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(sprintfBuffer, 256, "idx:%x ctx:%x\n", last_sd_inits[i].sd_ctx_index, last_sd_inits[i].ctx);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  
  return res;
}

int sysroot_zero_hook()
{
  int res = TAI_CONTINUE(int, sysroot_zero_hook_ref);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "------------------------\n");
    FILE_WRITE(global_log_fd, "called sysroot_zero_hook\n");
    FILE_WRITE(global_log_fd, "------------------------\n");
  }
  close_global_log();
  
  /*
  stacktrace_from_here_global("SceSdstor", 0, 110, 1);
  print_current_thread_info_global();
  */

  open_global_log();
  FILE_WRITE(global_log_fd, "------------------------\n");
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
      snprintf(sprintfBuffer, 256, "called mnt_pnt_chk_hook: %s %08x %08x res: %08x\n", blockDeviceName, mountNum, *mountData, res);
      FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
      snprintf(sprintfBuffer, 256, "called mbr_table_init_hook: %s %08x res: %08x\n", blockDeviceName, mountNum, res);
      FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    if(readCnt < 5) //limit size of log
    {
      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "called gen_read_hook idx:%x type:%x res:%x\n", idx, type, res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();
    }
    readCnt++;
  }

  return res;
}

//temporary fake untill I figure out what is wrong with sd card
int8_t sdCardMbr[0x200] = {
  0x53, 0x6F, 0x6E, 0x79, 0x20, 0x43, 0x6F, 0x6D, 0x70, 0x75, 0x74, 0x65, 0x72, 0x20, 0x45, 0x6E, 
  0x74, 0x65, 0x72, 0x74, 0x61, 0x69, 0x6E, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x49, 0x6E, 0x63, 0x2E, 
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x40, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x6F, 0x00, 0x09, 0x07, 0x01, 0x55, 0x05, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0D, 0xDA, 0x01, 0xFF, 0x0F, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA 
  };

int sdReadCnt = 0;

int sd_read_hook(sd_context_part* ctx, int sector, char* buffer, int nSectors)
{
  int res = TAI_CONTINUE(int, sd_read_hook_ref, ctx, sector, buffer, nSectors);

  if(sector == 0 && nSectors == 1)
  {
    memcpy(buffer, sdCardMbr, 0x200);
    res = 0;
  }

  if(sdReadCnt < 5) //limit size of log
  {
      open_global_log();
      {
        if(sector == 0)
        {
          snprintf(sprintfBuffer, 256, "called sd_read_hook sector: %x nSectors: %x mbr:%s res: %x\n", sector, nSectors, buffer, res);
          FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
        }
        else
        {
          snprintf(sprintfBuffer, 256, "called sd_read_hook sector: %x nSectors: %x res: %x\n", sector, nSectors, res);
          FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
        }
      }
      close_global_log();
  }

  sdReadCnt++;

  return res;
}

int init_partition_table_hook(int arg0, sdstor_mbr_ctx* data)
{
  int res = TAI_CONTINUE(int, init_partition_table_hook_ref, arg0, data);

  open_global_log();
  {  
    snprintf(sprintfBuffer, 256, "called init_partition_table_hook arg0: %x mbr_ctx: %x res: %x\n", arg0, data, res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();

  return res;
}