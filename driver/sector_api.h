#pragma once

#include <stdint.h>

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
