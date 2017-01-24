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

sd_context_global* ksceSdifGetSdContextGlobal(int sd_ctx_idx);

sd_context_part* ksceSdifGetSdContextPartEmmc(int sd_ctx_idx);
sd_context_part* ksceSdifGetSdContextPartGameCart(int sd_ctx_idx);
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

int psvemmcIntialize(int sectorsPerCluster)
{
  return 0;
}

int readSectorAsync(int sector, char* buffer, int nSectors)
{
  return 0;
}

int writeSectorAsync(int sector, char* buffer, int nSectors)
{
  return 0;
}

int psvemmcDeinitialize()
{
  return 0;
}

//=================================================

int module_start(SceSize argc, const void *args) 
{  
  //initialize emmc card if required
  
  return SCE_KERNEL_START_SUCCESS;
}
 
//Alias to inhibit compiler warning
void _start() __attribute__ ((weak, alias ("module_start")));
 
int module_stop(SceSize argc, const void *args) 
{
  //deinitialize buffers if required
  
  return SCE_KERNEL_STOP_SUCCESS;
}
