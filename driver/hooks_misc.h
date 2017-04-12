#pragma once

#include <psp2kern/types.h>

#include <taihen.h>

#include "sdstor_types.h"

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