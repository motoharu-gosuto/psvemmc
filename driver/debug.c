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

#include "defines.h"

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
#define SceSblGcAuthMgrDrmBBForDriver_NID 0x1926B182
#define SceSdifForDriver_NID 0x96D306FA
#define SceSysrootForDriver_NID 0x2ED7F97A
#define SceIofilemgrForDriver_NID 0x40FD29C7
#define SceIofilemgr_NID 0xF2FF276E
#define SceError_NID 0x5CD2CAD1
#define SceThreadmgrForDriver_NID 0xE2C40624
#define SceThreadmgr_NID 0x859A24B1
#define SceErrorUser_NID 0xD401318D
#define SceVshBridge_NID 0x35C5ACD4

int initialize_all_hooks()
{
  tai_module_info_t appmgr_info;
  appmgr_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceAppMgr", &appmgr_info) >= 0)
  {
    //hook that contains drm checks for game data mount
    appmgr_23D642C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &appmgr_23D642C_hook_ref, appmgr_info.modid, 0, 0x1642C, 1, appmgr_23D642C_hook);
  }

  tai_module_info_t gc_auth_info;
  gc_auth_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSblGcAuthMgr", &gc_auth_info) >= 0)
  {
    //export does not work
    //gc_22fd5d23_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &gc_22fd5d23_hook_ref, "SceSblGcAuthMgr", SceSblGcAuthMgrDrmBBForDriver_NID, 0x22fd5d23, gc_22fd5d23_hook);

    //import does work
    gc_22fd5d23_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &gc_22fd5d23_hook_ref, "SceNpDrm", SceSblGcAuthMgrDrmBBForDriver_NID, 0x22fd5d23, gc_22fd5d23_hook);
  }

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

      #ifdef ENABLE_SD_PATCHES
      sd_read_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sd_read_hook_ref, "SceSdstor", SceSdifForDriver_NID, 0xb9593652, sd_read_hook);
      #endif

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
    
    #ifdef ENABLE_LOW_SPEED_PATCH
    char lowSpeed_check[4] = {0xF0, 0xFF, 0xFF, 0x00};
    hs_dis_patch1_uid = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x6B34, lowSpeed_check, 4); //data:00C6EB30 data_CMD6_06000000_00FFFFF1 DCB 6, 0, 0, 0, 0xF1, 0xFF, 0xFF, 0x00

    char lowSpeed_set[4] = {0xF0, 0xFF, 0xFF, 0x80};
    hs_dis_patch2_uid = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x6B54, lowSpeed_set, 4); //data:00C6EB50 data_CMD6_06000000_80FFFFF1 DCB 6, 0, 0, 0, 0xF1, 0xFF, 0xFF, 0x80
    #endif

    char busWidth[1] = {0x02}; // for now - leaving it as it is 2 (4 bit transfer). 0 (1 bit transfer) does not work
    bus_size_patch_uid = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x6826, busWidth, 1); //patch (MOVS R3, #2) to (MOVS R3, #0)
    #endif
  }
  
  tai_module_info_t sysmem_info;
  sysmem_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSysmem", &sysmem_info) >= 0)
  {
    //sysroot_zero_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sysroot_zero_hook_ref, "SceSysmem", SceSysrootForDriver_NID, 0xf804f761, sysroot_zero_hook);
    
    //by some reason only import hook worked
    sysroot_zero_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sysroot_zero_hook_ref, "SceSdstor", SceSysrootForDriver_NID, 0xf804f761, sysroot_zero_hook);

    //debug_printf_callback_invoke_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &debug_printf_callback_invoke_ref, sysmem_info.modid, 0, 0x19FA8, 1, debug_printf_callback_invoke_hook);
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

    //does not work
    //sceIoOpenForDriver_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceIoOpenForDriver_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x75192972, sceIoOpenForDriver_hook);

    //user hook
    #ifdef ENABLE_IO_FILE_OPEN_LOG
    sceIoOpenForDriver_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceIoOpenForDriver_hook_ref, "SceIofilemgr", SceIofilemgr_NID, 0xCC67B6FD, sceIoOpenForDriver_hook);
    #endif
  }
 
  tai_module_info_t err_info;
  err_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceError", &err_info) >= 0)
  {
    sceErrorHistoryPostError_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceErrorHistoryPostError_hook_ref, "SceError", SceError_NID, 0x70F9D872, sceErrorHistoryPostError_hook);

    sceErrorHistoryUpdateSequenceInfo_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceErrorHistoryUpdateSequenceInfo_hook_ref, "SceError", SceError_NID, 0x6FBE4BDC, sceErrorHistoryUpdateSequenceInfo_hook);

    sceErrorGetExternalString_kernel_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceErrorGetExternalString_kernel_hook_ref, "SceError", SceError_NID, 0x85747003, sceErrorGetExternalString_kernel_hook);

    sceErrorHistoryGetError_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceErrorHistoryGetError_hook_ref, "SceError", SceError_NID, 0xF16DF981, sceErrorHistoryGetError_hook);
  }

  tai_module_info_t thread_info;
  thread_info.size = sizeof(tai_module_info_t);
  if(taiGetModuleInfoForKernel(KERNEL_PID, "SceKernelThreadMgr", &thread_info) >= 0)
  {
    //ksceKernelCreateThread_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelCreateThread_hook_ref, "SceKernelThreadMgr", SceThreadmgrForDriver_NID, 0xC6674E7D, ksceKernelCreateThread_hook);

    //sceKernelCreateThreadForUser_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceKernelCreateThreadForUser_hook_ref, "SceKernelThreadMgr", SceThreadmgr_NID, 0xC0FAF6A3, sceKernelCreateThreadForUser_hook);

    //---

    //ksceKernelWaitSema_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelWaitSema_hook_ref, "SceKernelThreadMgr", SceThreadmgrForDriver_NID, 0x3C8B55A9, ksceKernelWaitSema_hook);

    //ksceKernelSignalSema_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelSignalSema_hook_ref, "SceKernelThreadMgr", SceThreadmgrForDriver_NID, 0xD270498B, ksceKernelSignalSema_hook);
  }

  tai_module_info_t vsh_info;
  vsh_info.size = sizeof(tai_module_info_t);
  if(taiGetModuleInfoForKernel(KERNEL_PID, "SceVshBridge", &vsh_info) >= 0)
  {
    vshSblAuthMgrVerifySpsfo_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &vshSblAuthMgrVerifySpsfo_hook_ref, "SceVshBridge", SceVshBridge_NID, 0xBA7BDD18, vshSblAuthMgrVerifySpsfo_hook);
  }
  
  open_global_log();
  if(gc_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gc init hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gc init hook: %x\n", gc_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(init_mmc_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set mmc init hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mmc init hook: %x\n", init_mmc_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(init_sd_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sd init hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sd init hook: %x\n", init_sd_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(cmd55_41_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set cmd55_41_hook_id hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set cmd55_41_hook_id hook: %x\n", cmd55_41_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(gen_init_hook_uids[0] >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gen init 1 hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen init 1 hook: %x\n", gen_init_hook_uids[0]);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(gen_init_hook_uids[1] >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gen init 2 hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen init 2 hook: %x\n", gen_init_hook_uids[1]);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(gen_init_hook_uids[2] >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gen init 3 hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen init 3 hook: %x\n", gen_init_hook_uids[2]);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(sysroot_zero_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sysroot zero hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sysroot zero hook: %x\n", sysroot_zero_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(load_mbr_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set load mbr hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set load mbr hook: %x\n", load_mbr_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(mnt_pnt_chk_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set mnt ptr chk hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mnt ptr chk hook: %x\n", mnt_pnt_chk_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(mbr_table_init_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set mbr table init hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mbr table init hook: %x\n", mbr_table_init_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  for(int f = 0; f < 13; f++)
  {
    if(sdstor_dev_fs_ids[f] >= 0)
    {
      snprintf(sprintfBuffer, 256, "set sdstor_dev_fs function %d hook\n", (f + 1));
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    else
    {
      snprintf(sprintfBuffer, 256, "failed to set sdstor_dev_fs function %d hook: %x\n", (f + 1), sdstor_dev_fs_ids[f]);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
  }

  if(sceVfsMount_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceVfsMount hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsMount hook: %x\n", sceVfsMount_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceVfsAddVfs_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceVfsAddVfs hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsAddVfs hook: %x\n", sceVfsAddVfs_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceVfsUnmount_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceVfsUnmount hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsUnmount hook: %x\n", sceVfsUnmount_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceVfsDeleteVfs_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceVfsDeleteVfs hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsDeleteVfs hook: %x\n", sceVfsDeleteVfs_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(sceVfsGetNewNode_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceVfsGetNewNode hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceVfsGetNewNode hook: %x\n", sceVfsGetNewNode_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(gen_init_2_patch_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gen_init_2_patch hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen_init_2_patch hook: %x\n", gen_init_2_patch_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(gen_init_1_patch_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gen_init_1_patch hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen_init_1_patch hook: %x\n", gen_init_1_patch_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(gen_read_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gen_read hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gen_read hook: %x\n", gen_read_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sd_read_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sd_read hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sd_read hook: %x\n", sd_read_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(hs_dis_patch1_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set hs_dis_patch1\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set hs_dis_patch1: %x\n", hs_dis_patch1_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(hs_dis_patch2_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set hs_dis_patch2\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set hs_dis_patch2: %x\n", hs_dis_patch2_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(bus_size_patch_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set bus_size_patch\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set bus_size_patch: %x\n", bus_size_patch_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(mbr_init_zero_patch1_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set mbr_init_zero_patch1\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mbr_init_zero_patch1: %x\n", mbr_init_zero_patch1_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(mbr_init_zero_patch2_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set mbr_init_zero_patch2\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set mbr_init_zero_patch2: %x\n", mbr_init_zero_patch2_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(init_partition_table_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set init_partition_table_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set init_partition_table_hook: %x\n", init_partition_table_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(create_device_handle_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set create_device_handle_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set create_device_handle_hook: %x\n", create_device_handle_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(send_command_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set send_command_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set send_command_hook: %x\n", send_command_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceErrorHistoryPostError_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceErrorHistoryPostError_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceErrorHistoryPostError_hook: %x\n", sceErrorHistoryPostError_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceErrorHistoryUpdateSequenceInfo_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceErrorHistoryUpdateSequenceInfo_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceErrorHistoryUpdateSequenceInfo_hook: %x\n", sceErrorHistoryUpdateSequenceInfo_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceErrorGetExternalString_kernel_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceErrorGetExternalString_kernel_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceErrorGetExternalString_kernel_hook: %x\n", sceErrorGetExternalString_kernel_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceErrorHistoryGetError_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceErrorHistoryGetError_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceErrorHistoryGetError_hook: %x\n", sceErrorHistoryGetError_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(ksceKernelCreateThread_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set ksceKernelCreateThread_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set ksceKernelCreateThread_hook: %x\n", ksceKernelCreateThread_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceKernelCreateThreadForUser_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceKernelCreateThreadForUser_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceKernelCreateThreadForUser_hook: %x\n", sceKernelCreateThreadForUser_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceIoOpenForDriver_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceIoOpenForDriver_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceIoOpenForDriver_hook: %x\n", sceIoOpenForDriver_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(ksceKernelWaitSema_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set ksceKernelWaitSema_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set ksceKernelWaitSema_hook: %x\n", ksceKernelWaitSema_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(ksceKernelSignalSema_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set ksceKernelSignalSema_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set ksceKernelSignalSema_hook: %x\n", ksceKernelSignalSema_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(vshSblAuthMgrVerifySpsfo_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set vshSblAuthMgrVerifySpsfo_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set vshSblAuthMgrVerifySpsfo_hook: %x\n", vshSblAuthMgrVerifySpsfo_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(debug_printf_callback_invoke_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set debug_printf_callback_invoke_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set debug_printf_callback_invoke_hook: %x\n", debug_printf_callback_invoke_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(gc_22fd5d23_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set gc_22fd5d23_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set gc_22fd5d23_hook: %x\n", gc_22fd5d23_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(appmgr_23D642C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set appmgr_23D642C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set appmgr_23D642C_hook: %x\n", appmgr_23D642C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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

  if(sceErrorHistoryPostError_hook_id >= 0)
    taiHookReleaseForKernel(sceErrorHistoryPostError_hook_id, sceErrorHistoryPostError_hook_ref);

  if(sceErrorHistoryUpdateSequenceInfo_hook_id >= 0)
    taiHookReleaseForKernel(sceErrorHistoryUpdateSequenceInfo_hook_id, sceErrorHistoryUpdateSequenceInfo_hook_ref);

  if(sceErrorGetExternalString_kernel_hook_id >= 0)
    taiHookReleaseForKernel(sceErrorGetExternalString_kernel_hook_id, sceErrorGetExternalString_kernel_hook_ref);

  if(sceErrorHistoryGetError_hook_id >= 0)
    taiHookReleaseForKernel(sceErrorHistoryGetError_hook_id, sceErrorHistoryGetError_hook_ref);

  if(ksceKernelCreateThread_hook_id >= 0)
    taiHookReleaseForKernel(ksceKernelCreateThread_hook_id, ksceKernelCreateThread_hook_ref);

  if(sceKernelCreateThreadForUser_hook_id >= 0)
    taiHookReleaseForKernel(sceKernelCreateThreadForUser_hook_id, sceKernelCreateThreadForUser_hook_ref);

  if(sceIoOpenForDriver_hook_id >= 0)
    taiHookReleaseForKernel(sceIoOpenForDriver_hook_id, sceIoOpenForDriver_hook_ref);

  if(ksceKernelWaitSema_hook_id >= 0)
    taiHookReleaseForKernel(ksceKernelWaitSema_hook_id, ksceKernelWaitSema_hook_ref);

  if(ksceKernelSignalSema_hook_id >= 0)
    taiHookReleaseForKernel(ksceKernelSignalSema_hook_id, ksceKernelSignalSema_hook_ref);

  if(vshSblAuthMgrVerifySpsfo_hook_id >= 0)
    taiHookReleaseForKernel(vshSblAuthMgrVerifySpsfo_hook_id, vshSblAuthMgrVerifySpsfo_hook_ref);

  if(debug_printf_callback_invoke_id >= 0)
    taiHookReleaseForKernel(debug_printf_callback_invoke_id, debug_printf_callback_invoke_ref);

  if(gc_22fd5d23_hook_id >= 0)
    taiHookReleaseForKernel(gc_22fd5d23_hook_id, gc_22fd5d23_hook_ref);

  if(appmgr_23D642C_hook_id >= 0)
    taiHookReleaseForKernel(appmgr_23D642C_hook_id, appmgr_23D642C_hook_ref);

  return 0;
}
