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

#define CONCAT(a, b) a ## b

#define CONCAT3(a, b, c) a ## b ## c

#define TOKEN_TO_STRING(token) #token

#define RELEASE_HOOK(name) if(CONCAT(name, _id)  >= 0) \
                             taiHookReleaseForKernel(CONCAT(name, _id), CONCAT(name, _ref));

#define RELEASE_INJECT(name) if(CONCAT(name, _uid)  >= 0) \
                               taiInjectReleaseForKernel(CONCAT(name, _uid));

#define PRINT_HOOK(name) if(CONCAT(name, _id) >= 0){\
                           FILE_GLOBAL_WRITE_LEN(TOKEN_TO_STRING(CONCAT3(set , name, \n)));}\
                         else{\
                           snprintf(sprintfBuffer, 256, TOKEN_TO_STRING(CONCAT3(failed to set ,name,: %x\n)), CONCAT(name, _id));\
                           FILE_GLOBAL_WRITE_LEN(sprintfBuffer);}

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
#define SceAppMgrForDriver_NID 0xDCE180F8
#define SceSdifForDriver_NID 0x96D306FA
#define SceSysrootForDriver_NID 0x2ED7F97A
#define SceIofilemgrForDriver_NID 0x40FD29C7
#define SceIofilemgr_NID 0xF2FF276E
#define SceError_NID 0x5CD2CAD1
#define SceThreadmgrForDriver_NID 0xE2C40624
#define SceThreadmgr_NID 0x859A24B1
#define SceErrorUser_NID 0xD401318D
#define SceVshBridge_NID 0x35C5ACD4
#define SceFios2KernelForDriver_NID 0x54D6B9EB
#define SceSblACMgrForDriver_NID 0x9AD8E213
#define ScePfsMgrForKernel_NID 0xA067B56F
#define ScePfsFacadeForKernel_NID 0xC26DC7BD

int initialize_all_hooks()
{
  tai_module_info_t pfsmgr_info;
  pfsmgr_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "ScePfsMgr", &pfsmgr_info) >= 0)
  {
    pfs_mgr_2d48aea2_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &pfs_mgr_2d48aea2_hook_ref, "ScePfsMgr", ScePfsMgrForKernel_NID, 0x2d48aea2, pfs_mgr_2d48aea2_hook);

    pfs_mgr_2190AEC_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_2190AEC_hook_ref, pfsmgr_info.modid, 0, 0xAEC, 1, pfs_mgr_2190AEC_hook);    

    iofilemgr_feee44a9_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &iofilemgr_feee44a9_hook_ref, "ScePfsMgr", SceIofilemgrForDriver_NID, 0xfeee44a9, iofilemgr_feee44a9_hook);

    iofilemgr_d220539d_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &iofilemgr_d220539d_hook_ref, "ScePfsMgr", SceIofilemgrForDriver_NID, 0xd220539d, iofilemgr_d220539d_hook);

    //vfs_2_func1 for PFS_GDSD that fails with 0x8001008F
    pfs_mgr_219105C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_219105C_hook_ref, pfsmgr_info.modid, 0, 0x219105C - 0x2190000, 1, pfs_mgr_219105C_hook);

    pfs_mgr_2193624_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_2193624_hook_ref, pfsmgr_info.modid, 0, 0x2193624 - 0x2190000, 1, pfs_mgr_2193624_hook);

    pfs_mgr_2198928_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_2198928_hook_ref, pfsmgr_info.modid, 0, 0x2198928 - 0x2190000, 1, pfs_mgr_2198928_hook);

    //read wrapper - disabled because slow, may enable
    //pfs_mgr_2199064_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_2199064_hook_ref, pfsmgr_info.modid, 0, 0x2199064 - 0x2190000, 1, pfs_mgr_2199064_hook);

    //checker - does not work
    //pfs_mgr_219DE44_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_219DE44_hook_ref, pfsmgr_info.modid, 0, 0x219DE44 - 0x2190000, 1, pfs_mgr_219DE44_hook);

    //decryptor - disabled because slow, may enable
    //pfs_mgr_219DE7C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_219DE7C_hook_ref, pfsmgr_info.modid, 0, 0x219DE7C - 0x2190000, 1, pfs_mgr_219DE7C_hook);

    //node data copying
    //pfs_mgr_2199144_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_2199144_hook_ref, pfsmgr_info.modid, 0, 0x2199144 - 0x2190000, 1, pfs_mgr_2199144_hook);

    //this patch disables checks occuring while mounting Pd<rnd>0
    #ifdef ENABLE_SD_PATCHES
    char pfs_219DE44_check_patch[4] = {0x01, 0x20, 0x00, 0xBF};

    pfs_219DE44_check_patch1_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x21A28FC - 0x2190000, pfs_219DE44_check_patch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP) 

    pfs_219DE44_check_patch2_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x21A295C - 0x2190000, pfs_219DE44_check_patch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP) 
    #endif

    //vfs_node_2_func4 - segfaults
    //pfs_mgr_21910F4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_21910F4_hook_ref, pfsmgr_info.modid, 0, 0x21910F4 - 0x2190000, 1, pfs_mgr_21910F4_hook);

    //icv file related function
    pfs_mgr_2193B44_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_2193B44_hook_ref, pfsmgr_info.modid, 0, 0x2193B44 - 0x2190000, 1, pfs_mgr_2193B44_hook);

    //this patch disables checks occuring while calling ksceIoGetstat for pfs filesystem (vfs_node2_func4)
    #ifdef ENABLE_SD_PATCHES
    pfs_219DE44_check_patch3_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x21A3FF4 - 0x2190000, pfs_219DE44_check_patch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP)

    pfs_219DE44_check_patch4_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x21A4016 - 0x2190000, pfs_219DE44_check_patch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP)
    #endif

    //vfs_node_2_func1
    pfs_mgr_219112C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_219112C_hook_ref, pfsmgr_info.modid, 0, 0x219112C - 0x2190000, 1, pfs_mgr_219112C_hook);

    pfs_mgr_2192190_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_2192190_hook_ref, pfsmgr_info.modid, 0, 0x2192190 - 0x2190000, 1, pfs_mgr_2192190_hook);

    //this patch disables checks occuring while calling ksceIoOpen for pfs filesystem (vfs_node2_func1)
    #ifdef ENABLE_SD_PATCHES
    pfs_219DE44_check_patch5_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x2196190 - 0x2190000, pfs_219DE44_check_patch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP) 
    
    //looks like this patch is not required
    //pfs_219DE44_check_patch6_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x21A0440 - 0x2190000, pfs_219DE44_check_patch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP) 
    
    //looks like this patch is not required
    //pfs_219DE44_check_patch7_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x21A0524 - 0x2190000, pfs_219DE44_check_patch, 4); //patch (BLX) to (MOVS R0, #1 ; NOP) 
    #endif

    char test_patch[8] = {0x4F, 0xF6, 0x02, 0x70, 0xC8, 0xF2, 0x14, 0x00};
    //pfs_80140f02_test_patch1_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x219ADA6 - 0x2190000, test_patch, 8);
    //pfs_80140f02_test_patch2_uid = taiInjectDataForKernel(KERNEL_PID, pfsmgr_info.modid, 0, 0x219AF64 - 0x2190000, test_patch, 8);

    //encryption subroutine ?
    //pfs_mgr_219BF20_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &pfs_mgr_219BF20_hook_ref, pfsmgr_info.modid, 0, 0x219BF20 - 0x2190000, 1, pfs_mgr_219BF20_hook);

    iofilemgr_0b54f9e0_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &iofilemgr_0b54f9e0_hook_ref, "ScePfsMgr", SceIofilemgrForDriver_NID, 0x0b54f9e0, iofilemgr_0b54f9e0_hook);
  }

  tai_module_info_t appmgr_info;
  appmgr_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceAppMgr", &appmgr_info) >= 0)
  {
    //hook that contains drm checks for game data mount
    appmgr_23D642C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &appmgr_23D642C_hook_ref, appmgr_info.modid, 0, 0x1642C, 1, appmgr_23D642C_hook);

    sceAppMgrGameDataMountForDriver_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceAppMgrGameDataMountForDriver_hook_ref, "SceAppMgr", SceAppMgrForDriver_NID, 0xCE356B2D, sceAppMgrGameDataMountForDriver_hook);

    //subroutine called from sceAppMgrGameDataMountForDriver that contains call to sceAppMgrGameDataVfsMountForDriver
    appmgr_23D9B50_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &appmgr_23D9B50_hook_ref, appmgr_info.modid, 0, 0x19B50, 1, appmgr_23D9B50_hook);

    //contains call to sceVfsMount
    sceAppMgrGameDataVfsMountForDriver_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sceAppMgrGameDataVfsMountForDriver_hook_ref, "SceAppMgr", SceIofilemgrForDriver_NID, 0xd070bc48, sceAppMgrGameDataVfsMountForDriver_hook);

    //sceFiosKernelOverlayRemoveForProcessForDriver_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sceFiosKernelOverlayRemoveForProcessForDriver_hook_ref, "SceAppMgr", SceFios2KernelForDriver_NID, 0x23247efb, sceFiosKernelOverlayRemoveForProcessForDriver_hook);

    proc_read_sealedkey_23D6EA0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &proc_read_sealedkey_23D6EA0_hook_ref, appmgr_info.modid, 0, 0x16EA0, 1, proc_read_sealedkey_23D6EA0_hook);

    sbl_acmgr_0b6e6cd7_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sbl_acmgr_0b6e6cd7_hook_ref, "SceAppMgr", SceSblACMgrForDriver_NID, 0x0b6e6cd7, sbl_acmgr_0b6e6cd7_hook);

    proc_generate_random_path_23D4FBC_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &proc_generate_random_path_23D4FBC_hook_ref, appmgr_info.modid, 0, 0x14FBC, 1, proc_generate_random_path_23D4FBC_hook);

    proc_get_clearsign_stat_23D9A4C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &proc_get_clearsign_stat_23D9A4C_hook_ref, appmgr_info.modid, 0, 0x19A4C, 1, proc_get_clearsign_stat_23D9A4C_hook);

    pfs_mgr_a772209c_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &pfs_mgr_a772209c_hook_ref, "SceAppMgr", ScePfsMgrForKernel_NID, 0xa772209c, pfs_mgr_a772209c_hook);

    app_mgr_23D5028_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &app_mgr_23D5028_hook_ref, appmgr_info.modid, 0, 0x23D5028 - 0x23C0000, 1, app_mgr_23D5028_hook);

    app_mgr_23EF934_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &app_mgr_23EF934_hook_ref, appmgr_info.modid, 0, 0x23EF934 - 0x23C0000, 1, app_mgr_23EF934_hook);

    //sceIoGetstatForDriver
    app_mgr_75c96d25_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &app_mgr_75c96d25_hook_ref, "SceAppMgr", SceIofilemgrForDriver_NID, 0x75c96d25, app_mgr_75c96d25_hook);

    //sceIoOpenForDriver
    app_mgr_75192972_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &app_mgr_75192972_hook_ref, "SceAppMgr", SceIofilemgrForDriver_NID, 0x75192972, app_mgr_75192972_hook);    

    //sceIoReadForDriver
    app_mgr_e17efc03_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &app_mgr_e17efc03_hook_ref, "SceAppMgr", SceIofilemgrForDriver_NID, 0xe17efc03, app_mgr_e17efc03_hook);
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
    /*
    //not required - duplicate
    sceVfsMount_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsMount_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xB62DE9A6, sceVfsMount_hook);

    sceVfsAddVfs_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsAddVfs_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x673D2FCD, sceVfsAddVfs_hook);

    //by some reason this hook fails with TAI_ERROR_HOOK_ERROR
    sceVfsUnmount_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsUnmount_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x9C7E7B76, sceVfsUnmount_hook);

    //these are not required - duplicates
    //sceVfsDeleteVfs_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsDeleteVfs_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x9CBFA725, sceVfsDeleteVfs_hook);
    //sceVfsGetNewNode_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceVfsGetNewNode_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xD60B5C63, sceVfsGetNewNode_hook);

    //does not work
    //sceIoOpenForDriver_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceIoOpenForDriver_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x75192972, sceIoOpenForDriver_hook);

    //user hook
    #ifdef ENABLE_IO_FILE_OPEN_LOG
    sceIoOpenForDriver_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sceIoOpenForDriver_hook_ref, "SceIofilemgr", SceIofilemgr_NID, 0xCC67B6FD, sceIoOpenForDriver_hook);
    #endif

    iofilemgr_BF3848_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF3848_hook_ref, iofilemgr_info.modid, 0, 0x13848, 1, iofilemgr_BF3848_hook);
    */

    //important
    iofilemgr_BF3474_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF3474_hook_ref, iofilemgr_info.modid, 0, 0x13474, 1, iofilemgr_BF3474_hook);

    //important
    iofilemgr_sceVfsMount_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_sceVfsMount_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xb62de9a6, iofilemgr_sceVfsMount_hook);

    //important
    iofilemgr_sceVfsGetNewNode_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_sceVfsGetNewNode_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xd60b5c63, iofilemgr_sceVfsGetNewNode_hook);

    /*
    iofilemgr_BF1AF0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF1AF0_hook_ref, iofilemgr_info.modid, 0, 0x11AF0, 1, iofilemgr_BF1AF0_hook);
    */

    //important
    iofilemgr_BE5B30_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE5B30_hook_ref, iofilemgr_info.modid, 0, 0x5B30, 1, iofilemgr_BE5B30_hook);

    //important
    iofilemgr_BF18CC_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF18CC_hook_ref, iofilemgr_info.modid, 0, 0x00BF18CC - 0x00BE0000, 1, iofilemgr_BF18CC_hook);

    //important
    iofilemgr_BEDEB0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEDEB0_hook_ref, iofilemgr_info.modid, 0, 0xDEB0, 1, iofilemgr_BEDEB0_hook);

    
    //iofilemgr_BE62E8_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE62E8_hook_ref, iofilemgr_info.modid, 0, 0x62E8, 1, iofilemgr_BE62E8_hook);

    /*
    iofilemgr_BE5F10_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE5F10_hook_ref, iofilemgr_info.modid, 0, 0x5F10, 1, iofilemgr_BE5F10_hook);
    */
    
    iofilemgr_BE6788_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE6788_hook_ref, iofilemgr_info.modid, 0, 0x6788, 1, iofilemgr_BE6788_hook);
    
    /*
    iofilemgr_BEE3C8_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEE3C8_hook_ref, iofilemgr_info.modid, 0, 0xE3C8, 1, iofilemgr_BEE3C8_hook);

    iofilemgr_BEBAC0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEBAC0_hook_ref, iofilemgr_info.modid, 0, 0xBAC0, 1, iofilemgr_BEBAC0_hook);
    */

    //important
    iofilemgr_BE59BC_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE59BC_hook_ref, iofilemgr_info.modid, 0, 0x59BC, 1, iofilemgr_BE59BC_hook);

    /*
    //--------
    iofilemgr_BEE364_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEE364_hook_ref, iofilemgr_info.modid, 0, 0xBEE364 - 0x00BE0000, 1, iofilemgr_BEE364_hook);

    iofilemgr_BEE2D4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEE2D4_hook_ref, iofilemgr_info.modid, 0, 0xBEE2D4 - 0x00BE0000, 1, iofilemgr_BEE2D4_hook);

    iofilemgr_BEE2C4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEE2C4_hook_ref, iofilemgr_info.modid, 0, 0xBEE2C4 - 0x00BE0000, 1, iofilemgr_BEE2C4_hook);

    iofilemgr_BEDF5C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEDF5C_hook_ref, iofilemgr_info.modid, 0, 0xBEDF5C - 0x00BE0000, 1, iofilemgr_BEDF5C_hook);

    iofilemgr_BECE80_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BECE80_hook_ref, iofilemgr_info.modid, 0, 0xBECE80 - 0x00BE0000, 1, iofilemgr_BECE80_hook);

    iofilemgr_BEC808_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEC808_hook_ref, iofilemgr_info.modid, 0, 0xBEC808 - 0x00BE0000, 1, iofilemgr_BEC808_hook);

    iofilemgr_BEC530_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEC530_hook_ref, iofilemgr_info.modid, 0, 0xBEC530 - 0x00BE0000, 1, iofilemgr_BEC530_hook);

    iofilemgr_BEC010_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEC010_hook_ref, iofilemgr_info.modid, 0, 0xBEC010 - 0x00BE0000, 1, iofilemgr_BEC010_hook);

    iofilemgr_BE5A38_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE5A38_hook_ref, iofilemgr_info.modid, 0, 0xBE5A38 - 0x00BE0000, 1, iofilemgr_BE5A38_hook);

    iofilemgr_BE5814_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE5814_hook_ref, iofilemgr_info.modid, 0, 0xBE5814 - 0x00BE0000, 1, iofilemgr_BE5814_hook);
    */

    //important
    iofilemgr_BEBB84_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEBB84_hook_ref, iofilemgr_info.modid, 0, 0xBEBB84 - 0x00BE0000, 1, iofilemgr_BEBB84_hook);
    
    //important
    iofilemgr_BEC7C0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEC7C0_hook_ref, iofilemgr_info.modid, 0, 0xBEC7C0 - 0x00BE0000, 1, iofilemgr_BEC7C0_hook);
    
    /*
    iofilemgr_BEC578_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEC578_hook_ref, iofilemgr_info.modid, 0, 0xBEC578 - 0x00BE0000, 1, iofilemgr_BEC578_hook);
    */

    //segfault - with others
    /*
    iofilemgr_BEC620_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEC620_hook_ref, iofilemgr_info.modid, 0, 0xBEC620 - 0x00BE0000, 1, iofilemgr_BEC620_hook);
    */

    //segfault - with others
    /*
    iofilemgr_BECE0C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BECE0C_hook_ref, iofilemgr_info.modid, 0, 0xBECE0C - 0x00BE0000, 1, iofilemgr_BECE0C_hook);
    */

    // error - no hook 9001006
    //important
    iofilemgr_BE61C4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE61C4_hook_ref, iofilemgr_info.modid, 0, 0xBE61C4 - 0x00BE0000, 1, iofilemgr_BE61C4_hook);

    // error - no hook 9001005
    //iofilemgr_BEC56C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEC56C_hook_ref, iofilemgr_info.modid, 0, 0xBEC56C - 0x00BE0000, 1, iofilemgr_BEC56C_hook);

    //------

    // error on call
    //iofilemgr_exp_aa45010b_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_exp_aa45010b_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xaa45010b, iofilemgr_exp_aa45010b_hook);

    // error on call
    //iofilemgr_exp_21d57633_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_exp_21d57633_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x21d57633, iofilemgr_exp_21d57633_hook);

    //error on henkaku install
    //iofilemgr_exp_dc2d8bce_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_exp_dc2d8bce_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xdc2d8bce, iofilemgr_exp_dc2d8bce_hook);

    //error on henkaku install
    //iofilemgr_exp_6b3ca9f7_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_exp_6b3ca9f7_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x6b3ca9f7, iofilemgr_exp_6b3ca9f7_hook);

    //-----------

    iofilemgr_BE8444_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE8444_hook_ref, iofilemgr_info.modid, 0, 0xBE8444 - 0x00BE0000, 1, iofilemgr_BE8444_hook);

    //iofilemgr_50a63acf_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_50a63acf_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x50a63acf, iofilemgr_50a63acf_hook);

    //does not work - segfaults
    //iofilemgr_BE584C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE584C_hook_ref, iofilemgr_info.modid, 0, 0xBE584C - 0x00BE0000, 1, iofilemgr_BE584C_hook);

    iofilemgr_BE5CC4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE5CC4_hook_ref, iofilemgr_info.modid, 0, 0xBE5CC4 - 0x00BE0000, 1, iofilemgr_BE5CC4_hook);

    //vfs_func13
    //iofilemgr_f7dac0f5_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_f7dac0f5_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xf7dac0f5, iofilemgr_f7dac0f5_hook);

    //vfs_node_func4
    iofilemgr_a5a6a55c_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_a5a6a55c_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0xa5a6a55c, iofilemgr_a5a6a55c_hook);

    iofilemgr_BEA704_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BEA704_hook_ref, iofilemgr_info.modid, 0, 0xBEA704 - 0x00BE0000, 1, iofilemgr_BEA704_hook);

    //vfs_node_func2
    //iofilemgr_9e347c7d_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_9e347c7d_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x9e347c7d, iofilemgr_9e347c7d_hook);

    //vfs_node_func1 segfaults
    //iofilemgr_76b79bec_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &iofilemgr_76b79bec_hook_ref, "SceIofilemgr", SceIofilemgrForDriver_NID, 0x76b79bec, iofilemgr_76b79bec_hook);

    //sceIoReadForDriver main thread
    iofilemgr_BE7AEC_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE7AEC_hook_ref, iofilemgr_info.modid, 0, 0xBE7AEC - 0x00BE0000, 1, iofilemgr_BE7AEC_hook);

    //this is read alternative that should not be called
    iofilemgr_BF57F8_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF57F8_hook_ref, iofilemgr_info.modid, 0, 0xBF57F8 - 0x00BE0000, 1, iofilemgr_BF57F8_hook);

    //read alternative subroutne
    iofilemgr_BF66E4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF66E4_hook_ref, iofilemgr_info.modid, 0, 0xBF66E4 - 0x00BE0000, 1, iofilemgr_BF66E4_hook);

    //read alternative subroutne
    iofilemgr_BF6B6C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF6B6C_hook_ref, iofilemgr_info.modid, 0, 0xBF6B6C - 0x00BE0000, 1, iofilemgr_BF6B6C_hook);

    //read alternative subroutne
    //iofilemgr_BF69B4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF69B4_hook_ref, iofilemgr_info.modid, 0, 0xBF69B4 - 0x00BE0000, 1, iofilemgr_BF69B4_hook);

    iofilemgr_BF651C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF651C_hook_ref, iofilemgr_info.modid, 0, 0xBF651C - 0x00BE0000, 1, iofilemgr_BF651C_hook);

    //i/o dispatcher functions
    iofilemgr_BF89EC_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF89EC_hook_ref, iofilemgr_info.modid, 0, 0xBF89EC - 0x00BE0000, 1, iofilemgr_BF89EC_hook);

    iofilemgr_BF8BD4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF8BD4_hook_ref, iofilemgr_info.modid, 0, 0xBF8BD4 - 0x00BE0000, 1, iofilemgr_BF8BD4_hook); 

    iofilemgr_BF7C00_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF7C00_hook_ref, iofilemgr_info.modid, 0, 0xBF7C00 - 0x00BE0000, 1, iofilemgr_BF7C00_hook);

    iofilemgr_BF8BB0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF8BB0_hook_ref, iofilemgr_info.modid, 0, 0xBF8BB0 - 0x00BE0000, 1, iofilemgr_BF8BB0_hook);
    iofilemgr_BF7288_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF7288_hook_ref, iofilemgr_info.modid, 0, 0xBF7288 - 0x00BE0000, 1, iofilemgr_BF7288_hook);
    iofilemgr_BFB5F0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BFB5F0_hook_ref, iofilemgr_info.modid, 0, 0xBFB5F0 - 0x00BE0000, 1, iofilemgr_BFB5F0_hook);
    iofilemgr_BF88C4_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF88C4_hook_ref, iofilemgr_info.modid, 0, 0xBF88C4 - 0x00BE0000, 1, iofilemgr_BF88C4_hook);
    iofilemgr_BFB794_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BFB794_hook_ref, iofilemgr_info.modid, 0, 0xBFB794 - 0x00BE0000, 1, iofilemgr_BFB794_hook);
    
    iofilemgr_BF7268_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF7268_hook_ref, iofilemgr_info.modid, 0, 0xBF7268 - 0x00BE0000, 1, iofilemgr_BF7268_hook);
    iofilemgr_BF83A0_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF83A0_hook_ref, iofilemgr_info.modid, 0, 0xBF83A0 - 0x00BE0000, 1, iofilemgr_BF83A0_hook);
    iofilemgr_BFB628_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BFB628_hook_ref, iofilemgr_info.modid, 0, 0xBFB628 - 0x00BE0000, 1, iofilemgr_BFB628_hook);
    
    iofilemgr_BF7228_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF7228_hook_ref, iofilemgr_info.modid, 0, 0xBF7228 - 0x00BE0000, 1, iofilemgr_BF7228_hook);
    iofilemgr_BF83F8_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BF83F8_hook_ref, iofilemgr_info.modid, 0, 0xBF83F8 - 0x00BE0000, 1, iofilemgr_BF83F8_hook);
    iofilemgr_BFB748_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BFB748_hook_ref, iofilemgr_info.modid, 0, 0xBFB748 - 0x00BE0000, 1, iofilemgr_BFB748_hook);

    iofilemgr_BE7F4C_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BE7F4C_hook_ref, iofilemgr_info.modid, 0, 0xBE7F4C - 0x00BE0000, 1, iofilemgr_BE7F4C_hook);

    iofilemgr_BFB2A8_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &iofilemgr_BFB2A8_hook_ref, iofilemgr_info.modid, 0, 0xBFB2A8 - 0x00BE0000, 1, iofilemgr_BFB2A8_hook);

    //pfs pread impl
    pfs_facade_4238d2d2_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &pfs_facade_4238d2d2_hook_ref, "SceIofilemgr", ScePfsFacadeForKernel_NID, 0x4238d2d2, pfs_facade_4238d2d2_hook);
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

  if(sceAppMgrGameDataMountForDriver_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceAppMgrGameDataMountForDriver_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceAppMgrGameDataMountForDriver_hook: %x\n", sceAppMgrGameDataMountForDriver_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(appmgr_23D9B50_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set appmgr_23D9B50_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set appmgr_23D9B50_hook: %x\n", appmgr_23D9B50_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceAppMgrGameDataVfsMountForDriver_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceAppMgrGameDataVfsMountForDriver_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceAppMgrGameDataVfsMountForDriver_hook: %x\n", sceAppMgrGameDataVfsMountForDriver_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sceFiosKernelOverlayRemoveForProcessForDriver_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sceFiosKernelOverlayRemoveForProcessForDriver_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sceFiosKernelOverlayRemoveForProcessForDriver_hook: %x\n", sceFiosKernelOverlayRemoveForProcessForDriver_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(proc_read_sealedkey_23D6EA0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set proc_read_sealedkey_23D6EA0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set proc_read_sealedkey_23D6EA0_hook: %x\n", proc_read_sealedkey_23D6EA0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(sbl_acmgr_0b6e6cd7_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set sbl_acmgr_0b6e6cd7_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set sbl_acmgr_0b6e6cd7_hook: %x\n", sbl_acmgr_0b6e6cd7_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(proc_generate_random_path_23D4FBC_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set proc_generate_random_path_23D4FBC_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set proc_generate_random_path_23D4FBC_hook: %x\n", proc_generate_random_path_23D4FBC_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(proc_get_clearsign_stat_23D9A4C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set proc_get_clearsign_stat_23D9A4C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set proc_get_clearsign_stat_23D9A4C_hook: %x\n", proc_get_clearsign_stat_23D9A4C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_a772209c_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_a772209c_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_a772209c_hook: %x\n", pfs_mgr_a772209c_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2d48aea2_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2d48aea2_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2d48aea2_hook: %x\n", pfs_mgr_2d48aea2_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2190AEC_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2190AEC_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2190AEC_hook: %x\n", pfs_mgr_2190AEC_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  
  if(iofilemgr_feee44a9_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_feee44a9_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_feee44a9_hook: %x\n", iofilemgr_feee44a9_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_d220539d_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_d220539d_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_d220539d_hook: %x\n", iofilemgr_d220539d_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF3848_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF3848_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF3848_hook: %x\n", iofilemgr_BF3848_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF3474_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF3474_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF3474_hook: %x\n", iofilemgr_BF3474_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_sceVfsMount_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_sceVfsMount_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_sceVfsMount_hook: %x\n", iofilemgr_sceVfsMount_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_sceVfsGetNewNode_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_sceVfsGetNewNode_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_sceVfsGetNewNode_hook: %x\n", iofilemgr_sceVfsGetNewNode_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF1AF0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF1AF0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF1AF0_hook: %x\n", iofilemgr_BF1AF0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE5B30_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE5B30_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE5B30_hook: %x\n", iofilemgr_BE5B30_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }  

  if(iofilemgr_BF18CC_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF18CC_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF18CC_hook: %x\n", iofilemgr_BF18CC_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEDEB0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEDEB0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEDEB0_hook: %x\n", iofilemgr_BEDEB0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE62E8_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE62E8_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE62E8_hook: %x\n", iofilemgr_BE62E8_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE5F10_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE5F10_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE5F10_hook: %x\n", iofilemgr_BE5F10_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE6788_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE6788_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE6788_hook: %x\n", iofilemgr_BE6788_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEE3C8_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEE3C8_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEE3C8_hook: %x\n", iofilemgr_BEE3C8_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEBAC0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEBAC0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEBAC0_hook: %x\n", iofilemgr_BEBAC0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE61C4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE61C4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE61C4_hook: %x\n", iofilemgr_BE61C4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE59BC_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE59BC_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE59BC_hook: %x\n", iofilemgr_BE59BC_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  //-----------------

  if(iofilemgr_BEE364_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEE364_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEE364_hook: %x\n", iofilemgr_BEE364_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEE2D4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEE2D4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEE2D4_hook: %x\n", iofilemgr_BEE2D4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEE2C4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEE2C4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEE2C4_hook: %x\n", iofilemgr_BEE2C4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEDF5C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEDF5C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEDF5C_hook: %x\n", iofilemgr_BEDF5C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BECE80_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BECE80_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BECE80_hook: %x\n", iofilemgr_BECE80_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEC808_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEC808_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEC808_hook: %x\n", iofilemgr_BEC808_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEC620_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEC620_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEC620_hook: %x\n", iofilemgr_BEC620_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEC578_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEC578_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEC578_hook: %x\n", iofilemgr_BEC578_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEC56C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEC56C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEC56C_hook: %x\n", iofilemgr_BEC56C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEC530_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEC530_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEC530_hook: %x\n", iofilemgr_BEC530_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEC010_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEC010_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEC010_hook: %x\n", iofilemgr_BEC010_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE5A38_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE5A38_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE5A38_hook: %x\n", iofilemgr_BE5A38_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE5814_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE5814_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE5814_hook: %x\n", iofilemgr_BE5814_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEBB84_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEBB84_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEBB84_hook: %x\n", iofilemgr_BEBB84_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEC7C0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEC7C0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEC7C0_hook: %x\n", iofilemgr_BEC7C0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BECE0C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BECE0C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BECE0C_hook: %x\n", iofilemgr_BECE0C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  //---------------

  if(iofilemgr_exp_aa45010b_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_exp_aa45010b_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_exp_aa45010b_hook: %x\n", iofilemgr_exp_aa45010b_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_exp_21d57633_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_exp_21d57633_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_exp_21d57633_hook: %x\n", iofilemgr_exp_21d57633_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_exp_dc2d8bce_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_exp_dc2d8bce_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_exp_dc2d8bce_hook: %x\n", iofilemgr_exp_dc2d8bce_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_exp_6b3ca9f7_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_exp_6b3ca9f7_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_exp_6b3ca9f7_hook: %x\n", iofilemgr_exp_6b3ca9f7_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_219105C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_219105C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_219105C_hook: %x\n", pfs_mgr_219105C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2193624_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2193624_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2193624_hook: %x\n", pfs_mgr_2193624_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2198928_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2198928_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2198928_hook: %x\n", pfs_mgr_2198928_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2199064_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2199064_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2199064_hook: %x\n", pfs_mgr_2199064_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_219DE44_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_219DE44_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_219DE44_hook: %x\n", pfs_mgr_219DE44_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_219DE7C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_219DE7C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_219DE7C_hook: %x\n", pfs_mgr_219DE7C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2199144_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2199144_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2199144_hook: %x\n", pfs_mgr_2199144_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_219DE44_check_patch1_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_219DE44_check_patch1\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_219DE44_check_patch1: %x\n", pfs_219DE44_check_patch1_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_219DE44_check_patch2_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_219DE44_check_patch2\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_219DE44_check_patch2: %x\n", pfs_219DE44_check_patch2_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(app_mgr_23D5028_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set app_mgr_23D5028_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set app_mgr_23D5028_hook: %x\n", app_mgr_23D5028_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(app_mgr_23EF934_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set app_mgr_23EF934_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set app_mgr_23EF934_hook: %x\n", app_mgr_23EF934_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(app_mgr_75c96d25_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set app_mgr_75c96d25_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set app_mgr_75c96d25_hook: %x\n", app_mgr_75c96d25_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE8444_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE8444_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE8444_hook: %x\n", iofilemgr_BE8444_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_50a63acf_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_50a63acf_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_50a63acf_hook: %x\n", iofilemgr_50a63acf_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE584C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE584C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE584C_hook: %x\n", iofilemgr_BE584C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE5CC4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE5CC4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE5CC4_hook: %x\n", iofilemgr_BE5CC4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_f7dac0f5_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_f7dac0f5_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_f7dac0f5_hook: %x\n", iofilemgr_f7dac0f5_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_a5a6a55c_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_a5a6a55c_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_a5a6a55c_hook: %x\n", iofilemgr_a5a6a55c_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_21910F4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_21910F4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_21910F4_hook: %x\n", pfs_mgr_21910F4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2193B44_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2193B44_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2193B44_hook: %x\n", pfs_mgr_2193B44_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_219DE44_check_patch3_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_219DE44_check_patch3\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_219DE44_check_patch3: %x\n", pfs_219DE44_check_patch3_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_219DE44_check_patch4_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_219DE44_check_patch4\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_219DE44_check_patch4: %x\n", pfs_219DE44_check_patch4_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(app_mgr_75192972_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set app_mgr_75192972_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set app_mgr_75192972_hook: %x\n", app_mgr_75192972_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BEA704_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BEA704_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BEA704_hook: %x\n", iofilemgr_BEA704_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_9e347c7d_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_9e347c7d_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_9e347c7d_hook: %x\n", iofilemgr_9e347c7d_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_76b79bec_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_76b79bec_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_76b79bec_hook: %x\n", iofilemgr_76b79bec_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_219112C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_219112C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_219112C_hook: %x\n", pfs_mgr_219112C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_2192190_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_2192190_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_2192190_hook: %x\n", pfs_mgr_2192190_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_219DE44_check_patch5_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_219DE44_check_patch5\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_219DE44_check_patch5: %x\n", pfs_219DE44_check_patch5_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_219DE44_check_patch6_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_219DE44_check_patch6\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_219DE44_check_patch6: %x\n", pfs_219DE44_check_patch6_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_219DE44_check_patch7_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_219DE44_check_patch7\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_219DE44_check_patch7: %x\n", pfs_219DE44_check_patch7_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(app_mgr_e17efc03_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set app_mgr_e17efc03_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set app_mgr_e17efc03_hook: %x\n", app_mgr_e17efc03_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_80140f02_test_patch1_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_80140f02_test_patch1\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_80140f02_test_patch1: %x\n", pfs_80140f02_test_patch1_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_80140f02_test_patch2_uid >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_80140f02_test_patch2\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_80140f02_test_patch2: %x\n", pfs_80140f02_test_patch2_uid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_mgr_219BF20_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_mgr_219BF20_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_mgr_219BF20_hook: %x\n", pfs_mgr_219BF20_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE7AEC_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE7AEC_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE7AEC_hook: %x\n", iofilemgr_BE7AEC_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF57F8_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF57F8_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF57F8_hook: %x\n", iofilemgr_BF57F8_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF66E4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF66E4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF66E4_hook: %x\n", iofilemgr_BF66E4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF6B6C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF6B6C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF6B6C_hook: %x\n", iofilemgr_BF6B6C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF69B4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF69B4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF69B4_hook: %x\n", iofilemgr_BF69B4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF651C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF651C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF651C_hook: %x\n", iofilemgr_BF651C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF89EC_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF89EC_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF89EC_hook: %x\n", iofilemgr_BF89EC_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF8BD4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF8BD4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF8BD4_hook: %x\n", iofilemgr_BF8BD4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF7C00_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF7C00_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF7C00_hook: %x\n", iofilemgr_BF7C00_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF8BB0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF8BB0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF8BB0_hook: %x\n", iofilemgr_BF8BB0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF7288_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF7288_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF7288_hook: %x\n", iofilemgr_BF7288_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BFB5F0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BFB5F0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BFB5F0_hook: %x\n", iofilemgr_BFB5F0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF88C4_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF88C4_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF88C4_hook: %x\n", iofilemgr_BF88C4_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BFB794_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BFB794_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BFB794_hook: %x\n", iofilemgr_BFB794_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF7268_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF7268_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF7268_hook: %x\n", iofilemgr_BF7268_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF83A0_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF83A0_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF83A0_hook: %x\n", iofilemgr_BF83A0_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BFB628_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BFB628_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BFB628_hook: %x\n", iofilemgr_BFB628_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF7228_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF7228_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF7228_hook: %x\n", iofilemgr_BF7228_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BF83F8_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BF83F8_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BF83F8_hook: %x\n", iofilemgr_BF83F8_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BFB748_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BFB748_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BFB748_hook: %x\n", iofilemgr_BFB748_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BE7F4C_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BE7F4C_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BE7F4C_hook: %x\n", iofilemgr_BE7F4C_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(iofilemgr_BFB2A8_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set iofilemgr_BFB2A8_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set iofilemgr_BFB2A8_hook: %x\n", iofilemgr_BFB2A8_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  if(pfs_facade_4238d2d2_hook_id >= 0)
  {
    FILE_GLOBAL_WRITE_LEN("set pfs_facade_4238d2d2_hook\n");
  }
  else
  {
    snprintf(sprintfBuffer, 256, "failed to set pfs_facade_4238d2d2_hook: %x\n", pfs_facade_4238d2d2_hook_id);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }

  PRINT_HOOK(iofilemgr_0b54f9e0_hook);

  close_global_log();
  
  return 0;
}

int deinitialize_all_hooks()
{
  RELEASE_HOOK(gc_hook);
  RELEASE_HOOK(init_mmc_hook);
  RELEASE_HOOK(init_sd_hook);

  if (patch_uids[0] >= 0)
    taiInjectReleaseForKernel(patch_uids[0]);
  
  if (patch_uids[1] >= 0)
    taiInjectReleaseForKernel(patch_uids[1]);
  
  if (patch_uids[2] >= 0)
    taiInjectReleaseForKernel(patch_uids[2]);
  
  RELEASE_HOOK(cmd55_41_hook);
  
  if(gen_init_hook_uids[0] >= 0)
    taiHookReleaseForKernel(gen_init_hook_uids[0], gen_init_hook_refs[0]);
      
  if(gen_init_hook_uids[1] >= 0)
    taiHookReleaseForKernel(gen_init_hook_uids[1], gen_init_hook_refs[1]);
    
  if(gen_init_hook_uids[2] >= 0)
    taiHookReleaseForKernel(gen_init_hook_uids[2], gen_init_hook_refs[2]);
  
  RELEASE_HOOK(load_mbr_hook);
  RELEASE_HOOK(mnt_pnt_chk_hook);
  RELEASE_HOOK(mbr_table_init_hook);
  RELEASE_HOOK(sysroot_zero_hook);

  for(int f = 0; f < 13; f++)
  {
    if(sdstor_dev_fs_ids[f] >= 0)
      taiHookReleaseForKernel(sdstor_dev_fs_ids[f], sdstor_dev_fs_refs[f]);
  }
  
  RELEASE_HOOK(sceVfsMount_hook);
  RELEASE_HOOK(sceVfsAddVfs_hook);
  RELEASE_HOOK(sceVfsUnmount_hook);
  RELEASE_HOOK(sceVfsDeleteVfs_hook);
  RELEASE_HOOK(sceVfsGetNewNode_hook);
  
  RELEASE_INJECT(gen_init_2_patch);
  RELEASE_INJECT(gen_init_1_patch);

  RELEASE_HOOK(gen_read_hook);
  RELEASE_HOOK(sd_read_hook);

  RELEASE_INJECT(hs_dis_patch1);
  RELEASE_INJECT(hs_dis_patch2);
  RELEASE_INJECT(bus_size_patch);
  RELEASE_INJECT(mbr_init_zero_patch1);
  RELEASE_INJECT(mbr_init_zero_patch2);
  
  RELEASE_HOOK(init_partition_table_hook);
  RELEASE_HOOK(create_device_handle_hook);
  RELEASE_HOOK(send_command_hook);
  RELEASE_HOOK(sceErrorHistoryPostError_hook);
  RELEASE_HOOK(sceErrorHistoryUpdateSequenceInfo_hook);
  RELEASE_HOOK(sceErrorGetExternalString_kernel_hook);
  RELEASE_HOOK(sceErrorHistoryGetError_hook);
  RELEASE_HOOK(ksceKernelCreateThread_hook);
  RELEASE_HOOK(sceKernelCreateThreadForUser_hook);
  RELEASE_HOOK(sceIoOpenForDriver_hook);
  RELEASE_HOOK(ksceKernelWaitSema_hook);
  RELEASE_HOOK(ksceKernelSignalSema_hook);
  RELEASE_HOOK(vshSblAuthMgrVerifySpsfo_hook);
  RELEASE_HOOK(debug_printf_callback_invoke);
  RELEASE_HOOK(gc_22fd5d23_hook);
  RELEASE_HOOK(appmgr_23D642C_hook);
  RELEASE_HOOK(sceAppMgrGameDataMountForDriver_hook);
  RELEASE_HOOK(appmgr_23D9B50_hook);
  RELEASE_HOOK(sceAppMgrGameDataVfsMountForDriver_hook);
  RELEASE_HOOK(sceFiosKernelOverlayRemoveForProcessForDriver_hook);
  RELEASE_HOOK(proc_read_sealedkey_23D6EA0_hook);
  RELEASE_HOOK(sbl_acmgr_0b6e6cd7_hook);
  RELEASE_HOOK(proc_generate_random_path_23D4FBC_hook);
  RELEASE_HOOK(proc_get_clearsign_stat_23D9A4C_hook);
  RELEASE_HOOK(pfs_mgr_a772209c_hook);
  RELEASE_HOOK(pfs_mgr_2d48aea2_hook);
  RELEASE_HOOK(pfs_mgr_2190AEC_hook);
  RELEASE_HOOK(iofilemgr_feee44a9_hook);
  RELEASE_HOOK(iofilemgr_d220539d_hook);
  RELEASE_HOOK(iofilemgr_BF3848_hook);
  RELEASE_HOOK(iofilemgr_BF3474_hook);
  RELEASE_HOOK(iofilemgr_sceVfsMount_hook);
  RELEASE_HOOK(iofilemgr_sceVfsGetNewNode_hook);
  RELEASE_HOOK(iofilemgr_BF1AF0_hook);
  RELEASE_HOOK(iofilemgr_BE5B30_hook);
  RELEASE_HOOK(iofilemgr_BF18CC_hook);
  RELEASE_HOOK(iofilemgr_BEDEB0_hook);
  RELEASE_HOOK(iofilemgr_BE62E8_hook);
  RELEASE_HOOK(iofilemgr_BE5F10_hook);
  RELEASE_HOOK(iofilemgr_BE6788_hook);
  RELEASE_HOOK(iofilemgr_BEE3C8_hook);
  RELEASE_HOOK(iofilemgr_BEBAC0_hook);
  RELEASE_HOOK(iofilemgr_BE61C4_hook);
  RELEASE_HOOK(iofilemgr_BE59BC_hook);

  //---

  RELEASE_HOOK(iofilemgr_BEE364_hook);
  RELEASE_HOOK(iofilemgr_BEE2D4_hook);
  RELEASE_HOOK(iofilemgr_BEE2C4_hook);
  RELEASE_HOOK(iofilemgr_BEDF5C_hook);
  RELEASE_HOOK(iofilemgr_BECE80_hook);
  RELEASE_HOOK(iofilemgr_BEC808_hook);
  RELEASE_HOOK(iofilemgr_BEC620_hook);
  RELEASE_HOOK(iofilemgr_BEC578_hook);
  RELEASE_HOOK(iofilemgr_BEC56C_hook);
  RELEASE_HOOK(iofilemgr_BEC530_hook);
  RELEASE_HOOK(iofilemgr_BEC010_hook);
  RELEASE_HOOK(iofilemgr_BE5A38_hook);
  RELEASE_HOOK(iofilemgr_BE5814_hook);
  RELEASE_HOOK(iofilemgr_BEBB84_hook);
  RELEASE_HOOK(iofilemgr_BEC7C0_hook);
  RELEASE_HOOK(iofilemgr_BECE0C_hook);

  //-----

  RELEASE_HOOK(iofilemgr_exp_aa45010b_hook);
  RELEASE_HOOK(iofilemgr_exp_21d57633_hook);
  RELEASE_HOOK(iofilemgr_exp_dc2d8bce_hook);
  RELEASE_HOOK(iofilemgr_exp_6b3ca9f7_hook);

  //--------

  RELEASE_HOOK(pfs_mgr_219105C_hook);
  RELEASE_HOOK(pfs_mgr_2193624_hook);
  RELEASE_HOOK(pfs_mgr_2198928_hook);
  RELEASE_HOOK(pfs_mgr_2199064_hook);
  RELEASE_HOOK(pfs_mgr_219DE44_hook);
  RELEASE_HOOK(pfs_mgr_219DE7C_hook);
  RELEASE_HOOK(pfs_mgr_2199144_hook);

  //-------

  RELEASE_INJECT(pfs_219DE44_check_patch1);
  RELEASE_INJECT(pfs_219DE44_check_patch2);

  //------

  RELEASE_HOOK(app_mgr_23D5028_hook);
  RELEASE_HOOK(app_mgr_23EF934_hook);
  RELEASE_HOOK(app_mgr_75c96d25_hook);
  RELEASE_HOOK(iofilemgr_BE8444_hook);
  RELEASE_HOOK(iofilemgr_50a63acf_hook);
  RELEASE_HOOK(iofilemgr_BE584C_hook);
  RELEASE_HOOK(iofilemgr_BE5CC4_hook);
  RELEASE_HOOK(iofilemgr_f7dac0f5_hook);
  RELEASE_HOOK(iofilemgr_a5a6a55c_hook);
  RELEASE_HOOK(pfs_mgr_21910F4_hook);
  RELEASE_HOOK(pfs_mgr_2193B44_hook);
  
  //------

  RELEASE_INJECT(pfs_219DE44_check_patch3);
  RELEASE_INJECT(pfs_219DE44_check_patch4);

  //------

  RELEASE_HOOK(app_mgr_75192972_hook);
  RELEASE_HOOK(iofilemgr_BEA704_hook);
  RELEASE_HOOK(iofilemgr_9e347c7d_hook);
  RELEASE_HOOK(iofilemgr_76b79bec_hook);
  RELEASE_HOOK(pfs_mgr_219112C_hook);
  RELEASE_HOOK(pfs_mgr_2192190_hook);

  //------

  RELEASE_INJECT(pfs_219DE44_check_patch5);
  RELEASE_INJECT(pfs_219DE44_check_patch6);
  RELEASE_INJECT(pfs_219DE44_check_patch7);

  //------

  RELEASE_HOOK(app_mgr_e17efc03_hook);

  //-------

  RELEASE_INJECT(pfs_80140f02_test_patch1);
  RELEASE_INJECT(pfs_80140f02_test_patch2);

  RELEASE_HOOK(pfs_mgr_219BF20_hook);
  RELEASE_HOOK(iofilemgr_BE7AEC_hook);
  RELEASE_HOOK(iofilemgr_BF57F8_hook);
  RELEASE_HOOK(iofilemgr_BF66E4_hook);
  RELEASE_HOOK(iofilemgr_BF6B6C_hook);
  RELEASE_HOOK(iofilemgr_BF69B4_hook);
  RELEASE_HOOK(iofilemgr_BF651C_hook);
  RELEASE_HOOK(iofilemgr_BF89EC_hook);
  RELEASE_HOOK(iofilemgr_BF8BD4_hook);
  RELEASE_HOOK(iofilemgr_BF7C00_hook);

  //

  RELEASE_HOOK(iofilemgr_BF8BB0_hook);
  RELEASE_HOOK(iofilemgr_BF7288_hook);
  RELEASE_HOOK(iofilemgr_BFB5F0_hook);
  RELEASE_HOOK(iofilemgr_BF88C4_hook);
  RELEASE_HOOK(iofilemgr_BFB794_hook);
  RELEASE_HOOK(iofilemgr_BF7268_hook);
  RELEASE_HOOK(iofilemgr_BF83A0_hook);
  RELEASE_HOOK(iofilemgr_BFB628_hook);
  RELEASE_HOOK(iofilemgr_BF7228_hook);
  RELEASE_HOOK(iofilemgr_BF83F8_hook);
  RELEASE_HOOK(iofilemgr_BFB748_hook);
  RELEASE_HOOK(iofilemgr_BE7F4C_hook);
  RELEASE_HOOK(iofilemgr_BFB2A8_hook);
  RELEASE_HOOK(pfs_facade_4238d2d2_hook);
  RELEASE_HOOK(iofilemgr_0b54f9e0_hook);

  return 0;
}
