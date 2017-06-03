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

typedef struct vfs_mount_cc
{
   uint32_t unk_0;
   uint32_t unk_4;
   uint32_t unk_8; // pointer to data section, offset 34
   uint32_t unk_C; // pointer to data section, offset 1C
} vfs_mount_cc;

struct vfs_node;

typedef struct vfs_fd_lock
{
   SceUID mutex_SceVfsFdLock; // 0
   SceUID cond_SceVfsFdCond; // 4
   uint32_t unk_8;
   uint32_t unk_C;

} vfs_fd_lock;

typedef struct vfs_mount //size is not known exactly, at least 0xD0
{
   uint32_t fast_mutex_SceVfsMnt;

   uint32_t unk_4;
   uint32_t unk_8;
   uint32_t unk_C;

   uint8_t data1[0x30];

   struct vfs_node* unk_40; // child ?

   SceUID pool;     // 0x44 - SceIoVfsHeap

   uint32_t unk_48; // = 0x101
   
   union _devMajor
   {
      struct _dmDword
      {
         uint32_t unk_4C;
      } dw;
      struct _dmWord
      {
         uint16_t unk_4C;
         uint16_t unk_4E;
      } w;
      struct _dmByte
      {
         uint8_t unk_4C;
         uint8_t unk_4D;
         uint8_t unk_4E;
         uint8_t unk_4F;
      } b;
   } devMajor;

   uint32_t devMinor; // 0x50

   struct vfs_node* unk_54;
   uint32_t unk_58;   // counter
   struct vfs_add_data* add_data; // 0x5C

   uint32_t unk_60;   // counter

   uint32_t unk_64;
   uint32_t unk_68;
   uint32_t unk_6C;

   struct vfs_mount* unk_70; // next ?
   struct vfs_mount* unk_74; // prev ?

   struct vfs_mount* unk_78; //singly linked list related to pointer 19D0 in data section

   struct vfs_block_dev_info* blockDev; // 0x7C

   char unixMount[0x40]; // unixMount /gro/exfat, ux/exfat etc

   uint32_t unk_C0;   

   uint32_t unk_C4; // = mountInfo->unk_14

   vfs_fd_lock* fd_lock_ptr; // C8 - points to area in this structure

   vfs_mount_cc* unk_CC; // CC - points to area in this structure

   vfs_fd_lock fd_lock; // D0

   vfs_mount_cc unk_E0; //E0;

} vfs_mount;

typedef struct vfs_node_70
{
   uint32_t unk_0;
   uint32_t unk_4;
   uint32_t unk_8;
   struct vfs_node_70* unk_C; //zero or pointer to next element

   void* unk_10; //pointer to struct of at least size 0x94
   uint32_t unk_14; //number

   uint32_t str_len_18;
   char* str_1C;
} vfs_node_70;

typedef struct vfs_node
{
   uint32_t unk_0;
   uint32_t unk_4;  // most likely SceUID of current thread
   uint32_t unk_8;  // counter
   SceUID event_flag_SceVfsVnode; // 0xC - event flag SceVfsVnode

   uint32_t evid_bits; // 0x10
   uint32_t unk_14;
   uint32_t unk_18;
   uint32_t unk_1C;

   uint8_t data1[0x20];
   
   node_ops2 *ops;  // 0x40
   uint32_t unk_44;
   void* dev_info;  //allocated on heap with uid from uid field
                    //this is device specific / node specific data
                    //for partition node this will be vfs_device_info*

   vfs_mount* node; // 0x4C

   struct vfs_node* prev_node; // 0x50

   struct vfs_node* unk_54; // copied from node base
   uint32_t unk_58;  // counter - probably counter of nodes
   uint32_t unk_5C;

   uint32_t unk_60;
   uint32_t unk_64;
   uint32_t unk_68;
   SceUID pool_uid;  // 0x6C - SceIoVfsHeap or other pool

   vfs_node_70* unk_70;
   uint32_t unk_74; // = 0x8000
   uint32_t unk_78; // some flag
   uint32_t unk_7C;

   uint32_t unk_80;
   uint32_t unk_84;
   uint32_t unk_88;
   uint32_t unk_8C;

   uint32_t unk_90;
   uint32_t unk_94;
   uint32_t unk_98;
   uint32_t unk_9C;

   uint8_t data2[0x30];
   
   uint32_t unk_D0; //cur_node->devMajor.w.unk_4E

   uint8_t data3[0x2C];
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

//-------

struct io_scheduler;

typedef struct io_scheduler_dispatcher //size is 0xB8
{
   char unk_0[0x50];

   struct io_scheduler* unk_50; // Internal
   struct io_scheduler* unk_54; // Game Card
   struct io_scheduler* unk_58; // Removable
   struct io_scheduler* unk_5C; // Host File System
   struct io_scheduler* unk_60; // Default
   
   uint32_t unk_64; // pointer to unknown module data section
   uint32_t unk_68; //SceIoSchedulerDispatcher fast mutex
   uint32_t unk_6C;
   
   char unk_70[0x30];
   
   uint32_t unk_A0;
   uint32_t unk_A4;
   
   SceUID unk_A8; //SceIoSchedulerDispatcher event flag uid
   uint32_t unk_AC; // counter
   uint32_t unk_B0; // flag
   uint32_t unk_B4; // 0
   
} io_scheduler_dispatcher;

typedef struct io_scheduler_item //size is 0x14 - allocated from SceIoScheduler heap
{
   struct io_scheduler* unk_0; // parent
   uint32_t unk_4; // 0
   uint32_t unk_8; // 0
   uint32_t unk_C; // 0
   uint32_t unk_10; // pointer to unknown module data section
}io_scheduler_item;

typedef struct io_scheduler //size is 0xA8 - allocated from SceIoScheduler heap
{
   uint32_t unk_0; //code: 0x100, 0x201, 0x202, 0x300, 0x0
                  //Internal, Game Card, Removable, Host File System, Default
                  
   struct io_scheduler_item* unk_4; //item 0
   struct io_scheduler_item* unk_8; //item 1
   struct io_scheduler_item* unk_C; //item 2
   
   struct io_scheduler_item* unk_10; //item 3
   struct io_scheduler_item* unk_14; //item 4
   struct io_scheduler_item* unk_18; //item 5
   struct io_scheduler_item* unk_1C; //item 6
   
   struct io_scheduler_item* unk_20; //item 7
   struct io_scheduler_item* unk_24; //item 8
   struct io_scheduler_item* unk_28; //item 9
   struct io_scheduler_item* unk_2C; //item 10
   
   struct io_scheduler_item* unk_30; //item 11
   struct io_scheduler_item* unk_34; //item 12
   struct io_scheduler_item* unk_38; //item 13
   struct io_scheduler_item* unk_3C; //item 14
   struct io_scheduler_item* unk_40; //item 15
   
   uint32_t unk_44; // flag1 0x40000 or 0x00
   uint32_t unk_48; // flag2 0x40000 or 0x00
   struct io_scheduler_dispatcher* unk_4C; // parent
   
   SceUID unk_50; //SceIoScheduler mutex
   SceUID unk_54; //SceIoScheduler cond
   
   char unk_58; // 0
   char unk_59; // 0
   char unk_5A; // 0
   char unk_5B; // 0
   
   uint32_t unk_5C; // 0
   
   //this is probably separate structure
   SceUID unk_60; //SceIoSchedWorker thread id
   SceUID unk_64; //SceIoScheduler mutex from self
   SceUID unk_68; //SceIoScheduler cond from self
   struct io_scheduler* unk_6C; //pointer to self
   
   uint32_t unk_70; // 0
   uint16_t unk_74; // 0
   uint16_t unk_76; // 0
   uint32_t unk_78; // 0
   uint32_t unk_7C; // 0
   
   uint32_t unk_80; //
   uint32_t unk_84; // some flag
   
   uint32_t unk_88; //0x80
   uint32_t unk_8C; //0x80
   
   uint32_t unk_90; //0x80
   uint32_t unk_94; //0x80
   uint32_t unk_98; //0x80
   uint32_t unk_9C; //0x80
   
   uint32_t unk_A0; // 0
   uint32_t unk_A4; // 0

}io_scheduler;   

#pragma pack(pop)