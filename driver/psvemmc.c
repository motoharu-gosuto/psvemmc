#include "psvemmc.h"
 
//taihen plugin by yifanlu was used as a reference:
//https://github.com/yifanlu/taiHEN/blob/master/taihen.c
 
#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "qsort.h" //since there are no exports in kernel that are documented, had to use source of qsort

#include <taihen.h>
#include <module.h>

#define FILE_WRITE(f, msg) if(f >= 0) ksceIoWrite(f, msg, sizeof(msg))
#define FILE_WRITE_LEN(f, msg) if(f >= 0) ksceIoWrite(f, msg, strlen(msg))

SceUID global_log_fd;

void open_global_log()
{
  global_log_fd = ksceIoOpen("ux0:dump/psvemmc_dump.bin", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);  
}

void close_global_log()
{
  if(global_log_fd >= 0)
    ksceIoClose(global_log_fd);
}

//=================================================
 
#pragma pack(push, 1)

struct sd_context_global;
struct cmd_input;

typedef struct cmd_input // size is 0x240
{
   uint32_t size; // 0x240
   uint32_t unk_4;
   uint32_t command;
   uint32_t argument;
   
   uint8_t data0[0x50];   
   
   struct cmd_input* next_cmd;
   uint32_t unk_64;
   uint32_t unk_68;
   uint32_t unk_6C;
   
   uint32_t unk_70;
   uint32_t unk_74;
   struct sd_context_global* gctx_ptr;
   uint32_t unk_7C;
   
   uint8_t data1[0x1C0];
} cmd_input;

typedef struct sd_context_data // size is 0xC0
{
    struct cmd_input* cmd_ptr;
    struct cmd_input* cmd_ptr_next;
    uint32_t unk_8;
    uint32_t unk_C;
    
    uint32_t dev_type_idx; // (1,2,3)
    struct sd_context_part* ctx;
    uint32_t unk_18;
    uint32_t unk_1C;

    uint32_t array_idx; // (0,1,2)
    uint32_t unk_24;
    uint32_t unk_28;
    uint32_t unk_2C;

    void* membase_1000; // membase of SceSdif (0,1,2) memblock of size 0x1000
    uint32_t unk_34;
    uint32_t unk_38;
    SceUID uid_1000; // UID of SceSdif (0,1,2) memblock of size 0x1000

    uint32_t unk_40; // SceKernelThreadMgr related, probably UID for SceSdif (0,1,2)
    uint32_t unk_44;
    uint32_t unk_48;
    uint32_t unk_4C;

    uint32_t unk_50;
    uint32_t unk_54;
    uint32_t unk_58;
    uint32_t unk_5C;

    uint32_t unk_60;
    uint32_t unk_64;
    uint32_t unk_68;
    uint32_t unk_6C;

    uint32_t unk_70;
    uint32_t unk_74;
    uint32_t unk_78;
    uint32_t unk_7C;      

    //it looks like this chunk is separate structure since offset 0x2480 is used too often

    uint32_t unk_80;
    SceUID uid_10000; // UID of SceSdif (0,1,2) memblock of size 0x10000
    void* membase_10000; // membase of SceSdif (0,1,2) memblock of size 0x10000
    uint32_t unk_8C;

    uint32_t unk_90;
    uint32_t lockable_int;
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
} sd_context_data;

typedef struct sd_context_part // size is 0x398
{
   struct sd_context_global* gctx_ptr;
   
   uint8_t data[0x38C];
   
   void* unk_390;
   uint32_t unk_394;
} sd_context_part;

typedef struct sd_context_global // size is 0x24C0
{
    struct cmd_input commands[16];
    struct sd_context_data ctx_data;
} sd_context_global;

typedef struct output_23a4ef01
{
    int unk_0;
    int unk_4;
    int unk_8;
    int unk_C;
}output_23a4ef01;

#pragma pack(pop)

#define SCE_SDIF_DEV_EMMC 0
#define SCE_SDIF_DEV_GAME_CARD 1
#define SCE_SDIF_DEV_WLAN_BT 2

sd_context_global* ksceSdifGetSdContextGlobal(int sd_ctx_idx);

sd_context_part* ksceSdifGetSdContextPartMmc(int sd_ctx_idx);
sd_context_part* ksceSdifGetSdContextPartSd(int sd_ctx_idx);
sd_context_part* ksceSdifGetSdContextPartSdio(int sd_ctx_idx);

int ksceSdifGetCardInsertState1(int sd_ctx_idx);
int ksceSdifGetCardInsertState2(int sd_ctx_idx);

int ksceSdifInitializeSdContextPartMmc(int sd_ctx_index, sd_context_part** result);
int ksceSdifInitializeSdContextPartSd(int sd_ctx_index, sd_context_part** result);

//uses CMD17 for single sector and CMD23, CMD24 for multiple sectors
int ksceSdifReadSectorAsync(sd_context_part* ctx, int sector, char* buffer, int nSectors);
int ksceSdifReadSector(sd_context_part* ctx, int sector, char* buffer, int nSectors);

//uses CMD18 for single sector and CMD23, CMD25 for multiple sectors
int ksceSdifWriteSectorAsync(sd_context_part* ctx, int sector, char* buffer, int nSectors);
int ksceSdifWriteSector(sd_context_part* ctx, int sector, char* buffer, int nSectors);

int ksceSdifCopyCtx(sd_context_part* ctx, output_23a4ef01* unk0);

//=================================================

int ksceMsifReadSector(int sector, char* buffer, int nSectors);
int ksceMsifWriteSector(int sector, char* buffer, int nSectors);
int ksceMsifEnableSlowMode();
int ksceMsifDisableSlowMode();
int ksceMsifGetSlowModeState();

int ksceMsifInit1();
int ksceMsifInit2(char* unk0_40);

//=================================================

//defalut size of sector for SD MMC protocol
#define SD_DEFAULT_SECTOR_SIZE 0x200

sd_context_part* g_emmcCtx = 0;

sd_context_part* g_gcCtx = 0;

int g_emmcCtxInitialized = 0;

int g_gcCtxInitialized = 0;

int g_msCtxInitialized = 0;

int g_bytesPerSector = 0;
int g_sectorsPerCluster = 0;

int g_clusterPoolInitialized = 0;

SceUID g_clusterPool = 0;

void* g_clusterPoolPtr = 0;

//this function initializes a pool for single cluster
//this way I hope to save some time on free/malloc operations
//when I need to read single cluster

//this method should be called after partition table is read
//so that we know excatly bytesPerSector and sectorsPerCluster

int psvemmcInitialize(int bytesPerSector, int sectorsPerCluster)
{
  if(g_clusterPoolInitialized != 0)
    return -1;
  
  if(bytesPerSector != SD_DEFAULT_SECTOR_SIZE)
    return -2;
  
  g_bytesPerSector = bytesPerSector;
  g_sectorsPerCluster = sectorsPerCluster;
  
  g_clusterPool = ksceKernelAllocMemBlock("cluster_pool_kernel", SCE_KERNEL_MEMBLOCK_TYPE_KERNEL_RW, g_bytesPerSector * g_sectorsPerCluster, 0);
  if(g_clusterPool < 0)
    return g_clusterPool;
  
  int res_1 = ksceKernelGetMemBlockBase(g_clusterPool, &g_clusterPoolPtr);
  if(res_1 < 0)
    return res_1;
  
  g_clusterPoolInitialized = 1;

  return 0;
}

int psvemmcDeinitialize()
{
  if(g_clusterPoolInitialized == 0)
    return -1;
  
  int res_1 = ksceKernelFreeMemBlock(g_clusterPool);
  if(res_1 < 0)
    return res_1;
  
  g_clusterPoolInitialized = 0;
  
  return 0;
}

//this reads 0x200 byte sector
//other size is not supported since this is a restriction for single packet of SD MMC protocol

int readSector(int sector, char* buffer)
{
  if(g_emmcCtxInitialized == 0)
    return -1;
  
  char buffer_kernel[SD_DEFAULT_SECTOR_SIZE];
  
  int res_1 = ksceSdifReadSectorAsync(g_emmcCtx, sector, buffer_kernel, 1);
  if(res_1 < 0)
    return res_1;
  
  int res_2 = ksceKernelMemcpyKernelToUser((uintptr_t)buffer, buffer_kernel, SD_DEFAULT_SECTOR_SIZE);
  if(res_2 < 0)
    return res_2;
  
  return 0;
}

//this reads 0x200 byte sector

int readSectorGc(int sector, char* buffer)
{
  if(g_gcCtxInitialized == 0)
    return -1;
  
  char buffer_kernel[SD_DEFAULT_SECTOR_SIZE];
  
  int res_1 = ksceSdifReadSectorAsync(g_gcCtx, sector, buffer_kernel, 1);
  if(res_1 < 0)
    return res_1;
  
  int res_2 = ksceKernelMemcpyKernelToUser((uintptr_t)buffer, buffer_kernel, SD_DEFAULT_SECTOR_SIZE);
  if(res_2 < 0)
    return res_2;
  
  return 0;
}

//this reads 0x200 byte sector

int readSectorMs(int sector, char* buffer)
{
  if(g_msCtxInitialized == 0)
    return -1;
  
  char buffer_kernel[SD_DEFAULT_SECTOR_SIZE];
  
  int res_1 = ksceMsifReadSector(sector, buffer_kernel, 1);
  if(res_1 < 0)
    return res_1;
  
  int res_2 = ksceKernelMemcpyKernelToUser((uintptr_t)buffer, buffer_kernel, SD_DEFAULT_SECTOR_SIZE);
  if(res_2 < 0)
    return res_2;
  
  return 0;
}

//this writes 0x200 byte sector
//other size is not supported since this is a restriction for single packet of SD MMC protocol

int writeSector(int sector, char* buffer)
{
  if(g_emmcCtxInitialized == 0)
    return -1;
  
  //not implemented
  return -2;
}

//this writes 0x200 byte sector

int writeSectorGc(int sector, char* buffer)
{
  if(g_gcCtxInitialized == 0)
    return -1;
  
  //not implemented
  return -2;
}

//this writes 0x200 byte sector

int writeSectorMs(int sector, char* buffer)
{
  if(g_msCtxInitialized == 0)
    return -1;
  
  //not implemented
  return -2;
}

//this function reads single cluster
//number of clusters and size of sector should be taken from partition table
//however size of sector other than 0x200 is not currently expected

//to change sector size I need to set block size with CMD16 SET_BLOCKLEN
//however currently I do not know how to execute single SD EMMC commands

//I should also mention that not all card types support block len change

int readCluster(int cluster, char* buffer)
{
  if(g_emmcCtxInitialized == 0)
    return -1;
  
  if(g_clusterPoolInitialized == 0)
    return -2;
  
  int res_1 = ksceSdifReadSectorAsync(g_emmcCtx, g_sectorsPerCluster * cluster, g_clusterPoolPtr, g_sectorsPerCluster);
  if(res_1 < 0)
    return res_1;
  
  int res_2 = ksceKernelMemcpyKernelToUser((uintptr_t)buffer, g_clusterPoolPtr, g_bytesPerSector * g_sectorsPerCluster);
  if(res_2 < 0)
    return res_2;

  return 0;
}

int readClusterGc(int cluster, char* buffer)
{
  if(g_gcCtxInitialized == 0)
    return -1;
  
  if(g_clusterPoolInitialized == 0)
    return -2;
  
  int res_1 = ksceSdifReadSectorAsync(g_gcCtx, g_sectorsPerCluster * cluster, g_clusterPoolPtr, g_sectorsPerCluster);
  if(res_1 < 0)
    return res_1;
  
  int res_2 = ksceKernelMemcpyKernelToUser((uintptr_t)buffer, g_clusterPoolPtr, g_bytesPerSector * g_sectorsPerCluster);
  if(res_2 < 0)
    return res_2;

  return 0;
}

int readClusterMs(int cluster, char* buffer)
{
  if(g_msCtxInitialized == 0)
    return -1;
  
  if(g_clusterPoolInitialized == 0)
    return -2;
  
  int res_1 = ksceMsifReadSector(g_sectorsPerCluster * cluster, g_clusterPoolPtr, g_sectorsPerCluster);
  if(res_1 < 0)
    return res_1;
  
  int res_2 = ksceKernelMemcpyKernelToUser((uintptr_t)buffer, g_clusterPoolPtr, g_bytesPerSector * g_sectorsPerCluster);
  if(res_2 < 0)
    return res_2;

  return 0;
}

//this function writes single cluster
//number of clusters and size of sector should be taken from partition table
//however size of sector other than 0x200 is not currently expected

//to change sector size I need to set block size with CMD16 SET_BLOCKLEN
//however currently I do not know how to execute single SD EMMC commands

//I should also mention that not all card types support block len change

int writeCluster(int cluster, char* buffer)
{
  if(g_emmcCtxInitialized == 0)
    return -1;
  
  if(g_clusterPoolInitialized == 0)
    return -2;
  
  //not implemented
  return -3;
}

int writeClusterGc(int cluster, char* buffer)
{
  if(g_gcCtxInitialized == 0)
    return -1;
  
  if(g_clusterPoolInitialized == 0)
    return -2;
  
  //not implemented
  return -3;
}

int writeClusterMs(int cluster, char* buffer)
{
  if(g_msCtxInitialized == 0)
    return -1;
  
  if(g_clusterPoolInitialized == 0)
    return -2;
  
  //not implemented
  return -3;
}

//=================================================

int initialize_emmc_globals()
{
  g_emmcCtx = ksceSdifGetSdContextPartMmc(SCE_SDIF_DEV_EMMC);
  if(g_emmcCtx == 0)
  {
    if(ksceSdifInitializeSdContextPartMmc(SCE_SDIF_DEV_EMMC, &g_emmcCtx) < 0)
    {
      g_emmcCtxInitialized = 0;
      return -1;
    }
    else
    {
      g_emmcCtxInitialized = 1;
      return 0;
    }
  }
  else
  {
    g_emmcCtxInitialized = 1;
    return 0;
  }
}

int initialize_gc_mmc_device()
{
  g_gcCtx = ksceSdifGetSdContextPartMmc(SCE_SDIF_DEV_GAME_CARD);
  if(g_gcCtx == 0)
  {
    if(ksceSdifInitializeSdContextPartMmc(SCE_SDIF_DEV_GAME_CARD, &g_gcCtx) < 0)
    {
      g_gcCtxInitialized = 0;
      return -1;
    }
    else
    {
      g_gcCtxInitialized = 1;
      return 0;
    }
  }
  else
  {
    g_gcCtxInitialized = 1;
    return 0;
  }
}

int initialize_gc_sd_device()
{
  g_gcCtx = ksceSdifGetSdContextPartSd(SCE_SDIF_DEV_GAME_CARD);
  if(g_gcCtx == 0)
  {
    //currently I do not know how initialization should be triggered for sd device
    
    g_gcCtxInitialized = 0;
    return -1;
  }
  else
  {
    g_gcCtxInitialized = 1;
    return 0;
  }
}

int initialize_gc_globals()
{
  //to be able to switch between sd and mmc cards - we need to aquire type of device from global context
  //before querying context part
  //cartridge must be plugged before henkaku loads this driver
  
  sd_context_global* gctx = ksceSdifGetSdContextGlobal(SCE_SDIF_DEV_GAME_CARD);
  if(gctx->ctx_data.dev_type_idx == 1) // mmc type
    return initialize_gc_mmc_device();
  else if(gctx->ctx_data.dev_type_idx == 2) // sd type
    return initialize_gc_sd_device();
  else
    return -1;
}

int initialize_ms_globals()
{
  //currently I do not know how to initialize ms
  //there are at least two functions that must be called
  
  g_msCtxInitialized = 1;
  return 0;
}

tai_hook_ref_t gc_hook_ref;
SceUID gc_hook_id = -1; //hook of CMD56 init routine in SblGcAuthMgr

tai_hook_ref_t init_mmc_hook_ref;
SceUID init_mmc_hook_id = -1; //hook of mmc init function in Sdif

tai_hook_ref_t init_sd_hook_ref;
SceUID init_sd_hook_id = -1; // hook of sd init function in Sdif

SceUID patch_uids[3]; //these are used to patch number of iterations for CMD55, ACMD41 in Sdif

tai_hook_ref_t gen_init_hook_refs[3];
SceUID gen_init_hook_uids[3]; //these are used to hook generic init functions in SdStor

tai_hook_ref_t load_mbr_hook_ref;
SceUID load_mbr_hook_id = -1;

tai_hook_ref_t  mnt_pnt_chk_hook_ref;
SceUID mnt_pnt_chk_hook_id = -1;

tai_hook_ref_t mbr_table_init_hook_ref;
SceUID mbr_table_init_hook_id = -1;

tai_hook_ref_t cmd55_41_hook_ref; //hook of CMD55, ACMD41 preinit function in Sdif
SceUID cmd55_41_hook_id = -1;

tai_hook_ref_t sysroot_zero_hook_ref;
SceUID sysroot_zero_hook_id = -1;

tai_hook_ref_t sdstor_dev_fs_refs[13];
SceUID sdstor_dev_fs_ids[13] = {-1};

//==================================

#define MOD_LIST_SIZE 0x80
#define SEG_LIST_SIZE MOD_LIST_SIZE * 4
SceUID g_modlist[MOD_LIST_SIZE];

int moduleListIsConstructed = 0;

typedef struct range_pair
{
  uintptr_t start;
  uintptr_t end;
}range_pair;

typedef struct segment_info
{
  char moduleName[30];
  int seg;
  range_pair range;
}segment_info;

segment_info g_segList[SEG_LIST_SIZE];

int construct_module_range_table()
{
  if(moduleListIsConstructed > 0)
    return 0;

  moduleListIsConstructed = 1;

  open_global_log();
  FILE_WRITE(global_log_fd, "constructing module range table\n");
  close_global_log();

  size_t count = MOD_LIST_SIZE;

  memset(g_modlist, 0, sizeof(g_modlist));

  memset(g_segList, -1, sizeof(g_segList));
  
  int ret = ksceKernelGetModuleList(KERNEL_PID, 0x80000001, 1, g_modlist, &count);
  if(ret < 0)
    return ret;

  open_global_log();
  char buffer[100];
  snprintf(buffer, 100, "number of modules: %d\n", count);
  FILE_WRITE_LEN(global_log_fd, buffer);
  close_global_log();

  int segInfoIndex = 0;

  for (int m = 0; m < count; m++) 
  {
    SceKernelModuleInfo minfo;
    minfo.size = sizeof(SceKernelModuleInfo);
    ret = ksceKernelGetModuleInfo(KERNEL_PID, g_modlist[m], &minfo);
    if(ret < 0)
      return ret;

    for (int s = 0; s < 4; ++s) 
    {
      if (minfo.segments[s].vaddr == NULL) 
        continue;
      
      memset(g_segList[segInfoIndex].moduleName, 0, 30);
      memcpy(g_segList[segInfoIndex].moduleName, minfo.module_name, 28);

      g_segList[segInfoIndex].seg = s;

      g_segList[segInfoIndex].range.start = (uintptr_t)minfo.segments[s].vaddr;
      g_segList[segInfoIndex].range.end = (uintptr_t)minfo.segments[s].vaddr + minfo.segments[s].memsz;

      segInfoIndex++;
    }
  }

  //set final item
  g_segList[segInfoIndex].range.start = -1;
  g_segList[segInfoIndex].range.end = -1;

  return 0;
}

int compare_segments(const void *p, const void *q) 
{
    const segment_info* x = (const segment_info*)p;
    const segment_info* y = (const segment_info*)q;
    if (x->range.start < y->range.start)
        return -1;  // Return -1 if you want ascending, 1 if you want descending order. 
    else if (x->range.start > y->range.start)
        return 1;   // Return 1 if you want ascending, -1 if you want descending order. 

    return 0;
}

int sort_segment_table()
{
  if(moduleListIsConstructed == 0)
    return -1;

  qsort_imp(g_segList, SEG_LIST_SIZE, sizeof(segment_info), compare_segments);

  return 0; 
}

int find_in_segments(uintptr_t item)
{
  if(moduleListIsConstructed == 0)
      return -1;

  for (int s = 0; s < SEG_LIST_SIZE; s++) 
  {
    if(item >= g_segList[s].range.start && item < g_segList[s].range.end)
      return s;
  }

  return -1;  
}

int print_segment_table()
{
  if(moduleListIsConstructed == 0)
      return -1;

  for (int s = 0; s < SEG_LIST_SIZE; s++) 
  {
    if(g_segList[s].range.start == -1 || g_segList[s].range.end == -1)
      break;

    open_global_log();
    char buffer[100];
    snprintf(buffer, 100, "%s %d %08x %08x\n", g_segList[s].moduleName, g_segList[s].seg, g_segList[s].range.start, g_segList[s].range.end);
    FILE_WRITE_LEN(global_log_fd, buffer);
    close_global_log();
  }

  return 0; 
}

//==================================


#pragma pack(push, 1)

typedef struct device_init_info
{
  int sd_ctx_index;
  sd_context_part* ctx;
}device_init_info;

#pragma pack(pop)

#define DEVICE_INFO_SIZE 4

int last_mmc_index = 0;
int last_sd_index = 0;

device_init_info last_mmc_inits[DEVICE_INFO_SIZE];
device_init_info last_sd_inits[DEVICE_INFO_SIZE];

int clear_device_info_arrays()
{
  memset(last_mmc_inits, -1, sizeof(device_init_info) * DEVICE_INFO_SIZE);
  memset(last_sd_inits, -1, sizeof(device_init_info) * DEVICE_INFO_SIZE);
  return 0;
}

int print_device_info_arrays()
{
  char buffer[100];
  
  open_global_log();
  FILE_WRITE(global_log_fd, "------ mmc -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(buffer, 100, "idx:%x ctx:%x\n", last_mmc_inits[i].sd_ctx_index, last_mmc_inits[i].ctx);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  FILE_WRITE(global_log_fd, "------ sd  -------\n");
  for(int i = 0; i < DEVICE_INFO_SIZE; i++)
  {  
    snprintf(buffer, 100, "idx:%x ctx:%x\n", last_sd_inits[i].sd_ctx_index, last_sd_inits[i].ctx);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return 0;
}

//-----------

int gc_sd_init(void* args)
{
   sd_context_part* ctx_00BDCBC0 = ksceSdifGetSdContextPartSd(SCE_SDIF_DEV_GAME_CARD);
   if(ctx_00BDCBC0 == 0)
   {
      int res = ksceSdifInitializeSdContextPartSd(SCE_SDIF_DEV_GAME_CARD, &ctx_00BDCBC0);
      if(res != 0)
         return 0x808A0703;
   }
   return 0;
}

int gc_patch(int param0)
{
  /*
  int var_10 = param0;
  return ksceKernelRunWithStack(0x2000, &gc_sd_init, &var_10);
  */
  
  int res = TAI_CONTINUE(int, gc_hook_ref, param0);
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "call gc auth res:%x\n", res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return res;
}

int init_mmc_hook(int sd_ctx_index, sd_context_part** result)
{
  int res = TAI_CONTINUE(int, init_mmc_hook_ref, sd_ctx_index, result);
  
  /*
  int res = 0;
  
  //forward game card initialization to anoher function
  //other initializations should be fowarded to standard function
  
  if(sd_ctx_index == SCE_SDIF_DEV_GAME_CARD)
  {
    res = ksceSdifInitializeSdContextPartSd(sd_ctx_index, result);
  }
  else
  {
    res = TAI_CONTINUE(int, init_mmc_hook_ref, sd_ctx_index, result);
  }
  */
  
  last_mmc_inits[last_mmc_index].sd_ctx_index = sd_ctx_index;
  if(result != 0)
    last_mmc_inits[last_mmc_index].ctx = *result;
  else
    last_mmc_inits[last_mmc_index].ctx = (sd_context_part*)-1;
  
  last_mmc_index++;
  if(last_mmc_index == DEVICE_INFO_SIZE)
    last_mmc_index = 0;
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "init mmc - idx:%x ctx:%x res:%x\n", sd_ctx_index, *result, res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();

  return res;
}

int init_sd_hook(int sd_ctx_index, sd_context_part** result)
{
  int res = TAI_CONTINUE(int, init_sd_hook_ref, sd_ctx_index, result);
  
  last_sd_inits[last_sd_index].sd_ctx_index = sd_ctx_index;
  if(result != 0)
    last_sd_inits[last_sd_index].ctx = *result;
  else
    last_sd_inits[last_sd_index].ctx = (sd_context_part*)-1;
  
  last_sd_index++;
  if(last_sd_index == DEVICE_INFO_SIZE)
    last_sd_index = 0;
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "init sd - idx:%x ctx:%x res:%x\n", sd_ctx_index, *result, res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  //initialize_gc_globals(); //initialize all globals here since it can not be done on boot
  
  return res;
}

int cmd55_41_hook(sd_context_global* ctx)
{
  int res = TAI_CONTINUE(int, cmd55_41_hook_ref, ctx);
  
  /*
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "res cmd55_41:%x\n", res);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  */
  
  return res;
}

int gen_init_hook_1(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[0], ctx);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called gen_init_hook_1\n");
  }
  close_global_log();
  
  return res;
}

int gen_init_hook_2(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[1], ctx);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called gen_init_hook_2\n");
  }
  close_global_log();
  
  return res;
}

int gen_init_hook_3(void* ctx)
{
  int res = TAI_CONTINUE(int, gen_init_hook_refs[2], ctx);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called gen_init_hook_3\n");
  }
  close_global_log();
  
  return res;
}

int sysroot_zero_hook()
{
  int res = TAI_CONTINUE(int, sysroot_zero_hook_ref);
  
  open_global_log();
  {
    FILE_WRITE(global_log_fd, "called sysroot_zero_hook\n");
  }
  close_global_log();
  
  return res;
  
  //returning 1 here enables sd init
  //however it breaks existing functionality, including:
  //insertion detection of the card - looks like initilization of card is started upon insertion, however no "please wait" dialog is shown and card is not detected
  //upon suspend and then resume - causes hang of the whole system. touch does not respond, unable to power off, have to take out baterry
  
  return 1; //return 1 instead of hardcoded 0
}

int load_mbr_hook(int ctx_index)
{
  int res = TAI_CONTINUE(int, load_mbr_hook_ref, ctx_index);
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "called load_mbr_hook: %x\n", ctx_index);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return res;
}

int mnt_pnt_chk_hook(char* blockDeviceName, int mountNum, int* mountData)
{
  int res = TAI_CONTINUE(int, mnt_pnt_chk_hook_ref, blockDeviceName, mountNum, mountData);

  open_global_log();
  {
    if(blockDeviceName == 0 || mountData == 0)
    {
      FILE_WRITE(global_log_fd, "called mnt_pnt_chk_hook: data is invalid\n");
    }
    else
    {
      char buffer[200];
      snprintf(buffer, 200, "called mnt_pnt_chk_hook: %s %08x %08x %08x\n", blockDeviceName, mountNum, *mountData, res);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
  }
  close_global_log();

  return res;
}

int mbr_table_init_hook(char* blockDeviceName, int mountNum)
{
  int res = TAI_CONTINUE(int, mbr_table_init_hook_ref, blockDeviceName, mountNum);

  open_global_log();
  {
    if(blockDeviceName == 0)
    {
      FILE_WRITE(global_log_fd, "called mbr_table_init_hook: data is invalid\n");
    }
    else
    {
      char buffer[200];
      snprintf(buffer, 200, "called mbr_table_init_hook: %s %08x %08x\n", blockDeviceName, mountNum, res);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
  }
  close_global_log();

  return res;
}

//=======================================

#define ENABLE_LOG_VFS_SD_NODE

SceUID sdstor_dev_fs_log_fd;

void open_sdstor_dev_fs_log()
{
  sdstor_dev_fs_log_fd = ksceIoOpen("ux0:dump/sdstor_dev_fs.txt", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);  
}

void close_sdstor_dev_fs_log()
{
  if(sdstor_dev_fs_log_fd >= 0)
    ksceIoClose(sdstor_dev_fs_log_fd);
}

#define SIZE_OF_STACKTRACE 100

int stacktrace_from_here(char* moduleNameSearch, int segIndexSearch)
{
  //must be specified volatile or optimizer will do what it likes

  //I use this variables just for marking to see that stack data that I get is adequate
  volatile int mark0 = 0xA0A0A0A0;
  volatile int mark1 = 0x05050505;
  volatile int mark2 = 0x37373737;

  //------------------------
  //unless modules are reloaded, which is most likely not happening, we can do it once during this module load
  //construct_module_range_table();
  //sort_segment_table();
  //print_segment_table();

  volatile int* stackPtr = &mark0;

  for(int i = 0; i < SIZE_OF_STACKTRACE; i++)
  {
    int curValue = *stackPtr;
    int segidx = find_in_segments(curValue);
    if(segidx >= 0)
    {
      if(g_segList[segidx].seg == segIndexSearch)
      {
        if(strcmp(moduleNameSearch, g_segList[segidx].moduleName) == 0)
        {
          open_sdstor_dev_fs_log();
          {
            char buffer[100];
            snprintf(buffer, 100, "%08x %s %d %08x %08x\n", curValue, g_segList[segidx].moduleName, g_segList[segidx].seg, g_segList[segidx].range.start, (curValue - g_segList[segidx].range.start));
            FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
          }
          close_sdstor_dev_fs_log();
        }
      }      
    }

    stackPtr++;
  }

  return 0;
}

int vfs_func1_entered = 0;

int vfs_func1(void* ctx) //00C17015
{
  if(vfs_func1_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[0], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func1_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func1: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func1_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

int vfs_func3_entered = 0;

int vfs_func3(void* ctx) //00C1703D
{
  if(vfs_func3_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[1], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func3_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func3: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func3_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

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

int vfs_func12_entered = 0;

int vfs_func12(void* ctx) //00C175D1
{
  if(vfs_func12_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[2], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func12_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func12: %x %s\n", ctx, ((ctx_C175D0*)ctx)->blockDeviceName);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func12_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

typedef struct ctx_C17550
{
   int unk_0;
   char* blockDeviceName;
   int unk_8;
   int unk_C;
   
   char* dest;
   int len;
   int unk_18;
   
}ctx_C17550;

int vfs_func13_entered = 0;

int vfs_func13(void* ctx) //00C17551
{
  if(vfs_func13_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[3], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_func13_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_func13: %x %s\n", ctx, ((ctx_C17550*)ctx)->blockDeviceName);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_func13_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

int vfs_node_func1_entered = 0;

int vfs_node_func1(void* ctx) //00C17465
{
  if(vfs_node_func1_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[4], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func1_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func1: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func1_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

int vfs_node_func3_entered = 0;

int vfs_node_func3(void* ctx) //00C17459
{
  if(vfs_node_func3_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[5], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func3_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func3: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func3_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

typedef struct pair_C172E0
{
   char* blockDeviceName;
   int nameLength;
} pair_C172E0;

typedef struct data_C172E0
{
   int unk_0;
   int unk_4;
   int unk_8;
   int unk_C;
   
   int unk_10;
   int unk_14;
   int unk_18;
   int unk_1C;
   
   int unk_20;
   int unk_24;
   int unk_28;
   int unk_2C;
   
   int unk_30;
   int unk_34;
   int unk_38;
   int unk_3C;
   
   int unk_40;
   int unk_44;
   int unk_48;
   int unk_4C;
   
   int unk_50;
   int unk_54;
   int unk_58;
   int unk_5C;
   
   int unk_60;
   int unk_64;
   int unk_68;
   SceUID pool_uid;
}data_C172E0;

typedef struct ctx_C172E0
{
   data_C172E0* data;
   int* result;
   pair_C172E0* mount;
}ctx_C172E0;

int vfs_node_func4_entered = 0;

int vfs_node_func4(void* ctx) //00C172E1
{
  if(vfs_node_func4_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[6], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func4_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func4: %x %s\n", ctx, ((ctx_C172E0*)ctx)->mount->blockDeviceName);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func4_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

//int vfs_node_func5_entered = 0;

int vfs_node_func5(void* ctx) //00C17231 - HOOK DOES NOT WORK when writing to file is done
{
  //if(vfs_node_func5_entered == 1)
  //  return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[7], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  //vfs_node_func5_entered = 1;
  /*
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func5: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  */
  //vfs_node_func5_entered = 0;
  #endif

  return res;
}

//int vfs_node_func6_entered = 0;

int vfs_node_func6(void* ctx) //00C1717D - HOOK DOES NOT WORK when writing to file is done
{
  //if(vfs_node_func6_entered == 1)
  //  return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[8], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  //vfs_node_func6_entered = 1;
  /*
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func6: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  */
  //vfs_node_func6_entered = 0;
  #endif

  return res;
}

int vfs_node_func7_entered = 0;

int vfs_node_func7(void* ctx) //00C170C5
{
  if(vfs_node_func7_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[9], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func7_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func7: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func7_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

int vfs_node_func9_entered = 0;

int vfs_node_func9(void* ctx) //00C17291
{
  if(vfs_node_func9_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[10], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func9_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func9: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func9_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

int vfs_node_func19_entered = 0;

int vfs_node_func19(void* ctx) //00C171DD
{
  if(vfs_node_func19_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[11], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func19_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func19: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func19_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

int vfs_node_func20_entered = 0;

int vfs_node_func20(void* ctx) //00C17071
{
  if(vfs_node_func20_entered == 1)
    return -1;

  int res = TAI_CONTINUE(int, sdstor_dev_fs_refs[12], ctx);

  #ifdef ENABLE_LOG_VFS_SD_NODE
  vfs_node_func20_entered = 1;
  open_sdstor_dev_fs_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "vfs_node_func20: %x\n", ctx);
    FILE_WRITE_LEN(sdstor_dev_fs_log_fd, buffer);
  }
  close_sdstor_dev_fs_log();
  vfs_node_func20_entered = 0;
  #endif

  stacktrace_from_here("SceIofilemgr", 0);

  return res;
}

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
  }
  
  tai_module_info_t sdif_info;
  sdif_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdif", &sdif_info) >= 0)
  {
    init_mmc_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &init_mmc_hook_ref, "SceSdif", SceSdifForDriver_NID, 0x22c82e79, init_mmc_hook);
    init_sd_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &init_sd_hook_ref, "SceSdif", SceSdifForDriver_NID, 0xc1271539, init_sd_hook);
    
    char iterations[1] = {20};
      
    patch_uids[0] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x349A, iterations, 1); //patch MOVS R2, 5 to MOVS R2, 10
    patch_uids[1] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x34B8, iterations, 1); //patch MOVS R2, 5 to MOVS R2, 10
    patch_uids[2] = taiInjectDataForKernel(KERNEL_PID, sdif_info.modid, 0, 0x359A, iterations, 1); //patch MOVS R2, 5 to MOVS R2, 10
    
    cmd55_41_hook_id = taiHookFunctionOffsetForKernel(KERNEL_PID, &cmd55_41_hook_ref, sdif_info.modid, 0, 0x35E8, 1, cmd55_41_hook);
  }
  
  //SceSysmem.SceSysrootForDriver._exp_f804f761
  
  tai_module_info_t sysroot_info;
  sysroot_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSysmem", &sysroot_info) >= 0)
  {
    //sysroot_zero_hook_id = taiHookFunctionExportForKernel(KERNEL_PID, &sysroot_zero_hook_ref, "SceSysmem", SceSysrootForDriver_NID, 0xf804f761, sysroot_zero_hook);
    
    //by some reason only import hook worked
    sysroot_zero_hook_id = taiHookFunctionImportForKernel(KERNEL_PID, &sysroot_zero_hook_ref, "SceSdstor", SceSysrootForDriver_NID, 0xf804f761, sysroot_zero_hook);
  }
  
  open_global_log();
  if(gc_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set gc init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gc init hook: %x\n", gc_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(init_mmc_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mmc init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set mmc init hook: %x\n", init_mmc_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(init_sd_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sd init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sd init hook: %x\n", init_sd_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(cmd55_41_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set cmd55_41_hook_id hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set cmd55_41_hook_id hook: %x\n", cmd55_41_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(gen_init_hook_uids[0] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 1 hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gen init 1 hook: %x\n", gen_init_hook_uids[0]);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(gen_init_hook_uids[1] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 2 hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gen init 2 hook: %x\n", gen_init_hook_uids[1]);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(gen_init_hook_uids[2] >= 0)
  {
    FILE_WRITE(global_log_fd, "set gen init 3 hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set gen init 3 hook: %x\n", gen_init_hook_uids[2]);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(sysroot_zero_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sysroot zero hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set sysroot zero hook: %x\n", sysroot_zero_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  
  if(load_mbr_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set load mbr hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set load mbr hook: %x\n", load_mbr_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  if(mnt_pnt_chk_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mnt ptr chk hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set mnt ptr chk hook: %x\n", mnt_pnt_chk_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  if(mbr_table_init_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set mbr table init hook\n");
  }
  else
  {
    char buffer[100];
    snprintf(buffer, 100, "failed to set mbr table init hook: %x\n", mbr_table_init_hook_id);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }

  for(int f = 0; f < 13; f++)
  {
    if(sdstor_dev_fs_ids[f] >= 0)
    {
      char buffer[100];
      snprintf(buffer, 100, "set sdstor_dev_fs function %d hook\n", (f + 1));
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
    else
    {
      char buffer[100];
      snprintf(buffer, 100, "failed to set sdstor_dev_fs function %d hook: %x\n", (f + 1), sdstor_dev_fs_ids[f]);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
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
  
  return 0;
}

int print_ctx(int sd_index)
{
  sd_context_global* gctx = ksceSdifGetSdContextGlobal(sd_index);
  uint32_t idx = gctx->ctx_data.dev_type_idx;
  sd_context_part* pctx = gctx->ctx_data.ctx;
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "type:%x ctx:%x\n", idx, pctx);
    FILE_WRITE_LEN(global_log_fd, buffer);
  }
  close_global_log();
  
  return 0;
}

int initialize_gc_sd()
{
  sd_context_part* ctx_00BDCBC0 = ksceSdifGetSdContextPartSd(SCE_SDIF_DEV_GAME_CARD);
  if(ctx_00BDCBC0 == 0)
  {
    int res = ksceSdifInitializeSdContextPartSd(SCE_SDIF_DEV_GAME_CARD, &ctx_00BDCBC0);
    
    open_global_log();
    {
      char buffer[100];
      snprintf(buffer, 100, "res:%x \n", res);
      FILE_WRITE_LEN(global_log_fd, buffer);
    }
    close_global_log();
  }
  
  //print_ctx(SCE_SDIF_DEV_EMMC);
  print_ctx(SCE_SDIF_DEV_GAME_CARD);
  //print_ctx(SCE_SDIF_DEV_WLAN_BT);
  
  return 0;
}

int dump_memblock_1000(SceUID uid, void* membase)
{
  SceUID file = ksceIoOpen("ux0:dump/psvemmc_memblock_1000.bin", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);  

  if(file >= 0)
  {
    ksceIoWrite(file, membase, 0x1000);
    ksceIoClose(file);
  }
  
  return 0;
}

int validate_dump_memblock_1000(int index, SceUID uid, void* membase)
{
  if(membase > 0)
  {
    void* membaseCheck = 0;
    int res_1 = ksceKernelGetMemBlockBase(uid, &membaseCheck);
  
    if(res_1 == 0)
    {
      if(membase == membaseCheck)
      {
        open_global_log();
        char buffer[100];
        snprintf(buffer, 100, "device:%x membase 1000 OK\n", index);
        FILE_WRITE_LEN(global_log_fd, buffer);
        close_global_log();
        
        dump_memblock_1000(uid, membase);
        
        return 0;
      }
      else
      {
        open_global_log();
        char buffer[100];
        snprintf(buffer, 100, "device:%x membase 1000 does not match\n", index);
        FILE_WRITE_LEN(global_log_fd, buffer);
        close_global_log();
        return -1;
      }
    }
    else
    {
      open_global_log();
      char buffer[100];
      snprintf(buffer, 100, "device:%x failed to get membase 1000, error:%x\n", index, res_1);
      FILE_WRITE_LEN(global_log_fd, buffer);
      close_global_log();
      return -1;
    }
  }
  else
  {
    open_global_log();
    char buffer[100];
    snprintf(buffer, 100, "device:%x membase 1000 is empty\n", index);
    FILE_WRITE_LEN(global_log_fd, buffer);
    close_global_log();
    return -1;
  }
}

int dump_device_context_mem_blocks_1000()
{
  sd_context_global* gc0 = ksceSdifGetSdContextGlobal(0);
  sd_context_global* gc1 = ksceSdifGetSdContextGlobal(1);
  sd_context_global* gc2 = ksceSdifGetSdContextGlobal(2);
  
  sd_context_data* ctxd0 = &gc0->ctx_data;
  sd_context_data* ctxd1 = &gc1->ctx_data;
  sd_context_data* ctxd2 = &gc2->ctx_data;
  
  validate_dump_memblock_1000(ctxd0->array_idx, ctxd0->uid_1000, ctxd0->membase_1000);
  validate_dump_memblock_1000(ctxd1->array_idx, ctxd1->uid_1000, ctxd1->membase_1000);
  validate_dump_memblock_1000(ctxd2->array_idx, ctxd2->uid_1000, ctxd2->membase_1000);
  
  return 0;
}

int dump_memblock_10000(SceUID uid, void* membase)
{
  SceUID file = ksceIoOpen("ux0:dump/psvemmc_memblock_10000.bin", SCE_O_CREAT | SCE_O_APPEND | SCE_O_WRONLY, 0777);  

  if(file >= 0)
  {
    ksceIoWrite(file, membase, 0x10000);
    ksceIoClose(file);
  }
  
  return 0;
}

int validate_dump_memblock_10000(int index, SceUID uid, void* membase)
{
  if(membase > 0)
  {
    void* membaseCheck = 0;
    int res_1 = ksceKernelGetMemBlockBase(uid, &membaseCheck);
  
    if(res_1 == 0)
    {
      if(membase == membaseCheck)
      {
        open_global_log();
        char buffer[100];
        snprintf(buffer, 100, "device:%x membase 10000 OK\n", index);
        FILE_WRITE_LEN(global_log_fd, buffer);
        close_global_log();
        
        dump_memblock_10000(uid, membase);
        
        return 0;
      }
      else
      {
        open_global_log();
        char buffer[100];
        snprintf(buffer, 100, "device:%x membase 10000 does not match\n", index);
        FILE_WRITE_LEN(global_log_fd, buffer);
        close_global_log();
        return -1;
      }
    }
    else
    {
      open_global_log();
      char buffer[100];
      snprintf(buffer, 100, "device:%x failed to get membase 10000, error:%x\n", index, res_1);
      FILE_WRITE_LEN(global_log_fd, buffer);
      close_global_log();
      return -1;
    }
  }
  else
  {
    open_global_log();
    char buffer[100];
    snprintf(buffer, 100, "device:%x membase 10000 is empty\n", index);
    FILE_WRITE_LEN(global_log_fd, buffer);
    close_global_log();
    return -1;
  }
}

int dump_device_contex_mem_blocks_10000()
{
  sd_context_global* gc0 = ksceSdifGetSdContextGlobal(0);
  sd_context_global* gc1 = ksceSdifGetSdContextGlobal(1);
  sd_context_global* gc2 = ksceSdifGetSdContextGlobal(2);
  
  sd_context_data* ctxd0 = &gc0->ctx_data;
  sd_context_data* ctxd1 = &gc1->ctx_data;
  sd_context_data* ctxd2 = &gc2->ctx_data;
  
  validate_dump_memblock_10000(ctxd0->array_idx, ctxd0->uid_10000, ctxd0->membase_10000);
  validate_dump_memblock_10000(ctxd1->array_idx, ctxd1->uid_10000, ctxd1->membase_10000);
  validate_dump_memblock_10000(ctxd2->array_idx, ctxd2->uid_10000, ctxd2->membase_10000);
  
  return 0;
}

int dumpSegment(SceKernelModuleInfo* minfo, int index)
{
  if(minfo->segments <= 0)
    return -1;
  
  if (minfo->segments[index].vaddr <= 0) 
  {
    open_global_log();
    FILE_WRITE(global_log_fd, "segment is empty\n");
    close_global_log();
    return -1;
  }
  
  {
    open_global_log();
    char buffer[100];
    snprintf(buffer, 100, "%d %x %x\n", index, minfo->segments[index].vaddr, minfo->segments[index].memsz);
    FILE_WRITE_LEN(global_log_fd, buffer);
    close_global_log();
  }
  
  char filename[100] = {0};
  char moduleNameCopy[30] = {0};
  snprintf(moduleNameCopy, 30, minfo->module_name);
  snprintf(filename, 100, "ux0:dump/0x%08x_%s_%d.bin", (unsigned)minfo->segments[index].vaddr, moduleNameCopy, index);

  {
    open_global_log();
    char buffer[100];
    snprintf(buffer, 100, "%s\n", filename);
    FILE_WRITE_LEN(global_log_fd, buffer);
    close_global_log();
  }
  
  SceUID fout = ksceIoOpen(filename, SCE_O_CREAT | SCE_O_TRUNC | SCE_O_WRONLY, 0777);
  
  if(fout < 0)
     return -1;
  
  if(minfo->segments[index].memsz > 0)
  {
    ksceIoWrite(fout, minfo->segments[index].vaddr, minfo->segments[index].memsz);
  }
  
  ksceIoClose(fout);
  
  return 0;
}

int dump_sdstor_data()
{
  tai_module_info_t sdstor_info;
  sdstor_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &sdstor_info) >= 0) 
  {
    SceKernelModuleInfo minfo;
    minfo.size = sizeof(SceKernelModuleInfo);
    int ret = ksceKernelGetModuleInfo(KERNEL_PID, sdstor_info.modid, &minfo);
    if(ret >= 0)
    {
      open_global_log();
      FILE_WRITE(global_log_fd, "ready to dump sdstor data seg\n");
      close_global_log();
      
      dumpSegment(&minfo, 1);
    }
    else
    {
      open_global_log();
      FILE_WRITE(global_log_fd, "can not dump sdstor data seg\n");
      close_global_log();
    }
  }
  
  return 0;
}

int dump_sdif_data()
{
  tai_module_info_t sdif_info;
  sdif_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdif", &sdif_info) >= 0)
  {
    SceKernelModuleInfo minfo;
    minfo.size = sizeof(SceKernelModuleInfo);
    int ret = ksceKernelGetModuleInfo(KERNEL_PID, sdif_info.modid, &minfo);
    if(ret >= 0)
    {
      open_global_log();
      FILE_WRITE(global_log_fd, "ready to dump sdif data seg\n");
      close_global_log();
      
      dumpSegment(&minfo, 1);
    }
    else
    {
      open_global_log();
      FILE_WRITE(global_log_fd, "can not dump sdif data seg\n");
      close_global_log();
    }
  }
  
  return 0;
}

int module_start(SceSize argc, const void *args) 
{
  //initialize emmc if required
  initialize_emmc_globals();
  
  //initialize gc if required
  initialize_gc_globals();
  
  //initialize ms if required
  initialize_ms_globals();
  
  initialize_all_hooks();

  //deal with module table
  construct_module_range_table();
  sort_segment_table();
  print_segment_table();
  
  //dump_device_context_mem_blocks_1000();
  
  //dump_device_contex_mem_blocks_10000();
  
  //dump_sdstor_data();
  //dump_sdif_data();
  
  //-------------------------------
  
  /*
  print_device_info_arrays();
  
  clear_device_info_arrays();
  
  initialize_gc_sd();
  
  print_device_info_arrays();
  */
  
  return SCE_KERNEL_START_SUCCESS;
}
 
//Alias to inhibit compiler warning
void _start() __attribute__ ((weak, alias ("module_start")));
 
int module_stop(SceSize argc, const void *args) 
{
  deinitialize_all_hooks();
  
  //deinitialize buffers if required
  psvemmcDeinitialize();
  
  return SCE_KERNEL_STOP_SUCCESS;
}
