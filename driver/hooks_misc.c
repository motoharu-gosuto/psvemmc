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

tai_hook_ref_t appmgr_23D9B50_hook_ref;
SceUID appmgr_23D9B50_hook_id = -1;

tai_hook_ref_t sceAppMgrGameDataVfsMountForDriver_hook_ref;
SceUID sceAppMgrGameDataVfsMountForDriver_hook_id = -1;

tai_hook_ref_t sceFiosKernelOverlayRemoveForProcessForDriver_hook_ref;
SceUID sceFiosKernelOverlayRemoveForProcessForDriver_hook_id = -1;

tai_hook_ref_t proc_read_sealedkey_23D6EA0_hook_ref;
SceUID proc_read_sealedkey_23D6EA0_hook_id = -1;

tai_hook_ref_t sbl_acmgr_0b6e6cd7_hook_ref;
SceUID sbl_acmgr_0b6e6cd7_hook_id = -1;

tai_hook_ref_t proc_generate_random_path_23D4FBC_hook_ref;
SceUID proc_generate_random_path_23D4FBC_hook_id = -1;

tai_hook_ref_t proc_get_clearsign_stat_23D9A4C_hook_ref;
SceUID proc_get_clearsign_stat_23D9A4C_hook_id = -1;

tai_hook_ref_t pfs_mgr_a772209c_hook_ref;
SceUID pfs_mgr_a772209c_hook_id = -1;

tai_hook_ref_t pfs_mgr_2d48aea2_hook_ref;
SceUID pfs_mgr_2d48aea2_hook_id = -1;

tai_hook_ref_t pfs_mgr_2190AEC_hook_ref;
SceUID pfs_mgr_2190AEC_hook_id = -1;

tai_hook_ref_t iofilemgr_feee44a9_hook_ref;
SceUID iofilemgr_feee44a9_hook_id = -1;

tai_hook_ref_t iofilemgr_d220539d_hook_ref;
SceUID iofilemgr_d220539d_hook_id = -1;

tai_hook_ref_t iofilemgr_BF3848_hook_ref;
SceUID iofilemgr_BF3848_hook_id = -1;

tai_hook_ref_t iofilemgr_BF3474_hook_ref;
SceUID iofilemgr_BF3474_hook_id = -1;

tai_hook_ref_t iofilemgr_sceVfsMount_hook_ref;
SceUID iofilemgr_sceVfsMount_hook_id = -1;

tai_hook_ref_t iofilemgr_sceVfsGetNewNode_hook_ref;
SceUID iofilemgr_sceVfsGetNewNode_hook_id = -1;

tai_hook_ref_t iofilemgr_BF1AF0_hook_ref;
SceUID iofilemgr_BF1AF0_hook_id = -1;

tai_hook_ref_t iofilemgr_BE5B30_hook_ref;
SceUID iofilemgr_BE5B30_hook_id = -1;

tai_hook_ref_t iofilemgr_BF18CC_hook_ref;
SceUID iofilemgr_BF18CC_hook_id = - 1;

tai_hook_ref_t iofilemgr_BEDEB0_hook_ref;
SceUID iofilemgr_BEDEB0_hook_id = -1;

tai_hook_ref_t iofilemgr_BE62E8_hook_ref;
SceUID iofilemgr_BE62E8_hook_id = -1;

tai_hook_ref_t iofilemgr_BE5F10_hook_ref;
SceUID iofilemgr_BE5F10_hook_id = -1;

tai_hook_ref_t iofilemgr_BE6788_hook_ref;
SceUID iofilemgr_BE6788_hook_id = -1;

tai_hook_ref_t iofilemgr_BEE3C8_hook_ref;
SceUID iofilemgr_BEE3C8_hook_id = -1;

tai_hook_ref_t iofilemgr_BEBAC0_hook_ref;
SceUID iofilemgr_BEBAC0_hook_id = -1;

tai_hook_ref_t iofilemgr_BE61C4_hook_ref;
SceUID iofilemgr_BE61C4_hook_id = -1;

tai_hook_ref_t iofilemgr_BE59BC_hook_ref;
SceUID iofilemgr_BE59BC_hook_id = -1;

//------------------------------

tai_hook_ref_t iofilemgr_BEE364_hook_ref;
SceUID iofilemgr_BEE364_hook_id = -1;

tai_hook_ref_t iofilemgr_BEE2D4_hook_ref;
SceUID iofilemgr_BEE2D4_hook_id = -1;

tai_hook_ref_t iofilemgr_BEE2C4_hook_ref;
SceUID iofilemgr_BEE2C4_hook_id = -1;

tai_hook_ref_t iofilemgr_BEDF5C_hook_ref;
SceUID iofilemgr_BEDF5C_hook_id = -1;

tai_hook_ref_t iofilemgr_BECE80_hook_ref;
SceUID iofilemgr_BECE80_hook_id = -1;

tai_hook_ref_t iofilemgr_BEC808_hook_ref;
SceUID iofilemgr_BEC808_hook_id = -1;

tai_hook_ref_t iofilemgr_BEC620_hook_ref;
SceUID iofilemgr_BEC620_hook_id = -1;

tai_hook_ref_t iofilemgr_BEC578_hook_ref;
SceUID iofilemgr_BEC578_hook_id = -1;

tai_hook_ref_t iofilemgr_BEC56C_hook_ref;
SceUID iofilemgr_BEC56C_hook_id = -1;

tai_hook_ref_t iofilemgr_BEC530_hook_ref;
SceUID iofilemgr_BEC530_hook_id = -1;

tai_hook_ref_t iofilemgr_BEC010_hook_ref;
SceUID iofilemgr_BEC010_hook_id = -1;

tai_hook_ref_t iofilemgr_BE5A38_hook_ref;
SceUID iofilemgr_BE5A38_hook_id = -1;

tai_hook_ref_t iofilemgr_BE5814_hook_ref;
SceUID iofilemgr_BE5814_hook_id = -1;

tai_hook_ref_t iofilemgr_BEBB84_hook_ref;
SceUID iofilemgr_BEBB84_hook_id = -1;

tai_hook_ref_t iofilemgr_BEC7C0_hook_ref;
SceUID iofilemgr_BEC7C0_hook_id = -1;

tai_hook_ref_t iofilemgr_BECE0C_hook_ref;
SceUID iofilemgr_BECE0C_hook_id = -1;

//-----------------

tai_hook_ref_t iofilemgr_exp_aa45010b_hook_ref;
SceUID iofilemgr_exp_aa45010b_hook_id = -1;

tai_hook_ref_t iofilemgr_exp_21d57633_hook_ref;
SceUID iofilemgr_exp_21d57633_hook_id = -1;

tai_hook_ref_t iofilemgr_exp_dc2d8bce_hook_ref;
SceUID iofilemgr_exp_dc2d8bce_hook_id = -1;

tai_hook_ref_t iofilemgr_exp_6b3ca9f7_hook_ref;
SceUID iofilemgr_exp_6b3ca9f7_hook_id = -1;

//-----------------

tai_hook_ref_t pfs_mgr_219105C_hook_ref;
SceUID pfs_mgr_219105C_hook_id = -1;

tai_hook_ref_t pfs_mgr_2193624_hook_ref;
SceUID pfs_mgr_2193624_hook_id = -1;

tai_hook_ref_t pfs_mgr_2198928_hook_ref;
SceUID pfs_mgr_2198928_hook_id = -1;

tai_hook_ref_t pfs_mgr_2199064_hook_ref;
SceUID pfs_mgr_2199064_hook_id = -1;

tai_hook_ref_t pfs_mgr_219DE44_hook_ref;
SceUID pfs_mgr_219DE44_hook_id = -1;

tai_hook_ref_t pfs_mgr_219DE7C_hook_ref;
SceUID pfs_mgr_219DE7C_hook_id = -1;

tai_hook_ref_t pfs_mgr_2199144_hook_ref;
SceUID pfs_mgr_2199144_hook_id = -1;

SceUID pfs_219DE44_check_patch1_uid = -1;
SceUID pfs_219DE44_check_patch2_uid = -1;

SceUID pfs_219DE44_check_patch3_uid = -1;
SceUID pfs_219DE44_check_patch4_uid = -1;

SceUID pfs_219DE44_check_patch5_uid = -1;
SceUID pfs_219DE44_check_patch6_uid = -1;
SceUID pfs_219DE44_check_patch7_uid = -1;

//-----------

tai_hook_ref_t app_mgr_23D5028_hook_ref = -1;
SceUID app_mgr_23D5028_hook_id = -1;

tai_hook_ref_t app_mgr_23EF934_hook_ref = -1;
SceUID app_mgr_23EF934_hook_id = -1;

tai_hook_ref_t app_mgr_75c96d25_hook_ref = -1;
SceUID app_mgr_75c96d25_hook_id = -1;

tai_hook_ref_t iofilemgr_BE8444_hook_ref = -1;
SceUID iofilemgr_BE8444_hook_id = -1;

tai_hook_ref_t iofilemgr_50a63acf_hook_ref = -1;
SceUID iofilemgr_50a63acf_hook_id = -1;

tai_hook_ref_t iofilemgr_BE584C_hook_ref = -1;
SceUID iofilemgr_BE584C_hook_id = -1;

tai_hook_ref_t iofilemgr_BE5CC4_hook_ref = -1;
SceUID iofilemgr_BE5CC4_hook_id = -1;

tai_hook_ref_t iofilemgr_f7dac0f5_hook_ref = -1;
SceUID iofilemgr_f7dac0f5_hook_id = -1;

tai_hook_ref_t iofilemgr_a5a6a55c_hook_ref = -1;
SceUID iofilemgr_a5a6a55c_hook_id = -1;

//------

tai_hook_ref_t pfs_mgr_21910F4_hook_ref = -1;
SceUID pfs_mgr_21910F4_hook_id = -1;

tai_hook_ref_t pfs_mgr_2193B44_hook_ref = -1;
SceUID pfs_mgr_2193B44_hook_id = -1;

//-----------

tai_hook_ref_t app_mgr_75192972_hook_ref = -1;
SceUID app_mgr_75192972_hook_id = -1;

tai_hook_ref_t iofilemgr_BEA704_hook_ref = -1;
SceUID iofilemgr_BEA704_hook_id = -1;

tai_hook_ref_t iofilemgr_9e347c7d_hook_ref = -1;
SceUID iofilemgr_9e347c7d_hook_id = -1;

tai_hook_ref_t iofilemgr_76b79bec_hook_ref = -1;
SceUID iofilemgr_76b79bec_hook_id = -1;

tai_hook_ref_t pfs_mgr_219112C_hook_ref = -1;
SceUID pfs_mgr_219112C_hook_id = -1;

tai_hook_ref_t pfs_mgr_2192190_hook_ref = -1;
SceUID pfs_mgr_2192190_hook_id = -1;

//-----------

tai_hook_ref_t app_mgr_e17efc03_hook_ref = -1;
SceUID app_mgr_e17efc03_hook_id = -1;

//-----------

SceUID pfs_80140f02_test_patch1_uid = -1;
SceUID pfs_80140f02_test_patch2_uid = -1;

tai_hook_ref_t pfs_mgr_219BF20_hook_ref = -1;
SceUID pfs_mgr_219BF20_hook_id = -1;

tai_hook_ref_t iofilemgr_BE7AEC_hook_ref = -1;
SceUID iofilemgr_BE7AEC_hook_id = -1;

tai_hook_ref_t iofilemgr_BF57F8_hook_ref = -1;
SceUID iofilemgr_BF57F8_hook_id = -1;

tai_hook_ref_t iofilemgr_BF66E4_hook_ref = -1;
SceUID iofilemgr_BF66E4_hook_id = -1;

tai_hook_ref_t iofilemgr_BF6B6C_hook_ref = -1;
SceUID iofilemgr_BF6B6C_hook_id = -1;

tai_hook_ref_t iofilemgr_BF69B4_hook_ref = -1;
SceUID iofilemgr_BF69B4_hook_id = -1;

tai_hook_ref_t iofilemgr_BF651C_hook_ref = -1;
SceUID iofilemgr_BF651C_hook_id = -1;

tai_hook_ref_t iofilemgr_BF89EC_hook_ref = -1;
SceUID iofilemgr_BF89EC_hook_id = -1;

tai_hook_ref_t iofilemgr_BF8BD4_hook_ref = -1;
SceUID iofilemgr_BF8BD4_hook_id = -1;

tai_hook_ref_t iofilemgr_BF7C00_hook_ref = -1;
SceUID iofilemgr_BF7C00_hook_id = -1;

//

tai_hook_ref_t iofilemgr_BF8BB0_hook_ref = -1;
SceUID iofilemgr_BF8BB0_hook_id = -1;

tai_hook_ref_t iofilemgr_BF7288_hook_ref = -1;
SceUID iofilemgr_BF7288_hook_id = -1;

tai_hook_ref_t iofilemgr_BFB5F0_hook_ref = -1;
SceUID iofilemgr_BFB5F0_hook_id = -1;

tai_hook_ref_t iofilemgr_BF88C4_hook_ref = -1;
SceUID iofilemgr_BF88C4_hook_id = -1;

tai_hook_ref_t iofilemgr_BFB794_hook_ref = -1;
SceUID iofilemgr_BFB794_hook_id = -1;

tai_hook_ref_t iofilemgr_BF7268_hook_ref = -1;
SceUID iofilemgr_BF7268_hook_id = -1;

tai_hook_ref_t iofilemgr_BF83A0_hook_ref = -1;
SceUID iofilemgr_BF83A0_hook_id = -1;

tai_hook_ref_t iofilemgr_BFB628_hook_ref = -1;
SceUID iofilemgr_BFB628_hook_id = -1;

tai_hook_ref_t iofilemgr_BF7228_hook_ref = -1;
SceUID iofilemgr_BF7228_hook_id = -1;

tai_hook_ref_t iofilemgr_BF83F8_hook_ref = -1;
SceUID iofilemgr_BF83F8_hook_id = -1;

tai_hook_ref_t iofilemgr_BFB748_hook_ref = -1;
SceUID iofilemgr_BFB748_hook_id = -1;

tai_hook_ref_t iofilemgr_BE7F4C_hook_ref = -1;
SceUID iofilemgr_BE7F4C_hook_id = -1;

tai_hook_ref_t iofilemgr_BFB2A8_hook_ref = -1;
SceUID iofilemgr_BFB2A8_hook_id = -1;

tai_hook_ref_t pfs_facade_4238d2d2_hook_ref = -1;
SceUID pfs_facade_4238d2d2_hook_id = -1;

tai_hook_ref_t iofilemgr_0b54f9e0_hook_ref = -1;
SceUID iofilemgr_0b54f9e0_hook_id = -1;

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

int min(int a, int b)
{
  return (a > b) ? b : a;
}

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

int appmgr_23D9B50_hook(int unk0, int unk1, int unk2, int unk3, int arg_0, int arg_4, int arg_8, int arg_C)
{
  int res = TAI_CONTINUE(int, appmgr_23D9B50_hook_ref, unk0, unk1, unk2, unk3, arg_0, arg_4, arg_8, arg_C);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called appmgr_23D9B50_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int sceAppMgrGameDataVfsMountForDriver_hook(int vshMountId)
{
  int res = TAI_CONTINUE(int, sceAppMgrGameDataVfsMountForDriver_hook_ref, vshMountId);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called sceAppMgrGameDataVfsMountForDriver:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int sceFiosKernelOverlayRemoveForProcessForDriver_hook(int unk0, int unk1)
{
  int res = TAI_CONTINUE(int, sceFiosKernelOverlayRemoveForProcessForDriver_hook_ref, unk0, unk1);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called sceFiosKernelOverlayRemoveForProcessForDriver:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int proc_read_sealedkey_23D6EA0_hook(int unk0, int unk1)
{
  int res = TAI_CONTINUE(int, proc_read_sealedkey_23D6EA0_hook_ref, unk0, unk1);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called proc_read_sealedkey_23D6EA0_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int sbl_acmgr_0b6e6cd7_hook(int unk0)
{
  int res = TAI_CONTINUE(int, sbl_acmgr_0b6e6cd7_hook_ref, unk0);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called sbl_acmgr_0b6e6cd7_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int proc_generate_random_path_23D4FBC_hook(int unk0, int unk1)
{
  int res = TAI_CONTINUE(int, proc_generate_random_path_23D4FBC_hook_ref, unk0, unk1);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called proc_generate_random_path_23D4FBC_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int proc_get_clearsign_stat_23D9A4C_hook(int unk0, int unk1) //this function definitely has 1 argument however it works only with 2 args
{
  int res = TAI_CONTINUE(int, proc_get_clearsign_stat_23D9A4C_hook_ref, unk0); //do I need to pass arg here?

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called proc_get_clearsign_stat_23D9A4C_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int pfs_mgr_a772209c_hook(int unk0, int unk1, int unk2, int unk3, int arg_0, int arg_4)
{
  int res = TAI_CONTINUE(int, pfs_mgr_a772209c_hook_ref, unk0, unk1, unk2, unk3, arg_0, arg_4);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called pfs_mgr_a772209c_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int pfs_mgr_2d48aea2_hook(int unk0, int unk1, int unk2, int unk3)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2d48aea2_hook_ref, unk0, unk1, unk2, unk3);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called pfs_mgr_2d48aea2_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int pfs_mgr_2190AEC_hook(int unk0, int unk1, int unk2, int unk3, int arg_0)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2190AEC_hook_ref, unk0, unk1, unk2, unk3, arg_0);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called pfs_mgr_2190AEC_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int iofilemgr_feee44a9_hook(int unk0, int unk1, int unk2)
{
  int res = TAI_CONTINUE(int, iofilemgr_feee44a9_hook_ref, unk0, unk1, unk2);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called iofilemgr_feee44a9_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int iofilemgr_d220539d_hook(int unk0, int unk1, int unk2)
{
  int res = TAI_CONTINUE(int, iofilemgr_d220539d_hook_ref, unk0, unk1, unk2);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called iofilemgr_d220539d_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int iofilemgr_BF3848_hook(ctx_BF3848* ctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF3848_hook_ref, ctx);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called iofilemgr_BF3848_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    if(ctx > 0)
    {
      snprintf(sprintfBuffer, 256, "20: %08x\n", ctx->error); // this is exactly 0x8001008F error that is set after sceKernelSignalCondTo to "SceIofilemgr"
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log();

  return res;
}

int g_pfs_mount_enable_log = 0;

int iofilemgr_BF3474_hook(ctx_BF3848* ctx)
{
  if(ctx > 0)
  {
    if(ctx->operation == 3)
    {
      g_pfs_mount_enable_log = 1;
    }
  }

  int res = TAI_CONTINUE(int, iofilemgr_BF3474_hook_ref, ctx);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called iofilemgr_BF3474_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    if(ctx > 0)
    {
      if(ctx->operation == 3) //print only operation 3
      {
        snprintf(sprintfBuffer, 256, "20: %08x\n", ctx->error); // this is exactly 0x8001008F error that is set after sceKernelSignalCondTo to "SceIofilemgr"
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
    }
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log();

  if(ctx > 0)
  {
    if(ctx->operation == 3)
    {
      g_pfs_mount_enable_log = 0;
    }
  }

  return res;
}

//-------

int g_iofilemgr_BE62E8_hook_error = 0;
int g_iofilemgr_BE5F10_hook_error = 0;
int g_iofilemgr_BE6788_hook_error = 0;
int g_iofilemgr_BEE3C8_hook_error = 0;
int g_iofilemgr_BEBAC0_hook_error = 0;
int g_iofilemgr_sceVfsGetNewNode_hook_error = 0;
int g_iofilemgr_BE61C4_hook_error = 0;
int g_iofilemgr_BEDEB0_hook_error = 0;
int g_iofilemgr_BE59BC_hook_error = 0;
int g_iofilemgr_BE5B30_hook_error = 0;
int g_iofilemgr_BF18CC_hook_error = 0;
int g_iofilemgr_BF1AF0_hook_error = 0;

int g_iofilemgr_BE62E8_hook_error_cnt = 0;
int g_iofilemgr_BE5F10_hook_error_cnt = 0;
int g_iofilemgr_BE6788_hook_error_cnt = 0;
int g_iofilemgr_BEE3C8_hook_error_cnt = 0;
int g_iofilemgr_BEBAC0_hook_error_cnt = 0;
int g_iofilemgr_sceVfsGetNewNode_hook_error_cnt = 0;
int g_iofilemgr_BE61C4_hook_error_cnt = 0;
int g_iofilemgr_BEDEB0_hook_error_cnt = 0;
int g_iofilemgr_BE59BC_hook_error_cnt = 0;
int g_iofilemgr_BE5B30_hook_error_cnt = 0;
int g_iofilemgr_BF18CC_hook_error_cnt = 0;
int g_iofilemgr_BF1AF0_hook_error_cnt = 0;

//--------

int g_iofilemgr_BEE364_hook_error = 0;
int g_iofilemgr_BEE2D4_hook_error = 0;
int g_iofilemgr_BEE2C4_hook_error = 0;
int g_iofilemgr_BEDF5C_hook_error = 0;
int g_iofilemgr_BECE80_hook_error = 0;
int g_iofilemgr_BEC808_hook_error = 0;
int g_iofilemgr_BEC620_hook_error = 0;
int g_iofilemgr_BEC578_hook_error = 0;
int g_iofilemgr_BEC56C_hook_error = 0;
int g_iofilemgr_BEC530_hook_error = 0;
int g_iofilemgr_BEC010_hook_error = 0;
int g_iofilemgr_BE5A38_hook_error = 0;
int g_iofilemgr_BE5814_hook_error = 0;
int g_iofilemgr_BEBB84_hook_error = 0;
int g_iofilemgr_BEC7C0_hook_error = 0;
int g_iofilemgr_BECE0C_hook_error = 0;

int g_iofilemgr_BEE364_hook_error_cnt = 0;
int g_iofilemgr_BEE2D4_hook_error_cnt = 0;
int g_iofilemgr_BEE2C4_hook_error_cnt = 0;
int g_iofilemgr_BEDF5C_hook_error_cnt = 0;
int g_iofilemgr_BECE80_hook_error_cnt = 0;
int g_iofilemgr_BEC808_hook_error_cnt = 0;
int g_iofilemgr_BEC620_hook_error_cnt = 0;
int g_iofilemgr_BEC578_hook_error_cnt = 0;
int g_iofilemgr_BEC56C_hook_error_cnt = 0;
int g_iofilemgr_BEC530_hook_error_cnt = 0;
int g_iofilemgr_BEC010_hook_error_cnt = 0;
int g_iofilemgr_BE5A38_hook_error_cnt = 0;
int g_iofilemgr_BE5814_hook_error_cnt = 0;
int g_iofilemgr_BEBB84_hook_error_cnt = 0;
int g_iofilemgr_BEC7C0_hook_error_cnt = 0;
int g_iofilemgr_BECE0C_hook_error_cnt = 0;

//-------

int g_iofilemgr_exp_aa45010b_hook_error = 0;
int g_iofilemgr_exp_21d57633_hook_error = 0;
int g_iofilemgr_exp_dc2d8bce_hook_error = 0;
int g_iofilemgr_exp_6b3ca9f7_hook_error = 0;

int g_iofilemgr_exp_aa45010b_hook_error_cnt = 0;
int g_iofilemgr_exp_21d57633_hook_error_cnt = 0;
int g_iofilemgr_exp_dc2d8bce_hook_error_cnt = 0;
int g_iofilemgr_exp_6b3ca9f7_hook_error_cnt = 0;

//-------

int g_pfs_mgr_219105C_hook_error = 0;
int g_pfs_mgr_219105C_hook_error_cnt = 0;

int g_pfs_mgr_2193624_hook_error = 0;
int g_pfs_mgr_2193624_hook_error_cnt = 0;

int g_pfs_mgr_2198928_hook_error = 0;
int g_pfs_mgr_2198928_hook_error_cnt = 0;

int g_pfs_mgr_2199064_hook_error = 0;
int g_pfs_mgr_2199064_hook_error_cnt = 0;

int g_pfs_mgr_219DE44_hook_error = 0;
int g_pfs_mgr_219DE44_hook_error_cnt = 0;

int g_pfs_mgr_219DE7C_hook_error = 0;
int g_pfs_mgr_219DE7C_hook_error_cnt = 0;

int g_pfs_mgr_2199144_hook_error = 0;
int g_pfs_mgr_2199144_hook_error_cnt = 0;

//-------

vfs_mount g_mount_copys[10] = {0};
int g_mount_copys_cnt = 0;

char g_read_buffer_copys[0x50][0x200];
int g_read_buffer_copys_cnt = 0;

int g_read_buffer_ofsetLow[0x50] = {0};
int g_read_buffer_ofsetHi[0x50] = {0};
int g_read_buffer_size[0x50] = {0};

char g_crypto_buffer_before_copys[0x50][0x200];
int g_crypto_buffer_before_copys_cnt = 0;

char g_crypto_buffer_after_copys[0x50][0x200];
int g_crypto_buffer_after_copys_cnt = 0;

char nodeStr[0x100] = {0}; 
int nodeStrLen = 0;

//-------

int print_vfs_mount(vfs_mount* mnt)
{
  snprintf(sprintfBuffer, 256, "vfs_mount unixMount: %s\n", mnt->unixMount);
  FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

  snprintf(sprintfBuffer, 256, "vfs_mount devMajor: %08x\n", mnt->devMajor);
  FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

  snprintf(sprintfBuffer, 256, "vfs_mount devMinor: %08x\n", mnt->devMinor);
  FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

  if(mnt->add_data > 0)
  {
    snprintf(sprintfBuffer, 256, "vfs_mount add_data name: %s\n", mnt->add_data->name);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(mnt->blockDev > 0)
  {
    snprintf(sprintfBuffer, 256, "vfs_mount blockDev vitaMount: %s\n", mnt->blockDev->vitaMount);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

    snprintf(sprintfBuffer, 256, "vfs_mount blockDev filesystem: %s\n", mnt->blockDev->filesystem);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

    snprintf(sprintfBuffer, 256, "vfs_mount blockDev blockDevicePrimary: %s\n", mnt->blockDev->blockDevicePrimary);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  return 0;
}

int g_iofilemgr_sceVfsMount_enable_log = 0;

int iofilemgr_sceVfsMount_hook(vfs_mount_point_info_base* mountInfo)
{
  if(mountInfo > 0)
  {
    if(mountInfo->filesystem > 0)
    {
      if(strncmp(mountInfo->filesystem, "PFS_GDSD_INF", sizeof("PFS_GDSD_INF")) == 0)
      {
        g_iofilemgr_sceVfsMount_enable_log = 1;
      }
    }
  }

  int res = TAI_CONTINUE(int, iofilemgr_sceVfsMount_hook_ref, mountInfo);

  if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      snprintf(sprintfBuffer, 256, "called iofilemgr_sceVfsMount_hook:\nres: %08x\n", res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      if(mountInfo >= 0)
      {
        snprintf(sprintfBuffer, 256, "unixMount: %s filesystem: %s\n", mountInfo->unixMount, mountInfo->filesystem);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        
        snprintf(sprintfBuffer, 256, "devMajor: %08x devMinor: %08x\n", mountInfo->devMajor, mountInfo->devMinor);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

        if(mountInfo->blockDev1 >= 0)
        {
          snprintf(sprintfBuffer, 256, "vitaMount: %s filesystem: %s\n", mountInfo->blockDev1->vitaMount, mountInfo->blockDev1->filesystem);
          FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

          snprintf(sprintfBuffer, 256, "blockDevicePrimary: %s vshMountId: %08x\n", mountInfo->blockDev1->blockDevicePrimary, mountInfo->blockDev1->vshMountId);
          FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        }
      }

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE62E8_hook error: %08x cnt: %08x\n", g_iofilemgr_BE62E8_hook_error, g_iofilemgr_BE62E8_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE6788_hook error: %08x cnt: %08x\n", g_iofilemgr_BE6788_hook_error, g_iofilemgr_BE6788_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE5F10_hook error: %08x cnt: %08x\n", g_iofilemgr_BE5F10_hook_error, g_iofilemgr_BE5F10_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEE3C8_hook error: %08x cnt: %08x\n", g_iofilemgr_BEE3C8_hook_error, g_iofilemgr_BEE3C8_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEBAC0_hook error: %08x cnt: %08x\n", g_iofilemgr_BEBAC0_hook_error, g_iofilemgr_BEBAC0_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_sceVfsGetNewNode_hook error: %08x cnt: %08x\n", g_iofilemgr_sceVfsGetNewNode_hook_error, g_iofilemgr_sceVfsGetNewNode_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE61C4_hook error: %08x cnt: %08x\n", g_iofilemgr_BE61C4_hook_error, g_iofilemgr_BE61C4_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEDEB0_hook error: %08x cnt: %08x\n", g_iofilemgr_BEDEB0_hook_error, g_iofilemgr_BEDEB0_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE59BC_hook error: %08x cnt: %08x\n", g_iofilemgr_BE59BC_hook_error, g_iofilemgr_BE59BC_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE5B30_hook error: %08x cnt: %08x\n", g_iofilemgr_BE5B30_hook_error, g_iofilemgr_BE5B30_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BF18CC_hook error: %08x cnt: %08x\n", g_iofilemgr_BF18CC_hook_error, g_iofilemgr_BF18CC_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BF1AF0_hook error: %08x cnt: %08x\n", g_iofilemgr_BF1AF0_hook_error, g_iofilemgr_BF1AF0_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //------------------

      /*
      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEE364_hook error: %08x cnt: %08x\n", g_iofilemgr_BEE364_hook_error, g_iofilemgr_BEE364_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEE2D4_hook error: %08x cnt: %08x\n", g_iofilemgr_BEE2D4_hook_error, g_iofilemgr_BEE2D4_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEE2C4_hook error: %08x cnt: %08x\n", g_iofilemgr_BEE2C4_hook_error, g_iofilemgr_BEE2C4_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEDF5C_hook error: %08x cnt: %08x\n", g_iofilemgr_BEDF5C_hook_error, g_iofilemgr_BEDF5C_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BECE80_hook error: %08x cnt: %08x\n", g_iofilemgr_BECE80_hook_error, g_iofilemgr_BECE80_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEC808_hook error: %08x cnt: %08x\n", g_iofilemgr_BEC808_hook_error, g_iofilemgr_BEC808_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEC620_hook error: %08x cnt: %08x\n", g_iofilemgr_BEC620_hook_error, g_iofilemgr_BEC620_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEC578_hook error: %08x cnt: %08x\n", g_iofilemgr_BEC578_hook_error, g_iofilemgr_BEC578_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEC56C_hook error: %08x cnt: %08x\n", g_iofilemgr_BEC56C_hook_error, g_iofilemgr_BEC56C_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEC530_hook error: %08x cnt: %08x\n", g_iofilemgr_BEC530_hook_error, g_iofilemgr_BEC530_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEC010_hook error: %08x cnt: %08x\n", g_iofilemgr_BEC010_hook_error, g_iofilemgr_BEC010_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE5A38_hook error: %08x cnt: %08x\n", g_iofilemgr_BE5A38_hook_error, g_iofilemgr_BE5A38_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE5814_hook error: %08x cnt: %08x\n", g_iofilemgr_BE5814_hook_error, g_iofilemgr_BE5814_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEBB84_hook error: %08x cnt: %08x\n", g_iofilemgr_BEBB84_hook_error, g_iofilemgr_BEBB84_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BEC7C0_hook error: %08x cnt: %08x\n", g_iofilemgr_BEC7C0_hook_error, g_iofilemgr_BEC7C0_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BECE0C_hook error: %08x cnt: %08x\n", g_iofilemgr_BECE0C_hook_error, g_iofilemgr_BECE0C_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      */
      //------
      
      /*
      snprintf(sprintfBuffer, 256, "g_iofilemgr_exp_aa45010b_hook error: %08x cnt: %08x\n", g_iofilemgr_exp_aa45010b_hook_error, g_iofilemgr_exp_aa45010b_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_exp_21d57633_hook error: %08x cnt: %08x\n", g_iofilemgr_exp_21d57633_hook_error, g_iofilemgr_exp_21d57633_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_exp_dc2d8bce_hook error: %08x cnt: %08x\n", g_iofilemgr_exp_dc2d8bce_hook_error, g_iofilemgr_exp_dc2d8bce_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_exp_6b3ca9f7_hook error: %08x cnt: %08x\n", g_iofilemgr_exp_6b3ca9f7_hook_error, g_iofilemgr_exp_6b3ca9f7_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      */

      snprintf(sprintfBuffer, 256, "g_pfs_mgr_219105C_hook error: %08x cnt: %08x\n", g_pfs_mgr_219105C_hook_error, g_pfs_mgr_219105C_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_pfs_mgr_2193624_hook error: %08x cnt: %08x\n", g_pfs_mgr_2193624_hook_error, g_pfs_mgr_2193624_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_pfs_mgr_2198928_hook error: %08x cnt: %08x\n", g_pfs_mgr_2198928_hook_error, g_pfs_mgr_2198928_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_pfs_mgr_2199064_hook error: %08x cnt: %08x\n", g_pfs_mgr_2199064_hook_error, g_pfs_mgr_2199064_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_pfs_mgr_219DE44_hook error: %08x cnt: %08x\n", g_pfs_mgr_219DE44_hook_error, g_pfs_mgr_219DE44_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_pfs_mgr_219DE7C_hook error: %08x cnt: %08x\n", g_pfs_mgr_219DE7C_hook_error, g_pfs_mgr_219DE7C_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_pfs_mgr_2199144_hook error: %08x cnt: %08x\n", g_pfs_mgr_2199144_hook_error, g_pfs_mgr_2199144_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      FILE_GLOBAL_WRITE_LEN("======================================\n");

      //print string from vfs_node ctx_70
      /*
      snprintf(sprintfBuffer, 256, "nodeStrLen %08x\n", nodeStrLen);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      for(int i = 0; i < 0x100; i++)
      {
        char buffer[4];
        snprintf(buffer, 4, "%02x ", nodeStr[i]);
        FILE_GLOBAL_WRITE_LEN(buffer);
      }
      FILE_GLOBAL_WRITE_LEN("\n");

      FILE_GLOBAL_WRITE_LEN("======================================\n");
      */
 
      //print vfs_mount s
      /*
      for(int k = 0; k < 10; k++)
      {
        print_vfs_mount(&g_mount_copys[k]);
      
        FILE_GLOBAL_WRITE_LEN("======================================\n");
      }
      */

      //print bytes from read wrapper
      
      /*
      for(int k = 0; k < 2; k++)
      {
        snprintf(sprintfBuffer, 256, "offset lo: %08x offset hi: %08x size: %08x\n", g_read_buffer_ofsetLow[k], g_read_buffer_ofsetHi[k], g_read_buffer_size[k]);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

        for(int i = 0; i < 0x200; i++)
        {
          char buffer[4];
          snprintf(buffer, 4, "%02x ", g_read_buffer_copys[k][i]);
          FILE_GLOBAL_WRITE_LEN(buffer);
        }
        FILE_GLOBAL_WRITE_LEN("\n");

        FILE_GLOBAL_WRITE_LEN("======================================\n");
      }
      */

      //print bytes from crypto proc
      /*
      for(int i = 0; i < 0x200; i++)
      {
        char buffer[4];
        snprintf(buffer, 4, "%02x ", g_crypto_buffer_before_copys[0][i]);
        FILE_GLOBAL_WRITE_LEN(buffer);
      }
      FILE_GLOBAL_WRITE_LEN("\n");

      FILE_GLOBAL_WRITE_LEN("======================================\n");

      for(int i = 0; i < 0x200; i++)
      {
        char buffer[4];
        snprintf(buffer, 4, "%02x ", g_crypto_buffer_after_copys[0][i]);
        FILE_GLOBAL_WRITE_LEN(buffer);
      }
      FILE_GLOBAL_WRITE_LEN("\n");

      FILE_GLOBAL_WRITE_LEN("======================================\n");
      */
    }
    close_global_log();
  }

  if(mountInfo > 0)
  {
    if(mountInfo->filesystem > 0)
    {
      if(strncmp(mountInfo->filesystem, "PFS_GDSD_INF", sizeof("PFS_GDSD_INF")) == 0)
      {
        g_iofilemgr_sceVfsMount_enable_log = 0;
      }
    }
  }

  return res;
}

//---

int iofilemgr_BE62E8_hook(int unk0, int unk1, int unk2, int unk3, int arg_0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE62E8_hook_ref, unk0, unk1, unk2, unk3, arg_0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE62E8_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BE62E8_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE62E8_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BE5F10_hook(vfs_node *ctx, int unk1, int unk2, int unk3, int arg_0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE5F10_hook_ref, ctx, unk1, unk2, unk3, arg_0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE5F10_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BE5F10_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE5F10_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BE6788_hook(void *unk0, int unk1, vfs_node **vnode, int unk3, int unk4)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE6788_hook_ref, unk0, unk1, vnode, unk3, unk4);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE6788_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BE6788_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE6788_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEE3C8_hook(vfs_node *ctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEE3C8_hook_ref, ctx);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEE3C8_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BEE3C8_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEE3C8_hook_error = res;
      }
    }
  }

  return res;
}

void* iofilemgr_BEBAC0_hook()
{
  void* res = TAI_CONTINUE(void*, iofilemgr_BEBAC0_hook_ref);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEBAC0_hook_error_cnt++;

    if((int)res == 0x8001008f)
    {
      g_iofilemgr_BEBAC0_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_iofilemgr_BEBAC0_hook_error = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_sceVfsGetNewNode_hook(vfs_mount* mount, node_ops2* ops, int unused, vfs_node** node)
{
  int res = TAI_CONTINUE(int, iofilemgr_sceVfsGetNewNode_hook_ref, mount, ops, unused, node);

  memcpy((void*)&g_mount_copys[g_mount_copys_cnt], (void*)mount, sizeof(vfs_mount));
  g_mount_copys_cnt++;
  if(g_mount_copys_cnt == 10)
    g_mount_copys_cnt = 0;

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_sceVfsGetNewNode_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_sceVfsGetNewNode_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_sceVfsGetNewNode_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BE61C4_hook(int unk0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE61C4_hook_ref, unk0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE61C4_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BE61C4_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE61C4_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEDEB0_hook(uint32_t* a0, int a1, vfs_node* a2, int a3)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEDEB0_hook_ref, a0, a1, a2, a3);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEDEB0_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BEDEB0_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEDEB0_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BE59BC_hook(vfs_node* a0, void* a1)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE59BC_hook_ref, a0, a1);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE59BC_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BE59BC_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE59BC_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BE5B30_hook(vfs_node* a0, vfs_node* a1, void* a2, int a3, int a4)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE5B30_hook_ref, a0, a1, a2, a3, a4);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE5B30_hook_error_cnt++;

    if(res == 0x8001008f)
    {
      g_iofilemgr_BE5B30_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE5B30_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BF18CC_hook(vfs_mount* a0, uint32_t* a1)
{
   int res = TAI_CONTINUE(int, iofilemgr_BF18CC_hook_ref, a0, a1);

   if(1)
   {
      g_iofilemgr_BF18CC_hook_error_cnt++;

      if(res == 0x8001008f)
      {
        g_iofilemgr_BF18CC_hook_error = res;
      }
      else
      {
        if(res < 0)
        {
          g_iofilemgr_BF18CC_hook_error = res;
        }
      }
   }

   return res;
}

int iofilemgr_BF1AF0_hook(vfs_node *cur_node, int unk1, vfs_node *node)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF1AF0_hook_ref, cur_node, unk1, node);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BF1AF0_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BF1AF0_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BF1AF0_hook_error = res;
      }
    }
  }
  
  return res;
}

//---------------

int iofilemgr_BEE364_hook(vfs_node *node)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEE364_hook_ref, node);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEE364_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEE364_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEE364_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEE2D4_hook()
{
  int res = TAI_CONTINUE(int, iofilemgr_BEE2D4_hook_ref);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEE2D4_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEE2D4_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEE2D4_hook_error = res;
      }
    }
  }


  return res;
}

int iofilemgr_BEE2C4_hook()
{
  int res = TAI_CONTINUE(int, iofilemgr_BEE2C4_hook_ref);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEE2C4_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEE2C4_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEE2C4_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEDF5C_hook(int unk0, int unk1)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEDF5C_hook_ref, unk0, unk1);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEDF5C_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEDF5C_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEDF5C_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BECE80_hook(int unk0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BECE80_hook_ref, unk0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BECE80_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BECE80_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BECE80_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEC808_hook(int unk0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEC808_hook_ref, unk0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEC808_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEC808_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEC808_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEC620_hook(int unk0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEC620_hook_ref, unk0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEC620_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEC620_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEC620_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEC578_hook(int unk0, int unk1)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEC578_hook_ref, unk0, unk1);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEC578_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEC578_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEC578_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEC56C_hook(void *unk0, void *unk1)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEC56C_hook_ref, unk0, unk1);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEC56C_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEC56C_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEC56C_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEC530_hook(int unk0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEC530_hook_ref, unk0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEC530_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEC530_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEC530_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEC010_hook(int unk0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEC010_hook_ref, unk0);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEC010_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEC010_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEC010_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BE5A38_hook(int unk0, int unk1)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE5A38_hook_ref, unk0, unk1);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE5A38_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BE5A38_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE5A38_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BE5814_hook(vfs_node *ctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE5814_hook_ref, ctx);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BE5814_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BE5814_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BE5814_hook_error = res;
      }
    }
  }

  return res;
}

int iofilemgr_BEBB84_hook(vfs_mount* arg0, vfs_node* arg1, SceUID heapid, vfs_add_data* arg3)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEBB84_hook_ref, arg0, arg1, heapid, arg3);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======iofilemgr_BEBB84_hook===========\n");

    if(arg3 > 0)
    {
      snprintf(sprintfBuffer, 256, "add name: %s\n", arg3->name);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }

    if(arg0 > 0)
    {
      print_vfs_mount(arg0);
    }

    snprintf(sprintfBuffer, 256, "unk_70: %08x\n", arg1->unk_70);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

    snprintf(sprintfBuffer, 256, "res: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log();

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEBB84_hook_error_cnt++;
    
    if(res == 0x8001008f)
    {
      g_iofilemgr_BEBB84_hook_error = res;
    }
    else
    {
      if(res < 0)
      {
        g_iofilemgr_BEBB84_hook_error = res;
      }
    }
  }

  return res;
}

vfs_add_data* iofilemgr_BEC7C0_hook(char *filesystem)
{
  vfs_add_data* res = TAI_CONTINUE(vfs_add_data*, iofilemgr_BEC7C0_hook_ref, filesystem);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======iofilemgr_BEC7C0_hook===========\n");
    
    snprintf(sprintfBuffer, 256, "filesystem: %s\n", filesystem);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    
    if(res > 0)
    {
      snprintf(sprintfBuffer, 256, "add name: %s\n", res->name);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }

    snprintf(sprintfBuffer, 256, "res: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log();

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BEC7C0_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_iofilemgr_BEC7C0_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_iofilemgr_BEC7C0_hook_error = (int)res;
      }
    }
  }

  return res;
}

void* iofilemgr_BECE0C_hook()
{
  void* res = TAI_CONTINUE(void*, iofilemgr_BECE0C_hook_ref);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_BECE0C_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_iofilemgr_BECE0C_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_iofilemgr_BECE0C_hook_error = (int)res;
      }
    }
  }

  return res;
}

//----------------

int iofilemgr_exp_aa45010b_hook(vfs_node *ctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_exp_aa45010b_hook_ref, ctx);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_exp_aa45010b_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_iofilemgr_exp_aa45010b_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_iofilemgr_exp_aa45010b_hook_error = (int)res;
      }
    }
  }


  return res;
}

int iofilemgr_exp_21d57633_hook(vfs_node *node)
{
  int res = TAI_CONTINUE(int, iofilemgr_exp_21d57633_hook_ref, node);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_exp_21d57633_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_iofilemgr_exp_21d57633_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_iofilemgr_exp_21d57633_hook_error = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_exp_dc2d8bce_hook(void *mutex)
{
  int res = TAI_CONTINUE(int, iofilemgr_exp_dc2d8bce_hook_ref, mutex);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_exp_dc2d8bce_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_iofilemgr_exp_dc2d8bce_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_iofilemgr_exp_dc2d8bce_hook_error = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_exp_6b3ca9f7_hook(void *mutex)
{
  int res = TAI_CONTINUE(int, iofilemgr_exp_6b3ca9f7_hook_ref, mutex);

  if(1)
  //if(g_pfs_mount_enable_log > 0 && g_iofilemgr_sceVfsMount_enable_log > 0)
  { 
    g_iofilemgr_exp_6b3ca9f7_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_iofilemgr_exp_6b3ca9f7_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_iofilemgr_exp_6b3ca9f7_hook_error = (int)res;
      }
    }
  }

  return res;
}

int pfs_mgr_219105C_hook(pfs_mgr_219105C_hook_args* args)
{
  int res = TAI_CONTINUE(int, pfs_mgr_219105C_hook_ref, args);

  if(1)
  { 
    g_pfs_mgr_219105C_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_pfs_mgr_219105C_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_pfs_mgr_219105C_hook_error = (int)res;
      }
    }
  }

  return res;
}

int pfs_mgr_2193624_hook(int arg0, int arg1)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2193624_hook_ref, arg0, arg1);

  if(1)
  { 
    g_pfs_mgr_2193624_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_pfs_mgr_2193624_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_pfs_mgr_2193624_hook_error = (int)res;
      }
    }
  }

  return res;
}

int pfs_mgr_2198928_hook(int unk0, int unk1, int unk2, int unk3)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2198928_hook_ref, unk0, unk1, unk2, unk3);

  if(1)
  { 
    g_pfs_mgr_2198928_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_pfs_mgr_2198928_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_pfs_mgr_2198928_hook_error = (int)res;
      }
    }
  }

  return res;
}

const unsigned char sao_files_db_start[0x30] =
{
    0x53, 0x43, 0x45, 0x4e, 0x47, 0x50, 0x46, 0x53,
    0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x04, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
    0x0d, 0x00, 0x00, 0x00, 0xe6, 0x79, 0x2c, 0x65,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//read wrapper
int pfs_mgr_2199064_hook(node_holder* unk0, char *buffer, int size, int ignored, int offsetLo, int offsetHi, uint32_t* readBytes)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2199064_hook_ref, unk0, buffer, size, ignored, offsetLo, offsetHi, readBytes);
   
  //int isSaoFilesDb = memcmp(buffer, sao_files_db_start, 0x30);

  //int isSaoFilesDb = ((offsetLo == 0x80 || offsetHi == 0x80) && strncmp(buffer, "SCENGPFS", 8) == 0) ? 0 : 1;
  //int isSaoFilesDb = (offsetLo == 0x80 || offsetHi == 0x80) ? 0 : 1;

  if(1)
  //if(isSaoFilesDb == 0)
  { 
    g_pfs_mgr_2199064_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_pfs_mgr_2199064_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_pfs_mgr_2199064_hook_error = (int)res;
      }
    }
  }

  /*
  if(isSaoFilesDb == 0)
  {
    g_read_buffer_ofsetLow[g_read_buffer_copys_cnt] = offsetLo;
    g_read_buffer_ofsetHi[g_read_buffer_copys_cnt] = offsetHi;
    g_read_buffer_size[g_read_buffer_copys_cnt] = size;

    memcpy(g_read_buffer_copys[g_read_buffer_copys_cnt], buffer, min(size, 0x200));

    g_read_buffer_copys_cnt++;

    if(g_read_buffer_copys_cnt == 0x50)
      g_read_buffer_copys_cnt = 0;
  }
  */

  return res;
}

int pfs_mgr_219DE44_hook(char *unk0, char *unk1)
{
  int res = TAI_CONTINUE(int, pfs_mgr_219DE44_hook_ref, unk0, unk1);

  if(1)
  { 
    g_pfs_mgr_219DE44_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_pfs_mgr_219DE44_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_pfs_mgr_219DE44_hook_error = (int)res;
      }
    }
  }

  return res;
}

int pfs_mgr_219DE7C_hook(char bytes14[0x14], ctx_21A27B8* base, ctx_21A27B8_70* data_base, int size)
{
  int isSaoFilesDb = memcmp(data_base->unk_70, sao_files_db_start, 0x30);

  /*
  if(size == 0x160)
  {
    memcpy(&g_crypto_buffer_before_copys[g_crypto_buffer_before_copys_cnt], base, min(size, 0x200));
    g_crypto_buffer_before_copys_cnt++;
    if(g_crypto_buffer_before_copys_cnt == 0x50)
      g_crypto_buffer_before_copys_cnt = 0;
  }
  */

  int res = TAI_CONTINUE(int, pfs_mgr_219DE7C_hook_ref, bytes14, base, data_base, size);

  /*
  if(size == 0x160)
  {
    memcpy(&g_crypto_buffer_after_copys[g_crypto_buffer_after_copys_cnt], base, min(size, 0x200));
    g_crypto_buffer_after_copys_cnt++;
    if(g_crypto_buffer_after_copys_cnt == 0x50)
      g_crypto_buffer_after_copys_cnt = 0;
  }
  */

  if(isSaoFilesDb == 0)
  { 
    g_pfs_mgr_219DE7C_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_pfs_mgr_219DE7C_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_pfs_mgr_219DE7C_hook_error = (int)res;
      }
    }
  }

  return res;
}

int pfs_mgr_2199144_hook(node_holder* nodeHolder, result_2199144* result_pair)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2199144_hook_ref, nodeHolder, result_pair);

  if(1)
  { 
    g_pfs_mgr_2199144_hook_error_cnt++;
    
    if((int)res == 0x8001008f)
    {
      g_pfs_mgr_2199144_hook_error = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        g_pfs_mgr_2199144_hook_error = (int)res;
      }
    }
  }

  if(nodeHolder > 0)
  {
    vfs_node* node = nodeHolder->node;
    if(node > 0)
    {
      vfs_node_70* ctx70 = node->unk_70;
      if(ctx70 > 0)
      {
        char* str = ctx70->str_1C;
        int str_len = ctx70->str_len_18;
        if(str > 0)
        {
          if(nodeStrLen == 0)
          {
            memcpy(nodeStr, str, min(0x100, str_len));
            nodeStrLen = str_len;
          }
        }
        else
        {
          nodeStrLen = -4;
        }
      }
      else
      {
        nodeStrLen = -3;
      }
    }
    else
    {
      nodeStrLen = -2;
    }
  }
  else
  {
    nodeStrLen = -1;
  }

  return res;
}

//param.sfo parser procedure
int app_mgr_23D5028_hook(SceUID pid, int unk1, int unk2, int unk3)
{
  int res = TAI_CONTINUE(int, app_mgr_23D5028_hook_ref, pid, unk1, unk2, unk3);

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called app_mgr_23D5028_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int log_iofilemgr_in_app_mgr = 0;

//param.sfo read procedure
int app_mgr_23EF934_hook(int unk0, int unk1)
{
  log_iofilemgr_in_app_mgr = 1;

  int res = TAI_CONTINUE(int, app_mgr_23EF934_hook_ref, unk0, unk1);

  log_iofilemgr_in_app_mgr = 0;

  open_global_log();
  {
    FILE_GLOBAL_WRITE_LEN("======================================\n");
    snprintf(sprintfBuffer, 256, "called app_mgr_23EF934_hook:\nres: %08x\n", res);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    FILE_GLOBAL_WRITE_LEN("======================================\n");
  }
  close_global_log(); 

  return res;
}

int iofilemgr_BE8444_hook_err = 0;
int iofilemgr_BE8444_hook_err_cnt = 0;

int iofilemgr_50a63acf_hook_err = 0;
int iofilemgr_50a63acf_hook_err_cnt = 0;

int iofilemgr_BE584C_hook_err = 0;
int iofilemgr_BE584C_hook_err_cnt = 0;

int iofilemgr_BE5CC4_hook_err = 0;
int iofilemgr_BE5CC4_hook_err_cnt = 0;

node_ops1* iofilemgr_BE5CC4_hook_ops1;
node_ops2* iofilemgr_BE5CC4_hook_ops2;

int iofilemgr_f7dac0f5_hook_err = 0;
int iofilemgr_f7dac0f5_hook_err_cnt = 0;

int iofilemgr_a5a6a55c_hook_err = 0;
int iofilemgr_a5a6a55c_hook_err_cnt = 0;

int pfs_mgr_21910F4_hook_err = 0;
int pfs_mgr_21910F4_hook_err_cnt = 0;

int pfs_mgr_2193B44_hook_err = 0;
int pfs_mgr_2193B44_hook_err_cnt = 0;

//ksceIoGetstat hook
int app_mgr_75c96d25_hook(const char *file, SceIoStat *stat)
{
  int res = TAI_CONTINUE(int, app_mgr_75c96d25_hook_ref, file, stat);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      
      snprintf(sprintfBuffer, 256, "called ksceIoGetstat:\nfile: %s res: %08x\n", file, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BE8444_hook error: %08x cnt: %08x\n", iofilemgr_BE8444_hook_err, iofilemgr_BE8444_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_50a63acf_hook error: %08x cnt: %08x\n", iofilemgr_50a63acf_hook_err, iofilemgr_50a63acf_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "g_iofilemgr_BE62E8_hook error: %08x cnt: %08x\n", g_iofilemgr_BE62E8_hook_error, g_iofilemgr_BE62E8_hook_error_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "g_iofilemgr_BE6788_hook error: %08x cnt: %08x\n", g_iofilemgr_BE6788_hook_error, g_iofilemgr_BE6788_hook_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BE584C_hook error: %08x cnt: %08x\n", iofilemgr_BE584C_hook_err, iofilemgr_BE584C_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BE5CC4_hook error: %08x cnt: %08x\n", iofilemgr_BE5CC4_hook_err, iofilemgr_BE5CC4_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      if(iofilemgr_BE5CC4_hook_ops1 > 0)
      {
        int curValue = (int)iofilemgr_BE5CC4_hook_ops1;
        int segidx = find_in_segments(g_segListKernel, SEG_LIST_SIZE, &moduleListIsConstructedKernel, curValue);
        if(segidx >= 0)
        {
          open_global_log();
          {
            snprintf(sprintfBuffer, 256, "ops1: %08x %s %d %08x %08x\n", curValue, g_segListKernel[segidx].moduleName, g_segListKernel[segidx].seg, g_segListKernel[segidx].range.start, (curValue - g_segListKernel[segidx].range.start));
            FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
          }
          close_global_log();
        }
      }
      
      if(iofilemgr_BE5CC4_hook_ops2 > 0)
      {
        int curValue = (int)iofilemgr_BE5CC4_hook_ops2;
        int segidx = find_in_segments(g_segListKernel, SEG_LIST_SIZE, &moduleListIsConstructedKernel, curValue);
        if(segidx >= 0)
        {
          open_global_log();
          {
            snprintf(sprintfBuffer, 256, "ops2: %08x %s %d %08x %08x\n", curValue, g_segListKernel[segidx].moduleName, g_segListKernel[segidx].seg, g_segListKernel[segidx].range.start, (curValue - g_segListKernel[segidx].range.start));
            FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
          }
          close_global_log();
        }
      }

      //snprintf(sprintfBuffer, 256, "iofilemgr_f7dac0f5_hook error: %08x cnt: %08x\n", iofilemgr_f7dac0f5_hook_err, iofilemgr_f7dac0f5_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_a5a6a55c_hook error: %08x cnt: %08x\n", iofilemgr_a5a6a55c_hook_err, iofilemgr_a5a6a55c_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "pfs_mgr_21910F4_hook error: %08x cnt: %08x\n", pfs_mgr_21910F4_hook_err, pfs_mgr_21910F4_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "pfs_mgr_2193B44_hook error: %08x cnt: %08x\n", pfs_mgr_2193B44_hook_err, pfs_mgr_2193B44_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      FILE_GLOBAL_WRITE_LEN("======================================\n");
    }
    close_global_log(); 
  }

  return res;
}

int iofilemgr_BE8444_hook(void* args)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE8444_hook_ref, args);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BE8444_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BE8444_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BE8444_hook_err = (int)res;
      }
    }
  }

  return res;
}

//vfs_node_func15 hook
int iofilemgr_50a63acf_hook(vfs_node* n0, int unk1, int unk2)
{
  int res = TAI_CONTINUE(int, iofilemgr_50a63acf_hook_ref, n0, unk1, unk2);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_50a63acf_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_50a63acf_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_50a63acf_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BE584C_hook(int unk0, int unk1, int unk2, int unk3, vfs_node **node)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE584C_hook_ref, unk0, unk1, unk2, unk3, node);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BE584C_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BE584C_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BE584C_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BE5CC4_hook(vfs_node *ctx, int unk1, int unk2, int unk3, int arg_0)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE5CC4_hook_ref, ctx, unk1, unk2, unk3, arg_0);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BE5CC4_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BE5CC4_hook_err = (int)res;

      if(ctx > 0)
      {
        vfs_mount* mnt = ctx->node;
        if(mnt > 0)
        {
          vfs_add_data* add = mnt->add_data;
          if(add > 0)
          {
            iofilemgr_BE5CC4_hook_ops1 = add->funcs1;
          }
        }

        iofilemgr_BE5CC4_hook_ops2 = ctx->ops;
      }
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BE5CC4_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_f7dac0f5_hook(vfs_mount *mnt, int unk1, int unk2, int unk3, int arg_0, int arg_4, int arg_8)
{
  int res = TAI_CONTINUE(int, iofilemgr_f7dac0f5_hook_ref, mnt, unk1, unk2, unk3, arg_0, arg_4, arg_8);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_f7dac0f5_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_f7dac0f5_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_f7dac0f5_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_a5a6a55c_hook(vfs_node *ctx, int unk1, int unk2, int unk3)
{
  int res = TAI_CONTINUE(int, iofilemgr_a5a6a55c_hook_ref, ctx, unk1, unk2, unk3);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_a5a6a55c_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_a5a6a55c_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_a5a6a55c_hook_err = (int)res;
      }
    }
  }

  return res;
}

//vfs_node_2_func4
int pfs_mgr_21910F4_hook(void* args)
{
  int res = TAI_CONTINUE(int, pfs_mgr_21910F4_hook_ref, args);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    pfs_mgr_21910F4_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      pfs_mgr_21910F4_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        pfs_mgr_21910F4_hook_err = (int)res;
      }
    }
  }

  return res;
}

//icv file related function
int pfs_mgr_2193B44_hook(void* unk0, void* unk1, int num2)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2193B44_hook_ref, unk0, unk1, num2);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    pfs_mgr_2193B44_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      pfs_mgr_2193B44_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        pfs_mgr_2193B44_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BEA704_hook_err = 0;
int iofilemgr_BEA704_hook_err_cnt = 0;

int iofilemgr_9e347c7d_hook_err = 0;
int iofilemgr_9e347c7d_hook_err_cnt = 0;

int iofilemgr_76b79bec_hook_err = 0;
int iofilemgr_76b79bec_hook_err_cnt = 0;

int pfs_mgr_219112C_hook_err = 0;
int pfs_mgr_219112C_hook_err_cnt = 0;

int pfs_mgr_2192190_hook_err = 0;
int pfs_mgr_2192190_hook_err_cnt = 0;

int app_mgr_75192972_hook(const char* name, int flags, SceMode mode)
{
  int res = TAI_CONTINUE(int, app_mgr_75192972_hook_ref, name, flags, mode);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      
      snprintf(sprintfBuffer, 256, "called ksceIoOpen:\nname: %s res: %08x\n", name, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BEA704_hook error: %08x cnt: %08x\n", iofilemgr_BEA704_hook_err, iofilemgr_BEA704_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_a5a6a55c_hook error: %08x cnt: %08x\n", iofilemgr_a5a6a55c_hook_err, iofilemgr_a5a6a55c_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_9e347c7d_hook error: %08x cnt: %08x\n", iofilemgr_9e347c7d_hook_err, iofilemgr_9e347c7d_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_76b79bec_hook error: %08x cnt: %08x\n", iofilemgr_76b79bec_hook_err, iofilemgr_76b79bec_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "pfs_mgr_219112C_hook error: %08x cnt: %08x\n", pfs_mgr_219112C_hook_err, pfs_mgr_219112C_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "pfs_mgr_2192190_hook error: %08x cnt: %08x\n", pfs_mgr_2192190_hook_err, pfs_mgr_2192190_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      FILE_GLOBAL_WRITE_LEN("======================================\n");
    }
    close_global_log(); 
  }

  return res;
}

int iofilemgr_BEA704_hook(void* args)
{
  int res = TAI_CONTINUE(int, iofilemgr_BEA704_hook_ref, args);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BEA704_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BEA704_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BEA704_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_9e347c7d_hook(vfs_node *n0, int unk1, int unk2, int unk3, int arg_0)
{
  int res = TAI_CONTINUE(int, iofilemgr_9e347c7d_hook_ref, n0, unk1, unk2, unk3, arg_0);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_9e347c7d_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_9e347c7d_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_9e347c7d_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_76b79bec_hook(vfs_node *ctx, int unk1, int unk2, int unk3)
{
  int res = TAI_CONTINUE(int, iofilemgr_76b79bec_hook_ref, ctx, unk1, unk2, unk3);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_76b79bec_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_76b79bec_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_76b79bec_hook_err = (int)res;
      }
    }
  }

  return res;
}

//vfs_node_2_func1
int pfs_mgr_219112C_hook(void* args)
{
  int res = TAI_CONTINUE(int, pfs_mgr_219112C_hook_ref, args);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    pfs_mgr_219112C_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      pfs_mgr_219112C_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        pfs_mgr_219112C_hook_err = (int)res;
      }
    }
  }

  return res;
}

int pfs_mgr_2192190_hook(int unk0, int unk1, int unk2, int unk3, int arg_0)
{
  int res = TAI_CONTINUE(int, pfs_mgr_2192190_hook_ref, unk0, unk1, unk2, unk3, arg_0);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    pfs_mgr_2192190_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      pfs_mgr_2192190_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        pfs_mgr_2192190_hook_err = (int)res;
      }
    }
  }

  return res;
}

//---------------

int pfs_mgr_219BF20_hook_err = 0;
int pfs_mgr_219BF20_hook_err_cnt = 0;

int iofilemgr_BE7AEC_hook_err = 0;
int iofilemgr_BE7AEC_hook_err_cnt = 0;

int iofilemgr_BF57F8_hook_err = 0;
int iofilemgr_BF57F8_hook_err_cnt = 0;

int iofilemgr_BF66E4_hook_err = 0;
int iofilemgr_BF66E4_hook_err_cnt = 0;

int iofilemgr_BF66E4_hook_result_error = 0;
int iofilemgr_BF66E4_hook_result_error_cnt = 0;

int iofilemgr_BF6B6C_hook_err = 0;
int iofilemgr_BF6B6C_hook_err_cnt = 0;

int iofilemgr_BF69B4_hook_err = 0;
int iofilemgr_BF69B4_hook_err_cnt = 0;

int iofilemgr_BF651C_hook_err = 0;
int iofilemgr_BF651C_hook_err_cnt = 0;

int iofilemgr_BF651C_hook_ioctl_result = 0;
int iofilemgr_BF651C_hook_ioctl_result_cnt = 0;

int iofilemgr_BF89EC_hook_err = 0;
int iofilemgr_BF89EC_hook_err_cnt = 0;

int iofilemgr_BF8BD4_hook_err = 0;
int iofilemgr_BF8BD4_hook_err_cnt = 0;

int iofilemgr_BF7C00_hook_err;
int iofilemgr_BF7C00_hook_err_cnt;

//

int iofilemgr_BF8BB0_hook_err = 0;
int iofilemgr_BF7288_hook_err = 0;
int iofilemgr_BFB5F0_hook_err = 0;
int iofilemgr_BF88C4_hook_err = 0;
int iofilemgr_BFB794_hook_err = 0;
int iofilemgr_BF7268_hook_err = 0;
int iofilemgr_BF83A0_hook_err = 0;
int iofilemgr_BFB628_hook_err = 0;
int iofilemgr_BF7228_hook_err = 0;
int iofilemgr_BF83F8_hook_err = 0;
int iofilemgr_BFB748_hook_err = 0;

int iofilemgr_BF8BB0_hook_err_cnt = 0;
int iofilemgr_BF7288_hook_err_cnt = 0;
int iofilemgr_BFB5F0_hook_err_cnt = 0;
int iofilemgr_BF88C4_hook_err_cnt = 0;
int iofilemgr_BFB794_hook_err_cnt = 0;
int iofilemgr_BF7268_hook_err_cnt = 0;
int iofilemgr_BF83A0_hook_err_cnt = 0;
int iofilemgr_BFB628_hook_err_cnt = 0;
int iofilemgr_BF7228_hook_err_cnt = 0;
int iofilemgr_BF83F8_hook_err_cnt = 0;
int iofilemgr_BFB748_hook_err_cnt = 0;

int iofilemgr_BFB628_hook_err_r2 = 0;
int iofilemgr_BFB794_hook_ret = 0;
int iofilemgr_BF7C00_hook_ret = 0;

int iofilemgr_BE7F4C_hook_err = 0;
int iofilemgr_BE7F4C_hook_err_cnt = 0;

int iofilemgr_BFB2A8_hook_err = 0;
int iofilemgr_BFB2A8_hook_err_cnt = 0;

int pfs_facade_4238d2d2_hook_err = 0;
int pfs_facade_4238d2d2_hook_err_cnt = 0;

int iofilemgr_0b54f9e0_hook_err = 0;
int iofilemgr_0b54f9e0_hook_err_cnt = 0;

int app_mgr_e17efc03_hook(SceUID fd, char *data, SceSize size)
{
  int res = TAI_CONTINUE(int, app_mgr_e17efc03_hook_ref, fd, data, size);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    open_global_log();
    {
      FILE_GLOBAL_WRITE_LEN("======================================\n");
      
      snprintf(sprintfBuffer, 256, "called sceIoReadForDriver:\nfd: %08x size: %08x res: %08x\n", fd, size, res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "g_pfs_mgr_2199064_hook error: %08x cnt: %08x\n", g_pfs_mgr_2199064_hook_error, g_pfs_mgr_2199064_hook_error_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "pfs_mgr_219BF20_hook error: %08x cnt: %08x\n", pfs_mgr_219BF20_hook_err, pfs_mgr_219BF20_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //sceIoReadForDriver main thread
      snprintf(sprintfBuffer, 256, "iofilemgr_BE7AEC_hook (read) error: %08x cnt: %08x\n", iofilemgr_BE7AEC_hook_err, iofilemgr_BE7AEC_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //this is read alternative that should not be called
      snprintf(sprintfBuffer, 256, "iofilemgr_BF57F8_hook (callback) error: %08x cnt: %08x\n", iofilemgr_BF57F8_hook_err, iofilemgr_BF57F8_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //read alternative subroutne
      snprintf(sprintfBuffer, 256, "iofilemgr_BF66E4_hook error: %08x cnt: %08x\n", iofilemgr_BF66E4_hook_err, iofilemgr_BF66E4_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BF66E4_hook_result error: %08x cnt: %08x\n", iofilemgr_BF66E4_hook_result_error, iofilemgr_BF66E4_hook_result_error_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //read alternative subroutne
      snprintf(sprintfBuffer, 256, "iofilemgr_BF6B6C_hook error: %08x cnt: %08x\n", iofilemgr_BF6B6C_hook_err, iofilemgr_BF6B6C_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //read alternative subroutne
      //snprintf(sprintfBuffer, 256, "iofilemgr_BF69B4_hook error: %08x cnt: %08x\n", iofilemgr_BF69B4_hook_err, iofilemgr_BF69B4_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //ioctl wrapper
      snprintf(sprintfBuffer, 256, "iofilemgr_BF651C_hook (ioctl) error: %08x cnt: %08x\n", iofilemgr_BF651C_hook_err, iofilemgr_BF651C_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BF651C_hook_ioctl_result error: %08x cnt: %08x\n", iofilemgr_BF651C_hook_ioctl_result, iofilemgr_BF651C_hook_ioctl_result_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //io dispatcher
      snprintf(sprintfBuffer, 256, "iofilemgr_BF89EC_hook error: %08x cnt: %08x\n", iofilemgr_BF89EC_hook_err, iofilemgr_BF89EC_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BF8BD4_hook error: %08x cnt: %08x\n", iofilemgr_BF8BD4_hook_err, iofilemgr_BF8BD4_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //main i/o dispatcher
      snprintf(sprintfBuffer, 256, "iofilemgr_BF7C00_hook error: %08x cnt: %08x ret: %08x\n", iofilemgr_BF7C00_hook_err, iofilemgr_BF7C00_hook_err_cnt, iofilemgr_BF7C00_hook_ret);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //

      //snprintf(sprintfBuffer, 256, "iofilemgr_BF8BB0_hook error: %08x cnt: %08x\n", iofilemgr_BF8BB0_hook_err, iofilemgr_BF8BB0_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BF7288_hook error: %08x cnt: %08x\n", iofilemgr_BF7288_hook_err, iofilemgr_BF7288_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BFB5F0_hook error: %08x cnt: %08x\n", iofilemgr_BFB5F0_hook_err, iofilemgr_BFB5F0_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BF88C4_hook error: %08x cnt: %08x\n", iofilemgr_BF88C4_hook_err, iofilemgr_BF88C4_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BFB794_hook error: %08x cnt: %08x ret: %08x\n", iofilemgr_BFB794_hook_err, iofilemgr_BFB794_hook_err_cnt, iofilemgr_BFB794_hook_ret);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BF7268_hook error: %08x cnt: %08x\n", iofilemgr_BF7268_hook_err, iofilemgr_BF7268_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BF83A0_hook error: %08x cnt: %08x\n", iofilemgr_BF83A0_hook_err, iofilemgr_BF83A0_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BFB628_hook error: %08x cnt: %08x err2: %08x\n", iofilemgr_BFB628_hook_err, iofilemgr_BFB628_hook_err_cnt, iofilemgr_BFB628_hook_err_r2);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BF7228_hook error: %08x cnt: %08x\n", iofilemgr_BF7228_hook_err, iofilemgr_BF7228_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BF83F8_hook error: %08x cnt: %08x\n", iofilemgr_BF83F8_hook_err, iofilemgr_BF83F8_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      //snprintf(sprintfBuffer, 256, "iofilemgr_BFB748_hook error: %08x cnt: %08x\n", iofilemgr_BFB748_hook_err, iofilemgr_BFB748_hook_err_cnt);
      //FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BE7F4C_hook error: %08x cnt: %08x\n", iofilemgr_BE7F4C_hook_err, iofilemgr_BE7F4C_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_BFB2A8_hook error: %08x cnt: %08x\n", iofilemgr_BFB2A8_hook_err, iofilemgr_BFB2A8_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "pfs_facade_4238d2d2_hook error: %08x cnt: %08x\n", pfs_facade_4238d2d2_hook_err, pfs_facade_4238d2d2_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      snprintf(sprintfBuffer, 256, "iofilemgr_0b54f9e0_hook error: %08x cnt: %08x\n", iofilemgr_0b54f9e0_hook_err, iofilemgr_0b54f9e0_hook_err_cnt);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);

      FILE_GLOBAL_WRITE_LEN("======================================\n");

      for(int i = 0; i < min(0x100, size); i++)
      {
        char buffer[4];
        snprintf(buffer, 4, "%02x ", data[i]);
        FILE_GLOBAL_WRITE_LEN(buffer);
      }
      FILE_GLOBAL_WRITE_LEN("\n");

      FILE_GLOBAL_WRITE_LEN("======================================\n");

    }
    close_global_log(); 

    iofilemgr_BF651C_hook_err = 0;
    iofilemgr_BF651C_hook_err_cnt = 0;

    iofilemgr_BF651C_hook_ioctl_result = 0;
    iofilemgr_BF651C_hook_ioctl_result_cnt = 0;
  }

  return res;
}

int pfs_mgr_219BF20_hook(int unk0)
{
  int res = TAI_CONTINUE(int, pfs_mgr_219BF20_hook_ref, unk0);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    pfs_mgr_219BF20_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      pfs_mgr_219BF20_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        pfs_mgr_219BF20_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BE7AEC_hook(void* args)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE7AEC_hook_ref, args);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BE7AEC_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BE7AEC_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BE7AEC_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BF57F8_hook(SceUID fd, void *data, SceSize size, void* ptr)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF57F8_hook_ref, fd, data, size, ptr);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BF57F8_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BF57F8_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BF57F8_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BF66E4_hook(io_scheduler_dispatcher *disp, io_context *ioctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF66E4_hook_ref, disp, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BF66E4_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BF66E4_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BF66E4_hook_err = (int)res;
      }
    }
  }

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BF66E4_hook_result_error_cnt++;
    
    if((int)ioctx->op_result == 0x80140F02)
    {
      iofilemgr_BF66E4_hook_result_error = (int)ioctx->op_result;
    }
    else
    {
      if((int)ioctx->op_result < 0)
      {
        iofilemgr_BF66E4_hook_result_error = (int)ioctx->op_result;
      }
    }
  }

  return res;
}

int iofilemgr_BF6B6C_hook(io_context *ioctx, int	ioctlOutdata1, int ioctlOutdata0, int async, int arg_0_in, int arg_4_in, void *pool_A0,	int num, SceUID	fd, void *data,	SceSize	size)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF6B6C_hook_ref, ioctx, ioctlOutdata1, ioctlOutdata0, async, arg_0_in, arg_4_in, pool_A0, num, fd, data, size);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BF6B6C_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BF6B6C_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BF6B6C_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BF69B4_hook(io_context *ioctx, int io_op_index, int async, int unk3, int arg_0, io_device_code ioctlOutdata1,	int ioctlOutdata0, void	*pool_A0, int num)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF69B4_hook_ref, ioctx, io_op_index, async, unk3, arg_0, ioctlOutdata1, ioctlOutdata0, pool_A0, num);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BF69B4_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BF69B4_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BF69B4_hook_err = (int)res;
      }
    }
  }

  return res;
}

int iofilemgr_BF651C_hook(SceUID fd, void *outdata, int indataValue)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF651C_hook_ref, fd, outdata, indataValue);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BF651C_hook_err_cnt++;
    
    if((int)res == 0x8001008f)
    {
      iofilemgr_BF651C_hook_err = (int)res;
    }
    else
    {
      if((int)res < 0)
      {
        iofilemgr_BF651C_hook_err = (int)res;
      }
    }
  }

  if(log_iofilemgr_in_app_mgr > 0)
  {
    int* dataItems = (int*)outdata;
    int item1 = dataItems[1];

    if(item1 >= 0)
    {
      short uxth = item1 & 0x0000FFFF;
      if(uxth != 0x400)
      {
        iofilemgr_BF651C_hook_ioctl_result = 1;
        iofilemgr_BF651C_hook_ioctl_result_cnt++;
      }
    }
  }

  return res;
}

int iofilemgr_BF89EC_hook(io_scheduler *shed, io_context *ioctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF89EC_hook_ref, shed, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF89EC_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF89EC_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF89EC_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BF8BD4_hook(io_scheduler *shed, io_context *ioctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF8BD4_hook_ref, shed, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF8BD4_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF8BD4_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF8BD4_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

//main i/o dispatcher
int iofilemgr_BF7C00_hook(io_context *ioctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF7C00_hook_ref, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF7C00_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF7C00_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF7C00_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(res == 0x80140F02)
    {
      iofilemgr_BF7C00_hook_ret = res;
    }
    else
    {
      if(res < 0)
      {
        iofilemgr_BF7C00_hook_ret = res;
      }
    }
  }

  return res;
}

//

int iofilemgr_BF8BB0_hook(io_scheduler *shed, io_context *ioctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF8BB0_hook_ref, shed, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF8BB0_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF8BB0_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF8BB0_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BF7288_hook(io_context *ioctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF7288_hook_ref, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF7288_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF7288_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF7288_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BFB5F0_hook(io_scheduler *shed, io_context *ioctx, int num)
{
  int res = TAI_CONTINUE(int, iofilemgr_BFB5F0_hook_ref, shed, ioctx, num);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BFB5F0_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BFB5F0_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BFB5F0_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BF88C4_hook(io_context *ioctx, int *resptr)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF88C4_hook_ref, ioctx, resptr);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF88C4_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF88C4_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF88C4_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BFB794_hook(io_context *ioctx, int unk)
{
  int prev_err = -1;
  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      prev_err = (int)ioctx->op_result;
    }
  }

  int res = TAI_CONTINUE(int, iofilemgr_BFB794_hook_ref, ioctx, unk);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      if(prev_err == 0) //check first time set
      {
        iofilemgr_BFB794_hook_err_cnt++;
        
        if((int)ioctx->op_result == 0x80140F02)
        {
          iofilemgr_BFB794_hook_err = (int)ioctx->op_result;
        }
        else
        {
          if((int)ioctx->op_result < 0)
          {
            iofilemgr_BFB794_hook_err = (int)ioctx->op_result;
          }
        }
      }
    }
  }

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(res == 0x80140F02)
    {
      iofilemgr_BFB794_hook_ret = res;
    }
    else
    {
      if(res < 0)
      {
        iofilemgr_BFB794_hook_ret = res;
      }
    }
  }

  return res;
}

int iofilemgr_BF7268_hook(io_context *ioctx, void *ptr1)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF7268_hook_ref, ioctx, ptr1);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF7268_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF7268_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF7268_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BF83A0_hook(int *unk0, int *unk1, io_context *ioctx)
{
  int prev_err = -1;
  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      prev_err = (int)ioctx->op_result;
    }
  }

  int res = TAI_CONTINUE(int, iofilemgr_BF83A0_hook_ref, unk0, unk1, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      if(prev_err == 0) //check first time set
      {
        iofilemgr_BF83A0_hook_err_cnt++;
        
        if((int)ioctx->op_result == 0x80140F02)
        {
          iofilemgr_BF83A0_hook_err = (int)ioctx->op_result;
        }
        else
        {
          if((int)ioctx->op_result < 0)
          {
            iofilemgr_BF83A0_hook_err = (int)ioctx->op_result;
          }
        }
      }
    }
  }

  return res;
}

int iofilemgr_BFB628_hook(io_context *ioctx, int unk1, int err)
{
  int err_orig = err;

  int prev_err = -1;
  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      prev_err = (int)ioctx->op_result;
    }
  }

  int res = TAI_CONTINUE(int, iofilemgr_BFB628_hook_ref, ioctx, unk1, err);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      if(prev_err == 0) //check first time set
      {
        iofilemgr_BFB628_hook_err_cnt++;
        
        if((int)ioctx->op_result == 0x80140F02)
        {
          iofilemgr_BFB628_hook_err = (int)ioctx->op_result;
        }
        else
        {
          if((int)ioctx->op_result < 0)
          {
            iofilemgr_BFB628_hook_err = (int)ioctx->op_result;
          }
        }
      }
    }
  }

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(prev_err == 0) //check first time set
    {
      if(err_orig == 0x80140F02)
      {
        iofilemgr_BFB628_hook_err_r2 = err_orig;
      }
      else
      {
        if(err_orig < 0)
        {
          iofilemgr_BFB628_hook_err_r2 = err_orig;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BF7228_hook(io_context *ioctx)
{
  int prev_err = -1;
  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      prev_err = (int)ioctx->op_result;
    }
  }

  int res = TAI_CONTINUE(int, iofilemgr_BF7228_hook_ref, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      if(prev_err == 0)
      {
        iofilemgr_BF7228_hook_err_cnt++;
        
        if((int)ioctx->op_result == 0x80140F02)
        {
          iofilemgr_BF7228_hook_err = (int)ioctx->op_result;
        }
        else
        {
          if((int)ioctx->op_result < 0)
          {
            iofilemgr_BF7228_hook_err = (int)ioctx->op_result;
          }
        }
      }
    }
  }

  return res;
}

int iofilemgr_BF83F8_hook(io_scheduler *shed, io_context *ioctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BF83F8_hook_ref, shed, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      iofilemgr_BF83F8_hook_err_cnt++;
      
      if((int)ioctx->op_result == 0x80140F02)
      {
        iofilemgr_BF83F8_hook_err = (int)ioctx->op_result;
      }
      else
      {
        if((int)ioctx->op_result < 0)
        {
          iofilemgr_BF83F8_hook_err = (int)ioctx->op_result;
        }
      }
    }
  }

  return res;
}

int iofilemgr_BFB748_hook(io_scheduler *shed, io_context *ioctx)
{
  int prev_err = -1;
  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      prev_err = (int)ioctx->op_result;
    }
  }

  int res = TAI_CONTINUE(int, iofilemgr_BFB748_hook_ref, shed, ioctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    if(ioctx > 0)
    {
      if(prev_err == 0)
      {
        iofilemgr_BFB748_hook_err_cnt++;
        
        if((int)ioctx->op_result == 0x80140F02)
        {
          iofilemgr_BFB748_hook_err = (int)ioctx->op_result;
        }
        else
        {
          if((int)ioctx->op_result < 0)
          {
            iofilemgr_BFB748_hook_err = (int)ioctx->op_result;
          }
        }
      }
    }
  }

  return res;
}

int iofilemgr_BE7F4C_hook(sceIoLseekForDriver_args *ctx)
{
  int res = TAI_CONTINUE(int, iofilemgr_BE7F4C_hook_ref, ctx);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BE7F4C_hook_err_cnt++;
    
    if(res == 0x80140F02)
    {
      iofilemgr_BE7F4C_hook_err = res;
    }
    else
    {
      if(res < 0)
      {
        iofilemgr_BE7F4C_hook_err = res;
      }
    }
  }

  return res;
}

int iofilemgr_BFB2A8_hook(io_context *ioctx, void* param)
{
  int res = TAI_CONTINUE(int, iofilemgr_BFB2A8_hook_ref, ioctx, param);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_BFB2A8_hook_err_cnt++;
    
    if(res == 0x80140F02)
    {
      iofilemgr_BFB2A8_hook_err = res;
    }
    else
    {
      if(res < 0)
      {
        iofilemgr_BFB2A8_hook_err = res;
      }
    }
  }

  return res;
}

//pfs pread impl
int pfs_facade_4238d2d2_hook(sceIoPreadForDriver_args* args)
{
  int res = TAI_CONTINUE(int, pfs_facade_4238d2d2_hook_ref, args);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    pfs_facade_4238d2d2_hook_err_cnt++;
    
    if(res == 0x80140F02)
    {
      pfs_facade_4238d2d2_hook_err = res;
    }
    else
    {
      if(res < 0)
      {
        pfs_facade_4238d2d2_hook_err = res;
      }
    }
  }

  return res;
}

int iofilemgr_0b54f9e0_hook(sceIoPreadForDriver_args *args)
{
  int res = TAI_CONTINUE(int, iofilemgr_0b54f9e0_hook_ref, args);

  if(log_iofilemgr_in_app_mgr > 0)
  {
    iofilemgr_0b54f9e0_hook_err_cnt++;
    
    if(res == 0x80140F02)
    {
      iofilemgr_0b54f9e0_hook_err = res;
    }
    else
    {
      if(res < 0)
      {
        iofilemgr_0b54f9e0_hook_err = res;
      }
    }
  }

  return res;
}