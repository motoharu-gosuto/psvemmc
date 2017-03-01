#pragma once

#include <stdint.h>

#pragma pack(push,1)

typedef struct partition_entry_raw
{
   uint32_t partitionOffset;
   uint32_t partitionSize;
   uint8_t partitionCode;
   uint8_t partitionType;
   uint8_t partitionActive;
   uint32_t flags;
   uint16_t unk;
}partition_entry_raw;

struct sdstor_mbr_ctx;

typedef struct partition_entry //size is 0x1C
{
   uint32_t partitionOffset; //from MBR
   uint32_t partitionSize; //from MBR
   struct sdstor_mbr_ctx* ctx;
   struct partition_entry_raw* partitionEntryPtr; //pointer to partition entry in MBR
   char numericName[4]; //null terminated name (000, 001, ...)
   uint32_t flags1; //from MBR
   uint16_t unk2; //from MBR
   uint16_t unk3; //?
}partition_entry;

struct sd_stor_device;

typedef struct sdstor_mbr_ctx //size is 0x238
{
 uint32_t fast_mutex_SceSdStorPartitionTable;
 uint32_t unk_4;
 uint32_t unk_8;
 uint32_t unk_C;

 uint8_t unk_0[0x30];

 uint32_t unk_40;
 struct sd_stor_device* unk_44;
 uint32_t unk_48;
 uint32_t unk_4C;

 uint32_t unk_50;
 void* mbr_ptr; //pointer to corresponding MBR record in array (offset 0x40)
 struct partition_entry partitions[17]; // 16 real partition entries from MBR + some hardcoded 17th partition entry
 uint8_t unk_234; //some initialization flag (0, 1, 2)
 uint8_t unk_235;
 uint8_t unk_236; //some initialization flag (set to 0 if partition table is initialized)
 uint8_t unk_237;
}sdstor_mbr_ctx;

//named after "SceSdStorDevice" mutex
typedef struct sd_stor_device //size is 0x54
{
   uint32_t fast_mutex_SceSdStorDevice;
   uint32_t unk_4;
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
   
   char index1; // (0x00, 0x01, 0xFF, 0xFF, 0x03) // sd ctx index used to get sd_context_part* (0, 1 confirmed) (3 unknown)
   char index2; // (0x00, 0x00 / 0x01, 0x02, 0x03, 0x01) // device type idx , not related to sdif type!
   char unk_42;
   char unk_43;
   
   uint32_t unk_44;
   uint32_t unk_48;
   uint32_t unk_4C;
   
   void* devCtx; //pointer to dev ctx (sd_context_part_mmc*) or zero
}sd_stor_device;

//named after SceSdStorDeviceHandle mutex and pool
typedef struct sd_stor_device_handle //size is 0x58
{
   uint32_t fast_mutex_SceSdStorDeviceHandle;
   uint32_t unk_4;
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
   uint32_t partitionOffset;
   uint32_t partitionSize;
   uint32_t unk_4C;

   struct partition_entry* pentry;
   struct sd_stor_device* dev;
}sd_stor_device_handle; 

#pragma pack(pop)