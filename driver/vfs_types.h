#pragma once

#include <psp2kern/types.h>

#include <stdint.h>

#include "sdstor_types.h"

#pragma pack(push, 1)

//probably vnodeops
typedef struct node_ops2 // size is 0x74 (29 pointers)
{
  int (*func1)(void* ctx); // sceIoOpenForDriver
  int (*func2)(void* ctx); // sceIoOpenForDriver (another one ?)
  int (*func3)(void* ctx); // ?
  int (*func4)(void* ctx); // ?
  int (*func5)(void* ctx); // sceIoReadForDriver
  int (*func6)(void* ctx); // sceIoWriteForDriver
  int (*func7)(void* ctx); // sceIoLseekForDriver or sceIoLseek32?
  int (*func8)(void* ctx); // sceIoIoctlForDriver
  int (*func9)(void* ctx); // ?
  int (*func10)(void* ctx); // sceIoMkdirForDriver
  int (*func11)(void* ctx); // sceIoRmdirForDriver
  int (*func12)(void* ctx); // sceIoDopenForDriver
  int (*func13)(void* ctx); // sceIoDcloseForDriver
  int (*func14)(void* ctx); // sceIoDreadForDriver
  int (*func15)(void* ctx); // sceIoGetstatForDriver or sceIoGetstatByFdForDriver
  int (*func16)(void* ctx); // sceIoChstatForDriver or sceIoChstatByFdForDriver
  int (*func17)(void* ctx); // sceIoRenameForDriver
  int (*func18)(void* ctx); // not implemented by all
  int (*func19)(void* ctx); // sceIoPreadForDriver
  int (*func20)(void* ctx); // sceIoPwriteForDriver
  int (*func21)(void* ctx); // ?
  int (*func22)(void* ctx); // not referenced
  int (*func23)(void* ctx); // not referenced
  int (*func24)(void* ctx); // sceIoSyncForDriver or sceIoSyncByFdForDriver
  int (*func25)(void* ctx); // sceIoGetstatByFdForDriver
  int (*func26)(void* ctx); // sceIoChstatByFdForDriver
  int (*func27)(void* ctx); // ?
  int (*func28)(void* ctx); // ?
  int (*func29)(void* ctx); // not implemented by all
} node_ops2;

typedef struct vfs_node_unk_54
{
  uint32_t unk_0;
  uint32_t unk_4;

  uint32_t unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_54;

typedef struct vfs_node_unk_70_1
{
  void* unk_0;
  void* unk_4;

  void* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70_1;

typedef struct vfs_node_unk_70_3
{
  void* unk_0;
  void* unk_4;

  void* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70_3;

typedef struct vfs_node_unk_70_2
{
  uint32_t unk_0;
  uint32_t unk_4;

  uint32_t unk_8;
  vfs_node_unk_70_3* unk_C;

  //can be more bytes
} vfs_node_unk_70_2;

typedef struct vfs_node_unk_70_4
{
  void* unk_0;
  void* unk_4;

  void* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70_4;

typedef struct vfs_node_unk_70
{
  vfs_node_unk_70_1* unk_0;
  vfs_node_unk_70_2* unk_4;

  vfs_node_unk_70_4* unk_8;
  uint32_t unk_C;

  //can be more bytes
} vfs_node_unk_70;

typedef struct vfs_device_info //size is 0xC
{
   partition_entry* partition;
   sd_stor_device* device;
   uint32_t unk_8;
}vfs_device_info;

typedef struct vfs_node
{
   uint32_t unk_0;
   uint32_t unk_4; //uid ?, not ptr
   uint32_t unk_8;
   uint32_t unk_C; //uid ?, not ptr

   uint32_t unk_10; //num
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

   node_ops2 *ops;
   uint32_t unk_44;
   void* dev_info;
   uint32_t unk_4C; // not a pointer

   struct vfs_node* prev_node;
   vfs_node_unk_54* unk_54; //ptr
   uint32_t unk_58; //num
   uint32_t unk_5C;

   uint32_t unk_60;
   uint32_t unk_64;
   uint32_t unk_68;
   SceUID pool_uid;

   vfs_node_unk_70* unk_70; //ptr
   uint32_t unk_74; //num
   uint32_t unk_78; //num
   uint32_t unk_7C;

   uint32_t unk_80; //ptr ?
   uint32_t unk_84;
   uint32_t unk_88; //num
   uint32_t unk_8C;

   uint32_t unk_90; //num
   uint32_t unk_94; //num
   uint32_t unk_98;
   uint32_t unk_9C;

   uint32_t unk_A0;
   uint32_t unk_A4;
   uint32_t unk_A8;
   uint32_t unk_AC;

   uint32_t unk_B0;
   uint32_t unk_B4;
   uint32_t unk_B8;
   uint32_t unk_BC;

   uint32_t unk_C0;
   uint32_t unk_C4;
   uint32_t unk_C8;
   uint32_t unk_CC;

   uint32_t unk_D0; //num
}vfs_node;

typedef struct node_ops1 // size is 0x34 (13 pointers)
{
  int (*func1)(void* ctx);
  int (*func2)(void* ctx); // ?
  int (*func3)(void* ctx);
  int (*func4)(void* ctx);
  int (*func5)(void* ctx); // not implemented by all
  int (*func6)(void* ctx); // not implemented by all
  int (*func7)(void* ctx);
  int (*func8)(void* ctx); // not implemented by all
  int (*func9)(void* ctx); // called by sceVfsAddVfs
  int (*func10)(void* ctx); // called by sceVfsDeleteVfs
  int (*func11)(void* ctx); // not implemented by all
  int (*func12)(void* ctx); // sceIoDevctlForDriver
  int (*func13)(void* ctx); // ?
} node_ops1;

typedef struct vfs_add_data
{
    struct node_ops1* funcs1;
    const char *name; //max size is 0x20
    int flags; //0x0E
    int unk_C; //0x01 / 0x00 (root, leaf ?)
    
    int unk_10; //0x10
    struct node_ops2* funcs2;
    int unk_18; //0x00
    struct vfs_add_data* next_element; //ptr to next element, confirmed
} vfs_add_data;

typedef struct vfs_block_dev_info //size is 0x14
{
  char* vitaMount;
  char* filesystem; // Some name, I guess it is filesystem
  char* blockDevicePrimary;
  char* blockDeviceSecondary; // can be 0
  uint32_t vshMountId; //must be same as in mount_point_info
} vfs_block_dev_info;

typedef struct vfs_mount_point_info_base
{
  char* unixMount;
  uint32_t unk_4; //zero
  uint32_t devMajor;
  uint32_t devMinor;

  char* filesystem;
  uint32_t unk_14; //zero
  struct vfs_block_dev_info* blockDev1;
  uint32_t unk_1C; //zero
} vfs_mount_point_info_base;

typedef struct vfs_unmount_data
{
    const char *mountpoint;
    int flags;
} vfs_unmount_data; 

typedef struct vfs_node_info //size is 0x38
{
   char name[0x20];
   struct vfs_node* node; //can be zero
   uint32_t unk_24; // probably size of vfs_node
   uint32_t unk_28; // probably flags
   struct vfs_node_info* unk_2C; //can be zero
   struct vfs_node_info* unk_30; //can be zero
   struct vfs_node_info* unk_34; //can be zero
}vfs_node_info;

#pragma pack(pop)