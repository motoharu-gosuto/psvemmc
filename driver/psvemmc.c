#include "psvemmc.h"
 
//taihen plugin by yifanlu was used as a reference:
//https://github.com/yifanlu/taiHEN/blob/master/taihen.c
 
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

#include "debug.h"       //hook init
#include "glog.h"
#include "dump.h"        //some dumping functions
#include "thread_test.h" //to try get current thread id
#include "net.h"    //to init net
#include "mtable.h" //to init tables

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

//==============================================

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
  //print_segment_table();
  
  //print_thread_info();

  init_net();
  
  //dump_device_context_mem_blocks_1000();
  
  //dump_device_context_mem_blocks_10000();
  
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
  deinit_net();

  deinitialize_all_hooks();
  
  //deinitialize buffers if required
  psvemmcDeinitialize();
  
  return SCE_KERNEL_STOP_SUCCESS;
}
