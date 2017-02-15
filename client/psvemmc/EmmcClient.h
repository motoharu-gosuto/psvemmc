#pragma once

#include <stdint.h>

//TODO: need to merge server and client definitions into single file
//these are much better than original on server

#define PSVEMMC_COMMAND_PING 0
#define PSVEMMC_COMMAND_TERM 1

#define PSVEMMC_COMMAND_INIT 2
#define PSVEMMC_COMMAND_DEINIT 3

#define PSVEMMC_COMMAND_READ_SECTOR 4
#define PSVEMMC_COMMAND_READ_CLUSTER 5
#define PSVEMMC_COMMAND_WRITE_SECTOR 6
#define PSVEMMC_COMMAND_WRITE_CLUSTER 7

#define PSVEMMC_COMMAND_READ_SECTOR_MS 8
#define PSVEMMC_COMMAND_READ_CLUSTER_MS 9
#define PSVEMMC_COMMAND_WRITE_SECTOR_MS 10
#define PSVEMMC_COMMAND_WRITE_CLUSTER_MS 11

#define PSVEMMC_COMMAND_READ_SECTOR_GC 12
#define PSVEMMC_COMMAND_READ_CLUSTER_GC 13
#define PSVEMMC_COMMAND_WRITE_SECTOR_GC 14
#define PSVEMMC_COMMAND_WRITE_CLUSTER_GC 15

#pragma pack(push, 1)

struct command_response_base
{
   uint32_t command;
   uint32_t vita_err;
   uint32_t proxy_err;
};

struct command_0_request //ping
{
   int command;
};
 
struct command_0_response
{
   command_response_base base;
   char data[10];
};

struct command_1_request //terminate
{
   int command;
};

struct command_1_response
{
   command_response_base base;
   char data[10];
};

struct command_2_request //init
{
   int command;
   int bytesPerSector;
   int sectorsPerCluster;
};

struct command_2_response
{
   command_response_base base;
};

struct command_3_request //deinit
{
   int command;
};

struct command_3_response
{
   command_response_base base;
};

//--------------------------------------

struct command_4_request //read sector
{
   int command;
   int sector;
};

struct command_4_response
{
   command_response_base base;
   char data[0x200];
};

struct command_5_request //read cluster
{
   int command;
   int cluster;
};

struct command_5_response
{
   command_response_base base;
   //variable data length : g_bytesPerSector * g_sectorsPerCluster
};

struct command_6_request //write sector
{
   int command;
   int sector;
   char data[0x200];
};

struct command_6_response
{
   command_response_base base;
};

struct command_7_request //write cluster
{
   int command;
   int cluster;
   //variable data length : g_bytesPerSector * g_sectorsPerCluster
};

struct command_7_response
{
   command_response_base base;
};

//--------------------------------------

typedef struct command_8_request //read sector ms
{
   int command;
   int sector;
};

typedef struct command_8_response
{
   command_response_base base;
   char data[0x200];
};

typedef struct command_9_request //read cluster ms
{
   int command;
   int cluster;
};

typedef struct command_9_response
{
   command_response_base base;
   //variable data length : g_bytesPerSector * g_sectorsPerCluster
};

typedef struct command_10_request //write sector ms
{
   int command;
   int sector;
   char data[0x200];
};

typedef struct command_10_response
{
   command_response_base base;
};

typedef struct command_11_request //write cluster ms
{
   int command;
   int cluster;
   //variable data length : g_bytesPerSector * g_sectorsPerCluster
};

typedef struct command_11_response
{
   command_response_base base;
};

//--------------------------------------

typedef struct command_12_request //read sector gc
{
   int command;
   int sector;
};

typedef struct command_12_response
{
   command_response_base base;
   char data[0x200];
};

typedef struct command_13_request //read cluster gc
{
   int command;
   int cluster;
};

typedef struct command_13_response
{
   command_response_base base;
   //variable data length : g_bytesPerSector * g_sectorsPerCluster
};

typedef struct command_14_request //write sector gc
{
   int command;
   int sector;
   char data[0x200];
};

typedef struct command_14_response
{
   command_response_base base;
};

typedef struct command_15_request //write cluster gc
{
   int command;
   int cluster;
   //variable data length : g_bytesPerSector * g_sectorsPerCluster
};

typedef struct command_15_response
{
   command_response_base base;
};

#pragma pack(pop)

int initialize_emmc_proxy_connection(SOCKET& ConnectSocket);

int deinitialize_emmc_proxy_connection(SOCKET ConnectSocket);