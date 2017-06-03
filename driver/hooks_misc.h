#pragma once

#include <psp2kern/types.h>

#include <taihen.h>

#include "sdstor_types.h"

#include "vfs_types.h"

#include "sector_api.h"

extern tai_hook_ref_t gc_hook_ref;
extern SceUID gc_hook_id;

extern tai_hook_ref_t init_mmc_hook_ref;
extern SceUID init_mmc_hook_id;

extern tai_hook_ref_t init_sd_hook_ref;
extern SceUID init_sd_hook_id;

extern SceUID patch_uids[3];

extern tai_hook_ref_t gen_init_hook_refs[3];
extern SceUID gen_init_hook_uids[3];

extern tai_hook_ref_t load_mbr_hook_ref;
extern SceUID load_mbr_hook_id;

extern tai_hook_ref_t  mnt_pnt_chk_hook_ref;
extern SceUID mnt_pnt_chk_hook_id;

extern tai_hook_ref_t mbr_table_init_hook_ref;
extern SceUID mbr_table_init_hook_id;

extern tai_hook_ref_t cmd55_41_hook_ref;
extern SceUID cmd55_41_hook_id;

extern tai_hook_ref_t sysroot_zero_hook_ref;
extern SceUID sysroot_zero_hook_id;

extern SceUID gen_init_1_patch_uid;
extern SceUID gen_init_2_patch_uid;

extern tai_hook_ref_t gen_read_hook_ref;
extern SceUID gen_read_hook_id;

extern tai_hook_ref_t sd_read_hook_ref;
extern SceUID sd_read_hook_id;

extern SceUID hs_dis_patch1_uid;
extern SceUID hs_dis_patch2_uid;

extern SceUID bus_size_patch_uid;

extern SceUID mbr_init_zero_patch1_uid;
extern SceUID mbr_init_zero_patch2_uid;

extern tai_hook_ref_t init_partition_table_hook_ref;
extern SceUID init_partition_table_hook_id;

extern tai_hook_ref_t create_device_handle_hook_ref;
extern SceUID create_device_handle_hook_id;

extern tai_hook_ref_t send_command_hook_ref;
extern SceUID send_command_hook_id;

extern tai_hook_ref_t sceErrorHistoryPostError_hook_ref;
extern SceUID sceErrorHistoryPostError_hook_id;

extern tai_hook_ref_t sceErrorHistoryUpdateSequenceInfo_hook_ref;
extern SceUID sceErrorHistoryUpdateSequenceInfo_hook_id;

extern tai_hook_ref_t sceErrorGetExternalString_kernel_hook_ref;
extern SceUID sceErrorGetExternalString_kernel_hook_id;

extern tai_hook_ref_t sceErrorHistoryGetError_hook_ref;
extern SceUID sceErrorHistoryGetError_hook_id;

extern tai_hook_ref_t ksceKernelCreateThread_hook_ref;
extern SceUID ksceKernelCreateThread_hook_id;

extern tai_hook_ref_t sceKernelCreateThreadForUser_hook_ref;
extern SceUID sceKernelCreateThreadForUser_hook_id;

extern tai_hook_ref_t sceIoOpenForDriver_hook_ref;
extern SceUID sceIoOpenForDriver_hook_id;

extern tai_hook_ref_t ksceKernelWaitSema_hook_ref;
extern SceUID ksceKernelWaitSema_hook_id;

extern tai_hook_ref_t ksceKernelSignalSema_hook_ref;
extern SceUID ksceKernelSignalSema_hook_id;

extern tai_hook_ref_t vshSblAuthMgrVerifySpsfo_hook_ref;
extern SceUID vshSblAuthMgrVerifySpsfo_hook_id;

extern tai_hook_ref_t debug_printf_callback_invoke_ref;
extern SceUID debug_printf_callback_invoke_id;

extern tai_hook_ref_t gc_22fd5d23_hook_ref;
extern SceUID gc_22fd5d23_hook_id;

extern tai_hook_ref_t appmgr_23D642C_hook_ref;
extern SceUID appmgr_23D642C_hook_id;

extern tai_hook_ref_t sceAppMgrGameDataMountForDriver_hook_ref;
extern SceUID sceAppMgrGameDataMountForDriver_hook_id;

extern tai_hook_ref_t appmgr_23D9B50_hook_ref;
extern SceUID appmgr_23D9B50_hook_id;

extern tai_hook_ref_t sceAppMgrGameDataVfsMountForDriver_hook_ref;
extern SceUID sceAppMgrGameDataVfsMountForDriver_hook_id;

extern tai_hook_ref_t sceFiosKernelOverlayRemoveForProcessForDriver_hook_ref;
extern SceUID sceFiosKernelOverlayRemoveForProcessForDriver_hook_id;

extern tai_hook_ref_t proc_read_sealedkey_23D6EA0_hook_ref;
extern SceUID proc_read_sealedkey_23D6EA0_hook_id;

extern tai_hook_ref_t sbl_acmgr_0b6e6cd7_hook_ref;
extern SceUID sbl_acmgr_0b6e6cd7_hook_id;

extern tai_hook_ref_t proc_generate_random_path_23D4FBC_hook_ref;
extern SceUID proc_generate_random_path_23D4FBC_hook_id;

extern tai_hook_ref_t proc_get_clearsign_stat_23D9A4C_hook_ref;
extern SceUID proc_get_clearsign_stat_23D9A4C_hook_id;

extern tai_hook_ref_t pfs_mgr_a772209c_hook_ref;
extern SceUID pfs_mgr_a772209c_hook_id;

extern tai_hook_ref_t pfs_mgr_2d48aea2_hook_ref;
extern SceUID pfs_mgr_2d48aea2_hook_id;

extern tai_hook_ref_t pfs_mgr_2190AEC_hook_ref;
extern SceUID pfs_mgr_2190AEC_hook_id;

extern tai_hook_ref_t iofilemgr_feee44a9_hook_ref;
extern SceUID iofilemgr_feee44a9_hook_id;

extern tai_hook_ref_t iofilemgr_d220539d_hook_ref;
extern SceUID iofilemgr_d220539d_hook_id;

extern tai_hook_ref_t iofilemgr_BF3848_hook_ref;
extern SceUID iofilemgr_BF3848_hook_id;

extern tai_hook_ref_t iofilemgr_BF3474_hook_ref;
extern SceUID iofilemgr_BF3474_hook_id;

extern tai_hook_ref_t iofilemgr_sceVfsMount_hook_ref;
extern SceUID iofilemgr_sceVfsMount_hook_id;

extern tai_hook_ref_t iofilemgr_sceVfsGetNewNode_hook_ref;
extern SceUID iofilemgr_sceVfsGetNewNode_hook_id;

extern tai_hook_ref_t iofilemgr_BF1AF0_hook_ref;
extern SceUID iofilemgr_BF1AF0_hook_id;

extern tai_hook_ref_t iofilemgr_BE5B30_hook_ref;
extern SceUID iofilemgr_BE5B30_hook_id;

extern tai_hook_ref_t iofilemgr_BF18CC_hook_ref;
extern SceUID iofilemgr_BF18CC_hook_id;

extern tai_hook_ref_t iofilemgr_BEDEB0_hook_ref;
extern SceUID iofilemgr_BEDEB0_hook_id;

extern tai_hook_ref_t iofilemgr_BE62E8_hook_ref;
extern SceUID iofilemgr_BE62E8_hook_id;

extern tai_hook_ref_t iofilemgr_BE5F10_hook_ref;
extern SceUID iofilemgr_BE5F10_hook_id;

extern tai_hook_ref_t iofilemgr_BE6788_hook_ref;
extern SceUID iofilemgr_BE6788_hook_id;

extern tai_hook_ref_t iofilemgr_BEE3C8_hook_ref;
extern SceUID iofilemgr_BEE3C8_hook_id;

extern tai_hook_ref_t iofilemgr_BEBAC0_hook_ref;
extern SceUID iofilemgr_BEBAC0_hook_id;

extern tai_hook_ref_t iofilemgr_BE61C4_hook_ref;
extern SceUID iofilemgr_BE61C4_hook_id;

extern tai_hook_ref_t iofilemgr_BE59BC_hook_ref;
extern SceUID iofilemgr_BE59BC_hook_id;

//------

extern tai_hook_ref_t iofilemgr_BEE364_hook_ref;
extern SceUID iofilemgr_BEE364_hook_id;

extern tai_hook_ref_t iofilemgr_BEE2D4_hook_ref;
extern SceUID iofilemgr_BEE2D4_hook_id;

extern tai_hook_ref_t iofilemgr_BEE2C4_hook_ref;
extern SceUID iofilemgr_BEE2C4_hook_id;

extern tai_hook_ref_t iofilemgr_BEDF5C_hook_ref;
extern SceUID iofilemgr_BEDF5C_hook_id;

extern tai_hook_ref_t iofilemgr_BECE80_hook_ref;
extern SceUID iofilemgr_BECE80_hook_id;

extern tai_hook_ref_t iofilemgr_BEC808_hook_ref;
extern SceUID iofilemgr_BEC808_hook_id;

extern tai_hook_ref_t iofilemgr_BEC620_hook_ref;
extern SceUID iofilemgr_BEC620_hook_id;

extern tai_hook_ref_t iofilemgr_BEC578_hook_ref;
extern SceUID iofilemgr_BEC578_hook_id;

extern tai_hook_ref_t iofilemgr_BEC56C_hook_ref;
extern SceUID iofilemgr_BEC56C_hook_id;

extern tai_hook_ref_t iofilemgr_BEC530_hook_ref;
extern SceUID iofilemgr_BEC530_hook_id;

extern tai_hook_ref_t iofilemgr_BEC010_hook_ref;
extern SceUID iofilemgr_BEC010_hook_id;

extern tai_hook_ref_t iofilemgr_BE5A38_hook_ref;
extern SceUID iofilemgr_BE5A38_hook_id;

extern tai_hook_ref_t iofilemgr_BE5814_hook_ref;
extern SceUID iofilemgr_BE5814_hook_id;

extern tai_hook_ref_t iofilemgr_BEBB84_hook_ref;
extern SceUID iofilemgr_BEBB84_hook_id;

extern tai_hook_ref_t iofilemgr_BEC7C0_hook_ref;
extern SceUID iofilemgr_BEC7C0_hook_id;

extern tai_hook_ref_t iofilemgr_BECE0C_hook_ref;
extern SceUID iofilemgr_BECE0C_hook_id;

//------

extern tai_hook_ref_t iofilemgr_exp_aa45010b_hook_ref;
extern SceUID iofilemgr_exp_aa45010b_hook_id;

extern tai_hook_ref_t iofilemgr_exp_21d57633_hook_ref;
extern SceUID iofilemgr_exp_21d57633_hook_id;

extern tai_hook_ref_t iofilemgr_exp_dc2d8bce_hook_ref;
extern SceUID iofilemgr_exp_dc2d8bce_hook_id;

extern tai_hook_ref_t iofilemgr_exp_6b3ca9f7_hook_ref;
extern SceUID iofilemgr_exp_6b3ca9f7_hook_id;

//------

extern tai_hook_ref_t pfs_mgr_219105C_hook_ref;
extern SceUID pfs_mgr_219105C_hook_id;

extern tai_hook_ref_t pfs_mgr_2193624_hook_ref;
extern SceUID pfs_mgr_2193624_hook_id;

extern tai_hook_ref_t pfs_mgr_2198928_hook_ref;
extern SceUID pfs_mgr_2198928_hook_id;

extern tai_hook_ref_t pfs_mgr_2199064_hook_ref;
extern SceUID pfs_mgr_2199064_hook_id;

extern tai_hook_ref_t pfs_mgr_219DE44_hook_ref;
extern SceUID pfs_mgr_219DE44_hook_id;

extern tai_hook_ref_t pfs_mgr_219DE7C_hook_ref;
extern SceUID pfs_mgr_219DE7C_hook_id;

extern tai_hook_ref_t pfs_mgr_2199144_hook_ref;
extern SceUID pfs_mgr_2199144_hook_id;

extern SceUID pfs_219DE44_check_patch1_uid;
extern SceUID pfs_219DE44_check_patch2_uid;

extern SceUID pfs_219DE44_check_patch3_uid;
extern SceUID pfs_219DE44_check_patch4_uid;

//------

extern tai_hook_ref_t app_mgr_23D5028_hook_ref;
extern SceUID app_mgr_23D5028_hook_id;

extern tai_hook_ref_t app_mgr_23EF934_hook_ref;
extern SceUID app_mgr_23EF934_hook_id;

extern tai_hook_ref_t app_mgr_75c96d25_hook_ref;
extern SceUID app_mgr_75c96d25_hook_id;

extern tai_hook_ref_t iofilemgr_BE8444_hook_ref;
extern SceUID iofilemgr_BE8444_hook_id;

extern tai_hook_ref_t iofilemgr_50a63acf_hook_ref;
extern SceUID iofilemgr_50a63acf_hook_id;

extern tai_hook_ref_t iofilemgr_BE584C_hook_ref;
extern SceUID iofilemgr_BE584C_hook_id;

extern tai_hook_ref_t iofilemgr_BE5CC4_hook_ref;
extern SceUID iofilemgr_BE5CC4_hook_id;

extern tai_hook_ref_t iofilemgr_f7dac0f5_hook_ref;
extern SceUID iofilemgr_f7dac0f5_hook_id;

extern tai_hook_ref_t iofilemgr_a5a6a55c_hook_ref;
extern SceUID iofilemgr_a5a6a55c_hook_id;

//------

extern tai_hook_ref_t pfs_mgr_21910F4_hook_ref;
extern SceUID pfs_mgr_21910F4_hook_id;

extern tai_hook_ref_t pfs_mgr_2193B44_hook_ref;
extern SceUID pfs_mgr_2193B44_hook_id;

//-----------

extern tai_hook_ref_t app_mgr_75192972_hook_ref;
extern SceUID app_mgr_75192972_hook_id;

extern tai_hook_ref_t iofilemgr_BEA704_hook_ref;
extern SceUID iofilemgr_BEA704_hook_id;

extern tai_hook_ref_t iofilemgr_9e347c7d_hook_ref;
extern SceUID iofilemgr_9e347c7d_hook_id;

extern tai_hook_ref_t iofilemgr_76b79bec_hook_ref;
extern SceUID iofilemgr_76b79bec_hook_id;

extern tai_hook_ref_t pfs_mgr_219112C_hook_ref;
extern SceUID pfs_mgr_219112C_hook_id;

extern tai_hook_ref_t pfs_mgr_2192190_hook_ref;
extern SceUID pfs_mgr_2192190_hook_id;

//------

extern SceUID pfs_219DE44_check_patch5_uid;
extern SceUID pfs_219DE44_check_patch6_uid;
extern SceUID pfs_219DE44_check_patch7_uid;

//------

extern tai_hook_ref_t app_mgr_e17efc03_hook_ref;
extern SceUID app_mgr_e17efc03_hook_id;

//------

extern SceUID pfs_80140f02_test_patch1_uid;
extern SceUID pfs_80140f02_test_patch2_uid;

extern tai_hook_ref_t pfs_mgr_219BF20_hook_ref;
extern SceUID pfs_mgr_219BF20_hook_id;

extern tai_hook_ref_t iofilemgr_BE7AEC_hook_ref;
extern SceUID iofilemgr_BE7AEC_hook_id;

extern tai_hook_ref_t iofilemgr_BF57F8_hook_ref;
extern SceUID iofilemgr_BF57F8_hook_id;

extern tai_hook_ref_t iofilemgr_BF66E4_hook_ref;
extern SceUID iofilemgr_BF66E4_hook_id;

extern tai_hook_ref_t iofilemgr_BF6B6C_hook_ref;
extern SceUID iofilemgr_BF6B6C_hook_id;

extern tai_hook_ref_t iofilemgr_BF69B4_hook_ref;
extern SceUID iofilemgr_BF69B4_hook_id;

extern tai_hook_ref_t iofilemgr_BF651C_hook_ref;
extern SceUID iofilemgr_BF651C_hook_id;

extern tai_hook_ref_t iofilemgr_BF89EC_hook_ref;
extern SceUID iofilemgr_BF89EC_hook_id;

extern tai_hook_ref_t iofilemgr_BF8BD4_hook_ref;
extern SceUID iofilemgr_BF8BD4_hook_id;

extern tai_hook_ref_t iofilemgr_BF7C00_hook_ref;
extern SceUID iofilemgr_BF7C00_hook_id;

//

extern tai_hook_ref_t iofilemgr_BF8BB0_hook_ref;
extern SceUID iofilemgr_BF8BB0_hook_id;

extern tai_hook_ref_t iofilemgr_BF7288_hook_ref;
extern SceUID iofilemgr_BF7288_hook_id;

extern tai_hook_ref_t iofilemgr_BFB5F0_hook_ref;
extern SceUID iofilemgr_BFB5F0_hook_id;

extern tai_hook_ref_t iofilemgr_BF88C4_hook_ref;
extern SceUID iofilemgr_BF88C4_hook_id;

extern tai_hook_ref_t iofilemgr_BFB794_hook_ref;
extern SceUID iofilemgr_BFB794_hook_id;

extern tai_hook_ref_t iofilemgr_BF7268_hook_ref;
extern SceUID iofilemgr_BF7268_hook_id;

extern tai_hook_ref_t iofilemgr_BF83A0_hook_ref;
extern SceUID iofilemgr_BF83A0_hook_id;

extern tai_hook_ref_t iofilemgr_BFB628_hook_ref;
extern SceUID iofilemgr_BFB628_hook_id;

extern tai_hook_ref_t iofilemgr_BF7228_hook_ref;
extern SceUID iofilemgr_BF7228_hook_id;

extern tai_hook_ref_t iofilemgr_BF83F8_hook_ref;
extern SceUID iofilemgr_BF83F8_hook_id;

extern tai_hook_ref_t iofilemgr_BFB748_hook_ref;
extern SceUID iofilemgr_BFB748_hook_id;

extern tai_hook_ref_t iofilemgr_BE7F4C_hook_ref;
extern SceUID iofilemgr_BE7F4C_hook_id;

extern tai_hook_ref_t iofilemgr_BFB2A8_hook_ref;
extern SceUID iofilemgr_BFB2A8_hook_id;

//------

extern tai_hook_ref_t pfs_facade_4238d2d2_hook_ref;
extern SceUID pfs_facade_4238d2d2_hook_id;

extern tai_hook_ref_t iofilemgr_0b54f9e0_hook_ref;
extern SceUID iofilemgr_0b54f9e0_hook_id;

//------

int gc_patch(int param0);
int init_mmc_hook(int sd_ctx_index, sd_context_part** result);
int init_sd_hook(int sd_ctx_index, sd_context_part** result);
int cmd55_41_hook(sd_context_global* ctx);
int gen_init_hook_1(void* ctx);
int gen_init_hook_2(void* ctx);
int gen_init_hook_3(void* ctx);
int sysroot_zero_hook();
int load_mbr_hook(int ctx_index);
int mnt_pnt_chk_hook(char* blockDeviceName, int mountNum, int* mountData);
int mbr_table_init_hook(char* blockDeviceName, int mountNum);

int gen_read_hook(char *ctx, char *buffer, int sector, int nSectors);
int sd_read_hook(sd_context_part* ctx, int sector, char* buffer, int nSectors);

int init_partition_table_hook(int arg0, sdstor_mbr_ctx* data);
int create_device_handle(partition_entry* pentry, int unk1, sd_stor_device_handle** handle);

int send_command_hook(sd_context_global* ctx, cmd_input* cmd_data1, cmd_input* cmd_data2, int nIter, int num);

int sceErrorHistoryPostError_hook(void* src_user);
int sceErrorHistoryUpdateSequenceInfo_hook(void* src_user, int unk1);
int sceErrorGetExternalString_kernel_hook(void* dest_user, int unk);
int sceErrorHistoryGetError_hook(int unk0, void* dest_user);

int ksceKernelCreateThread_hook(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, int cpuAffinityMask, const SceKernelThreadOptParam *option);
int sceKernelCreateThreadForUser_hook(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, int cpuAffinityMask, const SceKernelThreadOptParam *option);

SceUID sceIoOpenForDriver_hook(const char *file, int flags, SceMode mode, void *args);

int ksceKernelWaitSema_hook(SceUID semaid, int signal, SceUInt *timeout);

int ksceKernelSignalSema_hook(SceUID semaid, int signal);

typedef struct spsfo_opt
{
  uint32_t unk_0;
  uint32_t unk_4;
} spsfo_opt;

int vshSblAuthMgrVerifySpsfo_hook(char *path_user, char *dest_user, int maxSize, spsfo_opt *opt);

int debug_printf_callback_invoke_hook(int unk0, int unk1, int unk2);

int gc_22fd5d23_hook(char* buffer);

int appmgr_23D642C_hook(int unk0, int unk1);

int sceAppMgrGameDataMountForDriver_hook(int unk0, int unk1, int unk2, int unk3);

int appmgr_23D9B50_hook(int unk0, int unk1, int unk2, int unk3, int arg_0, int arg_4, int arg_8, int arg_C);

int sceAppMgrGameDataVfsMountForDriver_hook(int vshMountId);

int sceFiosKernelOverlayRemoveForProcessForDriver_hook(int unk0, int unk1);

int proc_read_sealedkey_23D6EA0_hook(int unk0, int unk1);

int sbl_acmgr_0b6e6cd7_hook(int unk0);

int proc_generate_random_path_23D4FBC_hook(int unk0, int unk1);

int proc_get_clearsign_stat_23D9A4C_hook(int unk0, int unk1);

int pfs_mgr_a772209c_hook(int unk0, int unk1, int unk2, int unk3, int arg_0, int arg_4);

int pfs_mgr_2d48aea2_hook(int unk0, int unk1, int unk2, int unk3);

int pfs_mgr_2190AEC_hook(int unk0, int unk1, int unk2, int unk3, int arg_0);

int iofilemgr_feee44a9_hook(int unk0, int unk1, int unk2);

int iofilemgr_d220539d_hook(int unk0, int unk1, int unk2);

typedef struct ctx_BF3848
{
   uint32_t operation ;  //unk0
   uint32_t unk_4 ;  //var_38 - 3
   uint32_t unk_8 ;  //var_34 - 0 - arg0
   uint32_t unk_C ;  //var_30
   uint32_t unk_10 ; //var_2C - 0
   uint32_t unk_14 ; //var_28 - 0 - smth from thread ctx
   uint32_t unk_18 ; //var_24 - 0
   uint32_t unk_1C ; //var_20 - 0 - current thread id?
   int32_t error ;   //var_1C - 0 - result / error
   uint32_t unk_24 ; //var_18 - 0
} ctx_BF3848;

int iofilemgr_BF3848_hook(ctx_BF3848* ctx);

int iofilemgr_BF3474_hook(ctx_BF3848* ctx);

int iofilemgr_sceVfsMount_hook(vfs_mount_point_info_base *mountInfo);

int iofilemgr_sceVfsGetNewNode_hook(vfs_mount* mount, node_ops2* ops, int unused, vfs_node** node);

int iofilemgr_BE5B30_hook(vfs_node* a0, vfs_node* a1, void* a2, int a3, int a4);

int iofilemgr_BF18CC_hook(vfs_mount* a0, uint32_t* a1);

int iofilemgr_BF1AF0_hook(vfs_node *cur_node, int unk1, vfs_node *node);

int iofilemgr_BEDEB0_hook(uint32_t* a0, int a1, vfs_node* a2, int a3);

int iofilemgr_BE62E8_hook(int unk0, int unk1, int unk2, int unk3, int arg_0);

int iofilemgr_BE5F10_hook(vfs_node *ctx, int unk1, int unk2, int unk3, int arg_0);

int iofilemgr_BE6788_hook(void *unk0, int unk1, vfs_node **vnode, int unk3, int unk4);

int iofilemgr_BEE3C8_hook(vfs_node *ctx);

void* iofilemgr_BEBAC0_hook();

int iofilemgr_BE61C4_hook(int unk0);

int iofilemgr_BE59BC_hook(vfs_node* a0, void* a1);

//------------

int iofilemgr_BEE364_hook(vfs_node *node);

int iofilemgr_BEE2D4_hook();

int iofilemgr_BEE2C4_hook();

int iofilemgr_BEDF5C_hook(int unk0, int unk1);

int iofilemgr_BECE80_hook(int unk0);

int iofilemgr_BEC808_hook(int unk0);

int iofilemgr_BEC620_hook(int unk0);

int iofilemgr_BEC578_hook(int unk0, int unk1);

int iofilemgr_BEC56C_hook(void *unk0, void *unk1);

int iofilemgr_BEC530_hook(int unk0);

int iofilemgr_BEC010_hook(int unk0);

int iofilemgr_BE5A38_hook(int unk0, int unk1);

int iofilemgr_BE5814_hook(vfs_node *ctx);

int iofilemgr_BEBB84_hook(vfs_mount* arg0, vfs_node* arg1, SceUID heapid, vfs_add_data* arg3);

vfs_add_data* iofilemgr_BEC7C0_hook(char *filesystem);

void* iofilemgr_BECE0C_hook();

//------------

int iofilemgr_exp_aa45010b_hook(vfs_node *ctx);

int iofilemgr_exp_21d57633_hook(vfs_node *node);

int iofilemgr_exp_dc2d8bce_hook(void *mutex);

int iofilemgr_exp_6b3ca9f7_hook(void *mutex);

//-----------

typedef struct pfs_mgr_219105C_hook_args
{
  vfs_mount* arg0;
  void* arg1;
}pfs_mgr_219105C_hook_args;

int pfs_mgr_219105C_hook(pfs_mgr_219105C_hook_args* args);

int pfs_mgr_2193624_hook(int arg0, int arg1);

int pfs_mgr_2198928_hook(int unk0, int unk1, int unk2, int unk3);

typedef struct node_holder
{
   vfs_node * node;
   uint32_t unk_4;
} node_holder;

typedef struct result_2199144
{
  uint32_t unk_0;
  uint32_t unk_4;
}result_2199144;

//read wrapper
int pfs_mgr_2199064_hook(node_holder* unk0, char *buffer, int size, int ignored, int offsetLo, int offsetHi, uint32_t* readBytes);

int pfs_mgr_219DE44_hook(char *unk0, char *unk1);

typedef struct ctx_21A27B8_70 //size is 0x160
{
   //8 + 44 + 14 = 60

   char unk_70[0x8]; //header
   
   uint32_t unk_78; //field
   uint32_t unk_7C;

   uint32_t unk_80; //field
   uint32_t unk_84;
   uint32_t unk_88; //field
   uint32_t unk_8C;

   uint32_t unk_90; //field
   uint32_t unk_94; //field
   uint32_t unk_98; //field
   uint32_t unk_9C; //field

   uint32_t unk_A0;
   uint32_t unk_A4;

   char unk_A8[0x14];
   char unk_BC[0x14];
   char unk_D0[0x100];

} ctx_21A27B8_70;

typedef struct ctx_21A27B8_18 //size is unknown
{
   uint32_t unk_0;
   uint32_t unk_4;

}ctx_21A27B8_18;

typedef struct ctx_21A27B8_20 //size is 0x20
{
   ctx_21A27B8_18* unk_0;
   node_holder* unk_4;
   uint32_t unk_8;
   uint32_t unk_C;

   uint32_t unk_10;
   uint32_t unk_14;
   uint32_t unk_18;
   uint32_t unk_1C;

}ctx_21A27B8_20;

typedef struct ctx_21A27B8_40 //size is 0x28
{
   uint32_t unk_40;
   uint32_t unk_44;
   uint32_t unk_48;

   char unk_4C[0x14];

   uint32_t unk_60;
   uint32_t unk_64;

}ctx_21A27B8_40;

typedef struct ctx_21A27B8
{
   char data0[0x14];

   node_holder * unk_14;
   ctx_21A27B8_18* unk_18; //field
   uint32_t unk_1C;

   ctx_21A27B8_20 unk_20; //field

   ctx_21A27B8_40 unk_40; //field

   uint32_t unk_68; //field
   uint32_t unk_6C;

   ctx_21A27B8_70 unk_70;
   
   uint32_t unk_1D0;

}ctx_21A27B8;

int pfs_mgr_219DE7C_hook(char bytes14[0x14], ctx_21A27B8* base, ctx_21A27B8_70* data_base, int size);

int pfs_mgr_2199144_hook(node_holder* nodeHolder, result_2199144* result_pair);

//---------

int app_mgr_23D5028_hook(SceUID pid, int unk1, int unk2, int unk3);

int app_mgr_23EF934_hook(int unk0, int unk1);

//my vita sdk version is too old so I do not have SceIoStat in kernel headers

/** Structure to hold the status information about a file */
typedef struct SceIoStat {
	SceMode	st_mode;
	unsigned int	st_attr;
	/** Size of the file in bytes. */
	SceOff	st_size;
	/** Creation time. */
	SceDateTime	st_ctime;
	/** Access time. */
	SceDateTime	st_atime;
	/** Modification time. */
	SceDateTime	st_mtime;
	/** Device-specific data. */
	unsigned int	st_private[6];
} SceIoStat;

//ksceIoGetstat hook
int app_mgr_75c96d25_hook(const char *file, SceIoStat *stat);

//ksceIoGetstat thread
int iofilemgr_BE8444_hook(void* args);

int iofilemgr_50a63acf_hook(vfs_node* n0, int unk1, int unk2);

int iofilemgr_BE584C_hook(int unk0, int unk1, int unk2, int unk3, vfs_node **node);

int iofilemgr_BE5CC4_hook(vfs_node *ctx, int unk1, int unk2, int unk3, int arg_0);

//vfs_func13
int iofilemgr_f7dac0f5_hook(vfs_mount *mnt, int unk1, int unk2, int unk3, int arg_0, int arg_4, int arg_8);

//vfs_node_func4
int iofilemgr_a5a6a55c_hook(vfs_node *ctx, int unk1, int unk2, int unk3);

//-----------

//vfs_node_2_func4
int pfs_mgr_21910F4_hook(void* args);

int pfs_mgr_2193B44_hook(void* unk0, void* unk1, int num2);

//-----------

int app_mgr_75192972_hook(const char* name, int flags, SceMode mode);

int iofilemgr_BEA704_hook(void* args);

//vfs_node_func2
int iofilemgr_9e347c7d_hook(vfs_node *n0, int unk1, int unk2, int unk3, int arg_0);

//vfs_node_func1
int iofilemgr_76b79bec_hook(vfs_node *ctx, int unk1, int unk2, int unk3);

//-----------

//vfs_node_2_func1
int pfs_mgr_219112C_hook(void* args);

int pfs_mgr_2192190_hook(int unk0, int unk1, int unk2, int unk3, int arg_0);

//----------

//sceIoReadForDriver
int app_mgr_e17efc03_hook(SceUID fd, char *data, SceSize size);

int pfs_mgr_219BF20_hook(int unk0);

int iofilemgr_BE7AEC_hook(void* args);

int iofilemgr_BF57F8_hook(SceUID fd, void *data, SceSize size, void* ptr);

typedef unsigned int io_device_code;

typedef struct io_context //size is 0xB8
{
   uint32_t unk_0;
   uint32_t unk_4;
   uint32_t unk_8;
   uint32_t unk_C;
   
   uint32_t unk_10;
   uint32_t unk_14;
   void *data_0; //0x18
   uint32_t unk_1C;
   
   SceSize size_0; //0x20
   uint32_t io_op_index; // 0x24
   uint32_t ioctlOutdata0;
   io_device_code device_code; // 0x2C - ioctlOutdata1
   
   char async; //0x30
   char unk_31;
   char unk_32;
   char unk_33;
   uint32_t pid; //0x34 = ksceKernelGetProcessId
   uint32_t tid; //0x38 = ksceKernelGetThreadId
   uint32_t unk_3C; // = SceThreadmgrForDriver_fa54d49a
   
   uint32_t unk_40; // = SceThreadmgrForDriver_332e127c
   uint32_t cpu_mask; // 0x44 = ksceKernelGetThreadCpuAffinityMask
   uint32_t thread_priority; //0x48 = ksceKernelGetThreadCurrentPriority
   void* list0; //0x4C pointer to some linked list with elements of size 0x14 
   
   uint32_t listSize; //0x50 = number of items in linst
   void* list1; //0x54 = pointer to some linked list with elements of size 0x14 
   uint32_t sizeOfList; //0x58 = size of list in bytes
   uint16_t state; // 0x5C = bit 16 of state where state = (MRC p15, 0, state, c13, c0, 3)
   uint16_t unk_5E;
   
   uint32_t unk_60;
   SceUID fd; //0x64
   void *data_1; //0x68
   uint32_t unk_6C;
   
   SceSize size_1; //0x70
   uint32_t unk_74;
   uint32_t unk_78; // = -1
   uint32_t unk_7C; // = -1
   
   uint32_t op_result; // 0x80 - result of i/o operation
   uint32_t unk_84;
   void* unk_88;
   uint32_t unk_8C;
   
   uint32_t unk_90;
   uint32_t unk_94;
   uint32_t unk_98;
   uint32_t unk_9C;
   
   uint32_t unk_A0;
   uint32_t unk_A4;
   uint32_t unk_A8;
   uint32_t unk_AC;
   
   SceInt64 unk_B0; // = ksceKernelGetSystemTimeWide
   
}io_context;

int iofilemgr_BF66E4_hook(io_scheduler_dispatcher *disp, io_context *ioctx);

int iofilemgr_BF6B6C_hook(io_context *ioctx, int	ioctlOutdata1, int ioctlOutdata0, int async, int arg_0_in, int arg_4_in, void *pool_A0,	int num, SceUID	fd, void *data,	SceSize	size);

int iofilemgr_BF69B4_hook(io_context *ioctx, int io_op_index, int async, int unk3, int arg_0, io_device_code ioctlOutdata1,	int ioctlOutdata0, void	*pool_A0, int num);

int iofilemgr_BF651C_hook(SceUID fd, void *outdata, int indataValue);

int iofilemgr_BF89EC_hook(io_scheduler *shed, io_context *ioctx);

int iofilemgr_BF8BD4_hook(io_scheduler *shed, io_context *ioctx);

//main i/o dispatcher
int iofilemgr_BF7C00_hook(io_context *ioctx);

//

int iofilemgr_BF8BB0_hook(io_scheduler *shed, io_context *ioctx);
int iofilemgr_BF7288_hook(io_context *ioctx);
int iofilemgr_BFB5F0_hook(io_scheduler *shed, io_context *ioctx, int num);
int iofilemgr_BF88C4_hook(io_context *ioctx, int *resptr);
int iofilemgr_BFB794_hook(io_context *ioctx, int unk);
int iofilemgr_BF7268_hook(io_context *ioctx, void *ptr1);
int iofilemgr_BF83A0_hook(int *unk0, int *unk1, io_context *ioctx);
int iofilemgr_BFB628_hook(io_context *ioctx, int unk1, int err);
int iofilemgr_BF7228_hook(io_context *ioctx);
int iofilemgr_BF83F8_hook(io_scheduler *shed, io_context *ioctx);
int iofilemgr_BFB748_hook(io_scheduler *shed, io_context *ioctx);

typedef struct sceIoLseekForDriver_args
{
  SceUID fd;
  int whence;
  SceOff offset;
}sceIoLseekForDriver_args;

int iofilemgr_BE7F4C_hook(sceIoLseekForDriver_args *ctx);

int iofilemgr_BFB2A8_hook(io_context *ioctx, void* param);

typedef struct sceIoPreadForDriver_args
{
  SceUID uid;
  void *data;
  int size;
  int dummy;
  SceOff offset;
} sceIoPreadForDriver_args;

//pfs pread impl
int pfs_facade_4238d2d2_hook(sceIoPreadForDriver_args* args);

int iofilemgr_0b54f9e0_hook(sceIoPreadForDriver_args *args);