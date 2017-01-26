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
    
    uint32_t sd_index;
    struct sd_context_part* ctx;
    uint32_t unk_18;
    uint32_t unk_1C;
       
    uint8_t data[0xA0];
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

sd_context_part* ksceSdifGetSdContextPartEmmc(int sd_ctx_idx);
sd_context_part* ksceSdifGetSdContextPartGameCard(int sd_ctx_idx);
sd_context_part* ksceSdifGetSdContextPartWlanBt(int sd_ctx_idx);

int ksceSdifGetCardInsertState1(int sd_ctx_idx);
int ksceSdifGetCardInsertState2(int sd_ctx_idx);

int ksceSdifInitializeSdContextPart(int sd_ctx_index, sd_context_part** result);

//uses CMD17 for single sector and CMD23, CMD24 for multiple sectors
int ksceSdifReadSectorAsync(sd_context_part* ctx, int sector, char* buffer, int nSectors);
int ksceSdifReadSector(sd_context_part* ctx, int sector, char* buffer, int nSectors);

//uses CMD18 for single sector and CMD23, CMD25 for multiple sectors
int ksceSdifWriteSectorAsync(sd_context_part* ctx, int sector, char* buffer, int nSectors);
int ksceSdifWriteSector(sd_context_part* ctx, int sector, char* buffer, int nSectors);

int ksceSdifCopyCtx(sd_context_part* ctx, output_23a4ef01* unk0);

//=================================================

//defalut size of sector for SD MMC protocol
#define SD_DEFAULT_SECTOR_SIZE 0x200

sd_context_part* g_emmcCtx = 0;

int g_emmcCtxInitialized = 0;

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

//=================================================

int initialize_emmc()
{
  g_emmcCtx = ksceSdifGetSdContextPartEmmc(SCE_SDIF_DEV_EMMC);
  if(g_emmcCtx == 0)
  {
    if(ksceSdifInitializeSdContextPart(SCE_SDIF_DEV_EMMC, &g_emmcCtx) < 0)
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

int module_start(SceSize argc, const void *args) 
{
  //initialize emmc if required
  initialize_emmc();
  
  return SCE_KERNEL_START_SUCCESS;
}
 
//Alias to inhibit compiler warning
void _start() __attribute__ ((weak, alias ("module_start")));
 
int module_stop(SceSize argc, const void *args) 
{
  //deinitialize buffers if required
  psvemmcDeinitialize();
  
  return SCE_KERNEL_STOP_SUCCESS;
}
