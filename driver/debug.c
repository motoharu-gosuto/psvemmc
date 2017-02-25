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
#include "hooks_vfs_ops.h"
#include "hooks_vfs_funcs.h"

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
