#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <module.h>

#include "dump.h"
#include "glog.h"
#include "sector_api.h"

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

int dump_device_context_mem_blocks_10000()
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
