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
#include "vfs_types.h"
#include "mtable.h"
#include "stacktrace.h"
#include "sdstor_types.h"

char sprintfBuffer[256];

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
        snprintf(sprintfBuffer, 256, "device:%x membase 1000 OK\n", index);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        close_global_log();
        
        dump_memblock_1000(uid, membase);
        
        return 0;
      }
      else
      {
        open_global_log();
        snprintf(sprintfBuffer, 256, "device:%x membase 1000 does not match\n", index);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        close_global_log();
        return -1;
      }
    }
    else
    {
      open_global_log();
      snprintf(sprintfBuffer, 256, "device:%x failed to get membase 1000, error:%x\n", index, res_1);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      close_global_log();
      return -1;
    }
  }
  else
  {
    open_global_log();
    snprintf(sprintfBuffer, 256, "device:%x membase 1000 is empty\n", index);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
        snprintf(sprintfBuffer, 256, "device:%x membase 10000 OK\n", index);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        close_global_log();
        
        dump_memblock_10000(uid, membase);
        
        return 0;
      }
      else
      {
        open_global_log();
        snprintf(sprintfBuffer, 256, "device:%x membase 10000 does not match\n", index);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        close_global_log();
        return -1;
      }
    }
    else
    {
      open_global_log();
      snprintf(sprintfBuffer, 256, "device:%x failed to get membase 10000, error:%x\n", index, res_1);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      close_global_log();
      return -1;
    }
  }
  else
  {
    open_global_log();
    snprintf(sprintfBuffer, 256, "device:%x membase 10000 is empty\n", index);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
    FILE_GLOBAL_WRITE_LEN("segment is empty\n");
    close_global_log();
    return -1;
  }
  
  {
    open_global_log();
    snprintf(sprintfBuffer, 256, "%d %x %x\n", index, minfo->segments[index].vaddr, minfo->segments[index].memsz);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    close_global_log();
  }
  
  char filename[100] = {0};
  char moduleNameCopy[30] = {0};
  snprintf(moduleNameCopy, 30, minfo->module_name);
  snprintf(filename, 100, "ux0:dump/0x%08x_%s_%d.bin", (unsigned)minfo->segments[index].vaddr, moduleNameCopy, index);

  {
    open_global_log();
    snprintf(sprintfBuffer, 256, "%s\n", filename);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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

int dump_iofilemgr_data()
{
  tai_module_info_t iofilemgr_info;
  iofilemgr_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceIofilemgr", &iofilemgr_info) >= 0) 
  {
    SceKernelModuleInfo minfo;
    minfo.size = sizeof(SceKernelModuleInfo);
    int ret = ksceKernelGetModuleInfo(KERNEL_PID, iofilemgr_info.modid, &minfo);
    if(ret >= 0)
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("ready to dump iofilemgr data seg\n");
      close_global_log();
      
      dumpSegment(&minfo, 1);
    }
    else
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("can not dump iofilemgr data seg\n");
      close_global_log();
    }
  }
  
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
      FILE_GLOBAL_WRITE_LEN("ready to dump sdstor data seg\n");
      close_global_log();
      
      dumpSegment(&minfo, 1);
    }
    else
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("can not dump sdstor data seg\n");
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
      FILE_GLOBAL_WRITE_LEN("ready to dump sdif data seg\n");
      close_global_log();
      
      dumpSegment(&minfo, 1);
    }
    else
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("can not dump sdif data seg\n");
      close_global_log();
    }
  }
  
  return 0;
} 

int dump_exfatfs_data()
{
  tai_module_info_t exfatfs_info;
  exfatfs_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceExfatfs", &exfatfs_info) >= 0)
  {
    SceKernelModuleInfo minfo;
    minfo.size = sizeof(SceKernelModuleInfo);
    int ret = ksceKernelGetModuleInfo(KERNEL_PID, exfatfs_info.modid, &minfo);
    if(ret >= 0)
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("ready to dump exfatfs data seg\n");
      close_global_log();
      
      dumpSegment(&minfo, 1);
    }
    else
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("can not dump exfatfs data seg\n");
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
    snprintf(sprintfBuffer, 256, "type:%x ctx:%x\n", idx, pctx);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
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
      snprintf(sprintfBuffer, 256, "res:%x \n", res);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    close_global_log();
  }
  
  //print_ctx(SCE_SDIF_DEV_EMMC);
  print_ctx(SCE_SDIF_DEV_GAME_CARD);
  //print_ctx(SCE_SDIF_DEV_WLAN_BT);
  
  return 0;
}

//====================

int print_bytes(char* bytes, int size)
{ 
  open_global_log();
  for(int i = 0; i < size; i++)
  {
    char buffer[4];
    snprintf(buffer, 4, "%02x ", bytes[i]);
    FILE_GLOBAL_WRITE_LEN(buffer);
  }
  FILE_GLOBAL_WRITE_LEN("\n");
  close_global_log();

  return 0;
}

vfs_add_data* get_sdstor_dev_fs_info_base()
{
  tai_module_info_t m_info;
  m_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &m_info) >= 0) 
  {
    uintptr_t addr = 0;
    int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 1, 0x16FC, &addr);
    if(ofstRes == 0)
    {
      vfs_add_data* data = (vfs_add_data*)addr;
      return data;
    }
  }
  return (vfs_add_data*)-1;
}

vfs_add_data* get_sdstor_dummy_ttyp_dev_fs_info_base()
{
  tai_module_info_t m_info;
  m_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceIofilemgr", &m_info) >= 0) 
  {
    uintptr_t addr = 0;
    int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 1, 0x221C, &addr);
    if(ofstRes == 0)
    {
      vfs_add_data* data = (vfs_add_data*)addr;
      return data;
    }
  }
  return (vfs_add_data*)-1;
}

int dump_vfs_data()
{
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("dumping vfs data...\n");
  close_global_log();

  //vfs_add_data* data = get_sdstor_dev_fs_info_base();
  vfs_add_data* data = get_sdstor_dummy_ttyp_dev_fs_info_base();
  if((uint32_t)data == -1)
    return -1;

  while(data != 0)
  {
    open_global_log();
    {
      snprintf(sprintfBuffer, 256, "name: %s\n", data->name);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    close_global_log();

    data = data->next_element;
  }

  return 0;
}

vfs_node_info* get_vfs_node_info_base()
{
  tai_module_info_t m_info;
  m_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceIofilemgr", &m_info) >= 0) 
  {
    uintptr_t addr = 0;
    int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 1, 0x01A8, &addr);
    if(ofstRes == 0)
    {
      vfs_node_info* data = (vfs_node_info*)addr;
      return data;
    }
  }
  return (vfs_node_info*)-1;
}

int dump_vfs_node(vfs_node* node)
{
  if(node == 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("node is not set\n");
    close_global_log();
    return 0;
  }

  //actually this is NOT vfs_device_info type, need to fix

  vfs_device_info* dev_info = (vfs_device_info*)node->dev_info;
  if(dev_info == 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("dev info is not set\n");
    close_global_log();
    return 0;
  }  

  if(node->prev_node != 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("prev node is set\n");
    close_global_log();
  }

  //print_current_thread_info_global();

  open_global_log();
  {
    int segidx = find_in_segments(g_segListKernel, SEG_LIST_SIZE, &moduleListIsConstructedKernel, (uintptr_t)dev_info->partition);
    if(segidx >= 0)
    {
      snprintf(sprintfBuffer, 256, "ready: %s %x %x %x\n", g_segListKernel[segidx].moduleName, dev_info->partition, dev_info->device, dev_info->unk_8);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
    else
    {
      snprintf(sprintfBuffer, 256, "ready: %x %x %x\n", dev_info->partition, dev_info->device, dev_info->unk_8);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    }
  }
  close_global_log();

  return 0;

  /*
  partition_entry* partition = dev_info->partition;
  if(partition != 0)
  {
      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "partition: %s\n", partition->numericName);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      close_global_log();
  }
  else
  {
      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "partition: %s\n", "none");
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      close_global_log();
  }

  sd_stor_device* device = dev_info->device;
  if(device != 0)
  {

  }
  */

  return 0;
}

int dump_vfs_node_info()
{
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("dumping vfs node info...\n");
  close_global_log();

  vfs_node_info* data = get_vfs_node_info_base();
  if((uint32_t)data == -1)
    return -1;

  for(int i = 0; i < 96; i++)
  {
    if(strlen(data->name) > 0)
    {
      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "name: %s\n", data->name);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      close_global_log(); 

      dump_vfs_node(data->node);
    }

    data++;
  }

  return 0;
}

sdstor_mbr_ctx* get_sdstor_mbr_ctx_base()
{
  tai_module_info_t m_info;
  m_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &m_info) >= 0) 
  {
    uintptr_t addr = 0;
    int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 1, 0xA40, &addr);
    if(ofstRes == 0)
    {
      sdstor_mbr_ctx* data = (sdstor_mbr_ctx*)addr;
      return data;
    }
  }
  return (sdstor_mbr_ctx*)-1;
}

typedef struct mount_data
{
  uint8_t sdstor_mbr_ctx_index; //index or -1
  uint8_t unk1;
  uint8_t partitionCode;
  uint8_t unk3;
}mount_data;

typedef int (call_proc_get_mount)(char* blockDeviceName, int nameLength, mount_data* mountData);

partition_entry* find_partition_entry_by_code(sdstor_mbr_ctx* dev, char partitionCode)
{
  for(int i = 0; i < 17; i++)
  {
    partition_entry* pEntry = &dev->partitions[i];
    partition_entry_raw* praw = pEntry->partitionEntryPtr;
    if(praw == 0)
       continue;

    if(praw->partitionCode == partitionCode)
       return pEntry;
  }

  return 0;
}

int call_proc_get_mount_data_C15B80(char* blockDeviceName)
{
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("call_proc_get_mount_data_C15B80...\n");
  close_global_log();

  tai_module_info_t m_info;
  m_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdstor", &m_info) >= 0) 
  {
    uintptr_t addr = 0;
    int ofstRes = module_get_offset(KERNEL_PID, m_info.modid, 0, 0x1B81, &addr);
    if(ofstRes == 0)
    {
      call_proc_get_mount* proc = (call_proc_get_mount*)addr;

      mount_data mountData;

      int res = proc(blockDeviceName, strlen(blockDeviceName), &mountData);

      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "block: %s mount %x %x %x %x res: %x\n", blockDeviceName, mountData.sdstor_mbr_ctx_index, mountData.unk1, mountData.partitionCode, mountData.unk3 , res);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
      }
      close_global_log();

      sdstor_mbr_ctx* devBase = get_sdstor_mbr_ctx_base();
      if(mountData.sdstor_mbr_ctx_index != 0xFF)
      {
         sdstor_mbr_ctx* curDevice = devBase + mountData.sdstor_mbr_ctx_index;

         partition_entry* pentry = find_partition_entry_by_code(curDevice, mountData.partitionCode);
         
         if(pentry != 0)
         {
            open_global_log();
            {
              snprintf(sprintfBuffer, 256, "numeric: %s\n", pentry->numericName);
              FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
            }
            close_global_log();
         }
         
      }

      return 0;
    }
  }
  return 0;
}