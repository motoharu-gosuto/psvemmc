#pragma once

#include <psp2kern/types.h>

#include <taihen.h>

#include <stdint.h>

#include "vfs_types.h"

extern tai_hook_ref_t sceVfsMount_hook_ref;
extern tai_hook_ref_t sceVfsAddVfs_hook_ref;
extern tai_hook_ref_t sceVfsUnmount_hook_ref;
extern tai_hook_ref_t sceVfsDeleteVfs_hook_ref;
extern tai_hook_ref_t sceVfsGetNewNode_hook_ref;

extern SceUID sceVfsMount_hook_id;
extern SceUID sceVfsAddVfs_hook_id;
extern SceUID sceVfsUnmount_hook_id;
extern SceUID sceVfsDeleteVfs_hook_id;
extern SceUID sceVfsGetNewNode_hook_id;

int sceVfsMount_hook(vfs_mount_point_info_base* data);
int sceVfsAddVfs_hook(vfs_add_data* data);
int sceVfsUnmount_hook(vfs_unmount_data* data);
int sceVfsDeleteVfs_hook(const char* name, void** deleted_node);
int sceVfsGetNewNode_hook(void* ctx, node_ops2* ops, int unused, vfs_node** node);
