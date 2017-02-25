#pragma once

#include <psp2kern/types.h>

#include <taihen.h>

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