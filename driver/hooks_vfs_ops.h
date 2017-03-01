#pragma once

#include <psp2kern/types.h>

#include <stdint.h>

#include "vfs_types.h"

#include <taihen.h>

extern tai_hook_ref_t sdstor_dev_fs_refs[13];
extern SceUID sdstor_dev_fs_ids[13];
 
int vfs_func1(void* ctx);
int vfs_func3(void* ctx);

typedef struct ctx_C175D0
{
   int unk_0;
   int unk_4;
   int unk_8;
   
   char* blockDeviceName;
   int nameLength;
   
   int unk_14;
   int unk_18;
} ctx_C175D0;

int vfs_func12(void* ctx);

typedef struct vfs_func13_args
{
   uint32_t unk_0;
   char* blockDeviceName;
   uint32_t unk_8;
   uint32_t unk_C;

   char* numericName;
   uint32_t blockDeviceNameLength;
   uint32_t* numericNameLength;
}vfs_func13_args;

int vfs_func13(void* ctx);

typedef struct vnf1_arg1
{
  char* blockDevice;
  uint32_t nameLength;
  char* unixMount;
}vnf1_arg1;

typedef struct vfs_node_func1_args
{
   struct vfs_node* node;
   struct vnf1_arg1* arg1;
   uint32_t arg2;
   uint32_t arg3;
}vfs_node_func1_args;

int vfs_node_func1(void* ctx);

int vfs_node_func3(void* ctx);

typedef struct vnf4_arg2
{
   char* blockDeviceName;
   int nameLength;
} vnf4_arg2;

typedef struct vfs_node_func4_args
{
   vfs_node* node;
   vfs_node** new_node; //result
   vnf4_arg2* dev; //mount
   uint32_t arg3;
}vfs_node_func4_args;

int vfs_node_func4(void* ctx);
int vfs_node_func5(void* ctx);
int vfs_node_func6(void* ctx);

typedef struct vfs_node_func7_args
{
   struct vfs_node* node;
   void* arg1; //some ptr
   uint32_t arg2; //0x200 or 0 (size?)
   uint32_t arg3; 
   uint32_t arg_0; //0x1 or 0x0
}vfs_node_func7_args;

int vfs_node_func7(void* ctx);

typedef struct vnf9_arg2 //most likely this type is also vfs_node (same partial layout of fields)
{
   char* blockDeviceName;
   int nameLength;
   uint32_t unk_8;
   uint32_t unk_C;
   
   uint32_t unk_10;
   uint32_t unk_14;
   uint32_t unk_18;
   uint32_t unk_1C;
   
   uint32_t unk_20;
   uint32_t unk_24;
   uint32_t unk_28;
   uint32_t unk_2C;
   
   uint32_t unk_30;
   uint32_t unk_34;
   uint32_t unk_38;
   uint32_t unk_3C;
   
   uint32_t unk_40;
   uint32_t unk_44;
   partition_entry** unk_48;
}vnf9_arg2;

typedef struct vfs_node_func9_args
{
   struct vfs_node* node0;
   struct vfs_node* node1;
   struct vnf9_arg2* arg2;
   uint32_t arg3;
}vfs_node_func9_args;

int vfs_node_func9(void* ctx);
int vfs_node_func19(void* ctx);
int vfs_node_func20(void* ctx);