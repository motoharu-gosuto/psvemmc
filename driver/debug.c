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

char sprintfBuffer[256];

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

#define ENABLE_SD_PATCHES

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

      gen_read_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &gen_read_hook_ref, sdstor_info.modid, 0, 0x2AF4, 1, gen_read_hook); //0xC16AF4

      sd_read_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sd_read_hook_ref, "SceSdstor", SceSdifForDriver_NID, 0xb9593652, sd_read_hook);

      init_partition_table_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &init_partition_table_hook_ref, sdstor_info.modid, 0, 0x36C, 1, init_partition_table_hook); //0xC1436C

      create_device_handle_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &create_device_handle_hook_ref, sdstor_info.modid, 0, 0x80, 1, create_device_handle); //0xC14080

      #ifdef ENABLE_SD_PATCHES
      //different patches of zero proc

      //patch for proc_initialize_generic_2 - so that sd card type is not ignored
      char zeroCallOnePatch[4] = {0x01, 0x20, 0x00, 0xBF};
      gen_init_2_patch_uid = taiInjectDataForKernel(KERNEL_PID, sdstor_info.modid, 0, 0x2498, zeroCallOnePatch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP)

      //this patch is not required but it is much easier to replace BLX with this code that does not do anything
      //since BLX call of zero proc is logged, and I want to identify other places that might require patching
      char zeroCallZeroPatch[4] = {0x00, 0x20, 0x00, 0xBF};
      mbr_init_zero_patch1_uid = taiInjectDataForKernel(KERNEL_PID, sdstor_info.modid, 0, 0x10B0, zeroCallZeroPatch, 4); //patch of zero proc call inside mbr init function
      mbr_init_zero_patch2_uid = taiInjectDataForKernel(KERNEL_PID, sdstor_info.modid, 0, 0x10C0, zeroCallZeroPatch, 4); //patch of zero proc call inside mbr init function

     //patch for proc_initialize_generic_1 - so that sd card type is not ignored
     gen_init_1_patch_uid = taiInjectDataForKernel(KERNEL_PID, sdstor_info.modid, 0, 0x2022, zeroCallOnePatch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP)
     #endif
  }
  
  tai_module_info_t sdif_info;
  sdif_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdif", &sdif_info) >= 0)
  {
    init_mmc_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &init_mmc_hook_ref, "SceSdif", SceSdifForDriver_NID, 0x22c82e79, init_mmc_hook);
    init_sd_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &init_sd_hook_ref, "SceSdif", SceSdifForDriver_NID, 0xc1271539, init_sd_hook);
    
    char iterations[1] = {20};
      
    patch_uids[0] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x349A, iterations, 1); //patch (MOVS R2, 5) to (MOVS R2, 10)
    patch_uids[1] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x34B8, iterations, 1); //patch (MOVS R2, 5) to (MOVS R2, 10)
    patch_uids[2] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x359A, iterations, 1); //patch (MOVS R2, 5) to (MOVS R2, 10)
    
    cmd55_41_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &cmd55_41_hook_ref, sdif_info.modid, 0, 0x35E8, 1, cmd55_41_hook);

    send_command_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &send_command_hook_ref, sdif_info.modid, 0, 0x17E8, 1, send_command_hook);

    #ifdef ENABLE_SD_PATCHES
    
    //pages 35, 66
    
    
    char lowSpeed_check[4] = {0xF0, 0xFF, 0xFF, 0x00};
    hs_dis_patch1_uid = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x6B34, lowSpeed_check, 4); //data:00C6EB30 data_CMD6_06000000_00FFFFF1 DCB 6, 0, 0, 0, 0xF1, 0xFF, 0xFF, 0x00

    char lowSpeed_set[4] = {0xF0, 0xFF, 0xFF, 0x80};
    hs_dis_patch2_uid = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x6B54, lowSpeed_set, 4); //data:00C6EB50 data_CMD6_06000000_80FFFFF1 DCB 6, 0, 0, 0, 0xF1, 0xFF, 0xFF, 0x80
    

    char busWidth[1] = {0x02}; // for now - leaving it as it is 2 (4 bit transfer). 0 (1 bit transfer) does not work
    bus_size_patch_uid = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x6826, busWidth, 1); //patch (MOVS R3, #2) to (MOVS R3, #0)
    #endif
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
    snprintf(sprintfBuffer, 256, "failed to set gc init hook: %x\n", gc_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(init_mmc_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mmc init hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mmc init hook: %x\n", init_mmc_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(init_sd_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sd init hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sd init hook: %x\n", init_sd_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(cmd55_41_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set cmd55_41_hook_id hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set cmd55_41_hook_id hook: %x\n", cmd55_41_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(gen_init_hook_uids[0] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 1 hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen init 1 hook: %x\n", gen_init_hook_uids[0]);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(gen_init_hook_uids[1] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 2 hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen init 2 hook: %x\n", gen_init_hook_uids[1]);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(gen_init_hook_uids[2] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 3 hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen init 3 hook: %x\n", gen_init_hook_uids[2]);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(sysroot_zero_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sysroot zero hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sysroot zero hook: %x\n", sysroot_zero_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(load_mbr_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set load mbr hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set load mbr hook: %x\n", load_mbr_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(mnt_pnt_chk_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mnt ptr chk hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mnt ptr chk hook: %x\n", mnt_pnt_chk_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(mbr_table_init_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mbr table init hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mbr table init hook: %x\n", mbr_table_init_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  for(int f = 0; f < 13; f++)
  {
    if(sdstor_dev_fs_ids[f] >= 0)
    {
      snprintf(sprintfBuffer, 256, "set sdstor_dev_fs function %d hook\n", (f + 1));
      FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    }
    else
    {
      snprintf(sprintfBuffer, 256, "failed to set sdstor_dev_fs function %d hook: %x\n", (f + 1), sdstor_dev_fs_ids[f]);
      FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    }
  }

  if(sceVfsMount_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsMount hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsMount hook: %x\n", sceVfsMount_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(sceVfsAddVfs_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsAddVfs hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsAddVfs hook: %x\n", sceVfsAddVfs_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(sceVfsUnmount_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsUnmount hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsUnmount hook: %x\n", sceVfsUnmount_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(sceVfsDeleteVfs_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsDeleteVfs hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsDeleteVfs hook: %x\n", sceVfsDeleteVfs_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  
  if(sceVfsGetNewNode_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceVfsGetNewNode hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsGetNewNode hook: %x\n", sceVfsGetNewNode_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(gen_init_2_patch_uid >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen_init_2_patch hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen_init_2_patch hook: %x\n", gen_init_2_patch_uid);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(gen_init_1_patch_uid >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen_init_1_patch hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen_init_1_patch hook: %x\n", gen_init_1_patch_uid);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(gen_read_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen_read hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen_read hook: %x\n", gen_read_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(sd_read_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sd_read hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sd_read hook: %x\n", sd_read_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(hs_dis_patch1_uid >= 0)
  {
    FILE_WRITE(global_log_fd, "set hs_dis_patch1\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set hs_dis_patch1: %x\n", hs_dis_patch1_uid);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(hs_dis_patch2_uid >= 0)
  {
    FILE_WRITE(global_log_fd, "set hs_dis_patch2\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set hs_dis_patch2: %x\n", hs_dis_patch2_uid);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(bus_size_patch_uid >= 0)
  {
    FILE_WRITE(global_log_fd, "set bus_size_patch\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set bus_size_patch: %x\n", bus_size_patch_uid);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(mbr_init_zero_patch1_uid >= 0)
  {
    FILE_WRITE(global_log_fd, "set mbr_init_zero_patch1\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mbr_init_zero_patch1: %x\n", mbr_init_zero_patch1_uid);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(mbr_init_zero_patch2_uid >= 0)
  {
    FILE_WRITE(global_log_fd, "set mbr_init_zero_patch2\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mbr_init_zero_patch2: %x\n", mbr_init_zero_patch2_uid);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(init_partition_table_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set init_partition_table_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set init_partition_table_hook: %x\n", init_partition_table_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(create_device_handle_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set create_device_handle_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set create_device_handle_hook: %x\n", create_device_handle_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }

  if(send_command_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set send_command_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set send_command_hook: %x\n", send_command_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
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

  if(gen_init_2_patch_uid >= 0)
    taiInjectReleaseForKernel(gen_init_2_patch_uid);

  if(gen_init_1_patch_uid >= 0)
    taiInjectReleaseForKernel(gen_init_1_patch_uid);

  if(gen_read_hook_id >= 0)
    taiHookReleaseForKernel(gen_read_hook_id, gen_read_hook_ref);

  if(sd_read_hook_id >= 0)
    taiHookReleaseForKernel(sd_read_hook_id, sd_read_hook_ref);

  if(hs_dis_patch1_uid >= 0)
    taiInjectReleaseForKernel(hs_dis_patch1_uid);

  if(hs_dis_patch2_uid >= 0)
    taiInjectReleaseForKernel(hs_dis_patch2_uid);

  if(bus_size_patch_uid >= 0)
    taiInjectReleaseForKernel(bus_size_patch_uid);

  if(mbr_init_zero_patch1_uid >= 0)
    taiInjectReleaseForKernel(mbr_init_zero_patch1_uid);

  if(mbr_init_zero_patch2_uid >= 0)
    taiInjectReleaseForKernel(mbr_init_zero_patch2_uid);

  if(init_partition_table_hook_id >= 0)
    taiHookReleaseForKernel(init_partition_table_hook_id, init_partition_table_hook_ref);

  if(create_device_handle_hook_id >= 0)
    taiHookReleaseForKernel(create_device_handle_hook_id, create_device_handle_hook_ref);

  if(send_command_hook_id >= 0)
    taiHookReleaseForKernel(send_command_hook_id, send_command_hook_ref);

  return 0;
}
