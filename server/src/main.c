#include <stdio.h>
#include <malloc.h>

#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/processmgr.h>

#include <psvemmc.h>

#include "debugScreen.h"

//net initialization part is taken from xerpi:
//https://gist.github.com/xerpi/e426284df19c217a8128

//also some usefull info here:
//https://github.com/xerpi/libftpvita/blob/master/libftpvita/ftpvita.c
//https://github.com/psxdev/debugnet/blob/master/libdebugnet/source/debugnet.c
 
//some refresher info about sockets here
//http://stackoverflow.com/questions/16486361/creating-a-basic-c-c-tcp-socket-writer
//http://matrixsust.blogspot.ru/2011/10/basic-tcp-server-client.html
//http://www.linuxhowtos.org/C_C++/socket.htm

#define NET_INIT_SIZE 1*1024*1024
 
int _emmc_sock = 0;
void *net_memory = NULL;

int emmc_port = 1331;
 
char* emmc_sock_name = "emmcproxy";
 
int _cli_sock = 0;

int init_net()
{
  if (sceNetShowNetstat() == SCE_NET_ERROR_ENOTINIT) 
  {
      net_memory = malloc(NET_INIT_SIZE);

      SceNetInitParam initparam;
      initparam.memory = net_memory;
      initparam.size = NET_INIT_SIZE;
      initparam.flags = 0;

      sceNetInit(&initparam);
      sceKernelDelayThread(100 * 1000);
      
      psvDebugScreenPrintf("psvemmc: net initialized\n");
  }
  
  if (sceNetCtlInit() < 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to initialize netctl\n");
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: netctl initialized\n");
     
  SceNetSockaddrIn server;
    
  server.sin_len = sizeof(server);
  server.sin_family = SCE_NET_AF_INET;
  server.sin_addr.s_addr = SCE_NET_INADDR_ANY;
  server.sin_port = sceNetHtons(emmc_port);
  
  memset(server.sin_zero, 0, sizeof(server.sin_zero));

  _emmc_sock = sceNetSocket(emmc_sock_name, SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
  if(_emmc_sock < 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to create socket\n");
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: server socket created\n");
    
  int bind_res = sceNetBind(_emmc_sock, (SceNetSockaddr*)&server, sizeof(server));
  if(bind_res < 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to bind socket %x\n", bind_res);
    return -1;
  }
  
  SceNetCtlInfo info;
  if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) < 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to get network info\n");
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: server socket binded %s:%d\n", info.ip_address, emmc_port);
   
  if(sceNetListen(_emmc_sock, 128) < 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to listen socket\n");
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: listening for connection\n");
  
  return 0;
}

void deinit_net()
{
  if(_cli_sock)
  {
    if(sceNetSocketClose(_cli_sock) < 0)
    psvDebugScreenPrintf("psvemmc: failed to close client socket\n");
    _cli_sock = 0;
  }
 
  if (_emmc_sock) 
  {
    if(sceNetSocketClose(_emmc_sock) < 0)
    psvDebugScreenPrintf("psvemmc: failed to close server socket\n");
    _emmc_sock = 0;
  }
  
  sceNetCtlTerm();
  
  sceNetTerm();
  
  if (net_memory) 
  {
    free(net_memory);
    net_memory = NULL;
  }
}

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

typedef struct command_0_request //ping
{
  int command;
} command_0_request;
 
typedef struct command_0_response
{
    int command;
    int vita_err;
    int proxy_err;
    char data[10];
} command_0_response;

typedef struct command_1_request //terminate
{
  int command;
} command_1_request;

typedef struct command_1_response
{
    int command;
    int vita_err;
    int proxy_err;
    char data[10];
} command_1_response;

typedef struct command_2_request //init
{
  int command;
  int bytesPerSector;
  int sectorsPerCluster;
} command_2_request;

typedef struct command_2_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_2_response;

typedef struct command_3_request //deinit
{
  int command;
} command_3_request;

typedef struct command_3_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_3_response;

//--------------------------------------

typedef struct command_4_request //read sector
{
  int command;
  int sector;
} command_4_request;

typedef struct command_4_response
{
    int command;
    int vita_err;
    int proxy_err;
    char data[0x200];
} command_4_response;

typedef struct command_5_request //read cluster
{
  int command;
  int cluster;
} command_5_request;

typedef struct command_5_response
{
    int command;
    int vita_err;
    int proxy_err;
    //variable data length : g_bytesPerSector * g_sectorsPerCluster
} command_5_response;

typedef struct command_6_request //write sector
{
  int command;
  int sector;
  char data[0x200];
} command_6_request;

typedef struct command_6_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_6_response;

typedef struct command_7_request //write cluster
{
  int command;
  int cluster;
  //variable data length : g_bytesPerSector * g_sectorsPerCluster
} command_7_request;

typedef struct command_7_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_7_response;

//--------------------------------------

typedef struct command_8_request //read sector ms
{
  int command;
  int sector;
} command_8_request;

typedef struct command_8_response
{
    int command;
    int vita_err;
    int proxy_err;
    char data[0x200];
} command_8_response;

typedef struct command_9_request //read cluster ms
{
  int command;
  int cluster;
} command_9_request;

typedef struct command_9_response
{
    int command;
    int vita_err;
    int proxy_err;
    //variable data length : g_bytesPerSector * g_sectorsPerCluster
} command_9_response;

typedef struct command_10_request //write sector ms
{
  int command;
  int sector;
  char data[0x200];
} command_10_request;

typedef struct command_10_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_10_response;

typedef struct command_11_request //write cluster ms
{
  int command;
  int cluster;
  //variable data length : g_bytesPerSector * g_sectorsPerCluster
} command_11_request;

typedef struct command_11_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_11_response;

//--------------------------------------

typedef struct command_12_request //read sector gc
{
  int command;
  int sector;
} command_12_request;

typedef struct command_12_response
{
    int command;
    int vita_err;
    int proxy_err;
    char data[0x200];
} command_12_response;

typedef struct command_13_request //read cluster gc
{
  int command;
  int cluster;
} command_13_request;

typedef struct command_13_response
{
    int command;
    int vita_err;
    int proxy_err;
    //variable data length : g_bytesPerSector * g_sectorsPerCluster
} command_13_response;

typedef struct command_14_request //write sector gc
{
  int command;
  int sector;
  char data[0x200];
} command_14_request;

typedef struct command_14_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_14_response;

typedef struct command_15_request //write cluster gc
{
  int command;
  int cluster;
  //variable data length : g_bytesPerSector * g_sectorsPerCluster
} command_15_request;

typedef struct command_15_response
{
    int command;
    int vita_err;
    int proxy_err;
} command_15_response;

//--------------------------------------

#pragma pack(pop)

int handle_command_0() //ping
{
  command_0_response resp;
  memset(&resp, 0, sizeof(command_0_response));
  resp.command = PSVEMMC_COMMAND_PING;
  memcpy(resp.data, "emmcproxy", 9);
  
  psvDebugScreenPrintf("psvemmc: execute command 0\n");

  return sceNetSend(_cli_sock, &resp, sizeof(command_0_response), 0);
}

int handle_command_1() //terminate
{
  command_1_response resp;
  memset(&resp, 0, sizeof(command_1_response));
  resp.command = PSVEMMC_COMMAND_TERM;
  memcpy(resp.data, "emmcproxy", 9);
  
  psvDebugScreenPrintf("psvemmc: execute command 1\n");

  return sceNetSend(_cli_sock, &resp, sizeof(command_1_response), 0);
}

//defalut size of sector for SD MMC protocol
#define SD_DEFAULT_SECTOR_SIZE 0x200

static int g_bytesPerSector = 0;
static int g_sectorsPerCluster = 0;

static int g_clusterPoolInitialized = 0;

static SceUID g_clusterPool = 0;

static void* g_clusterPoolPtr = 0;

int initializePool(int bytesPerSector, int sectorsPerCluster)
{
  if(g_clusterPoolInitialized != 0)
    return -1;
  
  if(bytesPerSector != SD_DEFAULT_SECTOR_SIZE)
    return -2;
  
  g_bytesPerSector = bytesPerSector;
  g_sectorsPerCluster = sectorsPerCluster;
  
  g_clusterPool = sceKernelAllocMemBlock("cluster_pool", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, g_bytesPerSector * g_sectorsPerCluster, 0);
  if(g_clusterPool < 0)
    return g_clusterPool;

  int res_1 = sceKernelGetMemBlockBase(g_clusterPool, &g_clusterPoolPtr);
  if(res_1 < 0)
    return res_1;
  
  g_clusterPoolInitialized = 1;
  
  return 0;
}

int deinitializePool()
{
  if(g_clusterPoolInitialized == 0)
    return -1;
  
  int res_1 = sceKernelFreeMemBlock(g_clusterPool);
  if(res_1 < 0)
    return res_1;
  
  g_clusterPoolInitialized = 0;
  
  return 0;
}

int handle_command_2() //init
{
  command_2_response resp;
  memset(&resp, 0, sizeof(command_2_response));
  resp.command = PSVEMMC_COMMAND_INIT;
  
  if(g_clusterPoolInitialized != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 2\n");
    resp.vita_err = -2;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_2_response), 0);
    return -2;
  }
  
  int expLen = sizeof(command_2_request) - sizeof(int); //receive rest of the request
  
  command_2_request req;
  req.command = PSVEMMC_COMMAND_INIT;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.bytesPerSector, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 2\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_2_response), 0);
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 2\n");
  
  //initialize pool
  resp.vita_err = initializePool(req.bytesPerSector, req.sectorsPerCluster);
  if(resp.vita_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to initialize pool\n");
    return sceNetSend(_cli_sock, &resp, sizeof(command_2_response), 0);
  }
  
  //execute kernel function
  resp.proxy_err = psvemmcInitialize(req.bytesPerSector, req.sectorsPerCluster);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 2\n");
  }

  return sceNetSend(_cli_sock, &resp, sizeof(command_2_response), 0);
}

int handle_command_3() //deinit
{
  command_3_response resp;
  memset(&resp, 0, sizeof(command_3_response));
  resp.command = PSVEMMC_COMMAND_DEINIT;
  
  if(g_clusterPoolInitialized == 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 3\n");
    resp.vita_err = -2;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_3_response), 0);
    return -2;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 3\n");
  
  //execute kernel function
  resp.proxy_err = psvemmcDeinitialize();
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 3\n");
  }
  
  //deinitialize pool
  resp.vita_err = deinitializePool();
  if(resp.vita_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to deinitialize pool\n");
  }

  return sceNetSend(_cli_sock, &resp, sizeof(command_3_response), 0);
}

//-------------------------------------------------------------------------------

int handle_command_4() //read sector
{
  command_4_response resp;
  memset(&resp, 0, sizeof(command_4_response));
  resp.command = PSVEMMC_COMMAND_READ_SECTOR;
  
  int expLen = sizeof(command_4_request) - sizeof(int); //receive rest of the request
  
  command_4_request req;
  req.command = PSVEMMC_COMMAND_READ_SECTOR;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.sector, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 4\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_4_response), 0);
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 4\n");

  resp.proxy_err = readSector(req.sector, resp.data);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 4\n");
  }

  int bytesToSend = sizeof(command_4_response);
  int bytesWereSend = 0;
  while(bytesWereSend != bytesToSend)
  {
     int sendLen = sceNetSend(_cli_sock, ((char*)&resp) + bytesWereSend, bytesToSend - bytesWereSend, 0);
     if(sendLen <= 0)
     {
        psvDebugScreenPrintf("psvemmc: failed to send data\n");
        return - 1;
     }
     
     bytesWereSend = bytesWereSend + sendLen;
  }
  
  return 0;
}

int handle_command_5() //read cluster
{
  command_5_response resp;
  memset(&resp, 0, sizeof(command_5_response));
  resp.command = PSVEMMC_COMMAND_READ_CLUSTER;
  
  int expLen = sizeof(command_5_request) - sizeof(int); //receive rest of the request
  
  command_5_request req;
  req.command = PSVEMMC_COMMAND_READ_CLUSTER;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.cluster, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 5\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_5_response), 0);
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 5\n");
  
  resp.proxy_err = readCluster(req.cluster, g_clusterPoolPtr);
  
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 5\n");
  }
  
  //send header first
  sceNetSend(_cli_sock, &resp, sizeof(command_5_response), 0);
  
  //send cluster data next
  int bytesToSend = g_bytesPerSector * g_sectorsPerCluster;
  int bytesWereSend = 0;
  while(bytesWereSend != bytesToSend)
  {
     int sendLen = sceNetSend(_cli_sock, ((char*)g_clusterPoolPtr) + bytesWereSend, bytesToSend - bytesWereSend, 0);
     if(sendLen <= 0)
     {
        psvDebugScreenPrintf("psvemmc: failed to send data\n");
        return - 1;
     }
     
     bytesWereSend = bytesWereSend + sendLen;
  }
  
  return 0;
}

int handle_command_6() //write sector
{
  command_6_response resp;
  memset(&resp, 0, sizeof(command_6_response));
  resp.command = PSVEMMC_COMMAND_WRITE_SECTOR;
  
  int expLen = sizeof(command_6_request) - sizeof(int); //receive rest of the request
  
  command_6_request req;
  req.command = PSVEMMC_COMMAND_WRITE_SECTOR;
  
  int bytesToReceive = expLen;
  int bytesWereReceived = 0;
  while(bytesToReceive != bytesWereReceived)
  {
    int recvLen = sceNetRecv(_cli_sock, ((char*)&req) + sizeof(int) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
    if(recvLen <= 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to receive command 6\n");
      resp.vita_err = -1;
    
      sceNetSend(_cli_sock, &resp, sizeof(command_6_response), 0);
      return -1;
    }
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 6\n");

  resp.proxy_err = writeSector(req.sector, req.data);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 6\n");
  }
  
  return sceNetSend(_cli_sock, &resp, sizeof(command_6_response), 0);
}

int handle_command_7() //write cluster
{
  command_7_response resp;
  memset(&resp, 0, sizeof(command_7_response));
  resp.command = PSVEMMC_COMMAND_WRITE_CLUSTER;
  
  int expLen = sizeof(command_7_request) - sizeof(int); //receive rest of the request
  
  command_7_request req;
  req.command = PSVEMMC_COMMAND_WRITE_CLUSTER;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.cluster, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 7\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_7_response), 0);
    return -1;
  }
  
  //now receive data to write into the pool
  
  int bytesToReceive = g_bytesPerSector * g_sectorsPerCluster;
  int bytesWereReceived = 0;
  while(bytesToReceive != bytesWereReceived)
  {
    int recvLen = sceNetRecv(_cli_sock, ((char*)g_clusterPoolPtr) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
    if(recvLen <= 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to receive command 7\n");
      resp.vita_err = -1;
    
      sceNetSend(_cli_sock, &resp, sizeof(command_7_response), 0);
      return -1;
    }
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 7\n");
  
  resp.proxy_err = writeCluster(req.cluster, g_clusterPoolPtr);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 7\n");
  }
  
  return sceNetSend(_cli_sock, &resp, sizeof(command_7_response), 0);
}

//-------------------------------------------------------------------------------

int handle_command_8() //read sector
{
  command_8_response resp;
  memset(&resp, 0, sizeof(command_8_response));
  resp.command = PSVEMMC_COMMAND_READ_SECTOR_MS;
  
  int expLen = sizeof(command_8_request) - sizeof(int); //receive rest of the request
  
  command_8_request req;
  req.command = PSVEMMC_COMMAND_READ_SECTOR_MS;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.sector, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 8\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_8_response), 0);
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 8\n");

  resp.proxy_err = readSectorMs(req.sector, resp.data);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 8\n");
  }

  int bytesToSend = sizeof(command_8_response);
  int bytesWereSend = 0;
  while(bytesWereSend != bytesToSend)
  {
     int sendLen = sceNetSend(_cli_sock, ((char*)&resp) + bytesWereSend, bytesToSend - bytesWereSend, 0);
     if(sendLen <= 0)
     {
        psvDebugScreenPrintf("psvemmc: failed to send data\n");
        return - 1;
     }
     
     bytesWereSend = bytesWereSend + sendLen;
  }
  
  return 0;
}

int handle_command_9() //read cluster
{
  command_9_response resp;
  memset(&resp, 0, sizeof(command_9_response));
  resp.command = PSVEMMC_COMMAND_READ_CLUSTER_MS;
  
  int expLen = sizeof(command_9_request) - sizeof(int); //receive rest of the request
  
  command_9_request req;
  req.command = PSVEMMC_COMMAND_READ_CLUSTER_MS;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.cluster, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 9\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_9_response), 0);
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 9\n");
  
  resp.proxy_err = readClusterMs(req.cluster, g_clusterPoolPtr);
  
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 9\n");
  }
  
  //send header first
  sceNetSend(_cli_sock, &resp, sizeof(command_9_response), 0);
  
  //send cluster data next
  int bytesToSend = g_bytesPerSector * g_sectorsPerCluster;
  int bytesWereSend = 0;
  while(bytesWereSend != bytesToSend)
  {
     int sendLen = sceNetSend(_cli_sock, ((char*)g_clusterPoolPtr) + bytesWereSend, bytesToSend - bytesWereSend, 0);
     if(sendLen <= 0)
     {
        psvDebugScreenPrintf("psvemmc: failed to send data\n");
        return - 1;
     }
     
     bytesWereSend = bytesWereSend + sendLen;
  }
  
  return 0;
}

int handle_command_10() //write sector
{
  command_10_response resp;
  memset(&resp, 0, sizeof(command_10_response));
  resp.command = PSVEMMC_COMMAND_WRITE_SECTOR_MS;
  
  int expLen = sizeof(command_10_request) - sizeof(int); //receive rest of the request
  
  command_10_request req;
  req.command = PSVEMMC_COMMAND_WRITE_SECTOR_MS;
  
  int bytesToReceive = expLen;
  int bytesWereReceived = 0;
  while(bytesToReceive != bytesWereReceived)
  {
    int recvLen = sceNetRecv(_cli_sock, ((char*)&req) + sizeof(int) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
    if(recvLen <= 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to receive command 10\n");
      resp.vita_err = -1;
    
      sceNetSend(_cli_sock, &resp, sizeof(command_10_response), 0);
      return -1;
    }
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 10\n");

  resp.proxy_err = writeSectorMs(req.sector, req.data);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 10\n");
  }
  
  return sceNetSend(_cli_sock, &resp, sizeof(command_10_response), 0);
}

int handle_command_11() //write cluster
{
  command_11_response resp;
  memset(&resp, 0, sizeof(command_11_response));
  resp.command = PSVEMMC_COMMAND_WRITE_CLUSTER_MS;
  
  int expLen = sizeof(command_11_request) - sizeof(int); //receive rest of the request
  
  command_11_request req;
  req.command = PSVEMMC_COMMAND_WRITE_CLUSTER_MS;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.cluster, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 11\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_11_response), 0);
    return -1;
  }
  
  //now receive data to write into the pool
  
  int bytesToReceive = g_bytesPerSector * g_sectorsPerCluster;
  int bytesWereReceived = 0;
  while(bytesToReceive != bytesWereReceived)
  {
    int recvLen = sceNetRecv(_cli_sock, ((char*)g_clusterPoolPtr) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
    if(recvLen <= 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to receive command 11\n");
      resp.vita_err = -1;
    
      sceNetSend(_cli_sock, &resp, sizeof(command_11_response), 0);
      return -1;
    }
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 11\n");
  
  resp.proxy_err = writeClusterMs(req.cluster, g_clusterPoolPtr);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 11\n");
  }
  
  return sceNetSend(_cli_sock, &resp, sizeof(command_11_response), 0);
}

//-------------------------------------------------------------------------------

int handle_command_12() //read sector
{
  command_12_response resp;
  memset(&resp, 0, sizeof(command_12_response));
  resp.command = PSVEMMC_COMMAND_READ_SECTOR_GC;
  
  int expLen = sizeof(command_12_request) - sizeof(int); //receive rest of the request
  
  command_12_request req;
  req.command = PSVEMMC_COMMAND_READ_SECTOR_GC;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.sector, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 12\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_12_response), 0);
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 12\n");

  resp.proxy_err = readSectorGc(req.sector, resp.data);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 12\n");
  }

  int bytesToSend = sizeof(command_12_response);
  int bytesWereSend = 0;
  while(bytesWereSend != bytesToSend)
  {
     int sendLen = sceNetSend(_cli_sock, ((char*)&resp) + bytesWereSend, bytesToSend - bytesWereSend, 0);
     if(sendLen <= 0)
     {
        psvDebugScreenPrintf("psvemmc: failed to send data\n");
        return - 1;
     }
     
     bytesWereSend = bytesWereSend + sendLen;
  }
  
  return 0;
}

int handle_command_13() //read cluster
{
  command_13_response resp;
  memset(&resp, 0, sizeof(command_13_response));
  resp.command = PSVEMMC_COMMAND_READ_CLUSTER_GC;
  
  int expLen = sizeof(command_13_request) - sizeof(int); //receive rest of the request
  
  command_13_request req;
  req.command = PSVEMMC_COMMAND_READ_CLUSTER_GC;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.cluster, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 13\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_13_response), 0);
    return -1;
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 13\n");
  
  resp.proxy_err = readClusterGc(req.cluster, g_clusterPoolPtr);
  
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 13\n");
  }
  
  //send header first
  sceNetSend(_cli_sock, &resp, sizeof(command_13_response), 0);
  
  //send cluster data next
  int bytesToSend = g_bytesPerSector * g_sectorsPerCluster;
  int bytesWereSend = 0;
  while(bytesWereSend != bytesToSend)
  {
     int sendLen = sceNetSend(_cli_sock, ((char*)g_clusterPoolPtr) + bytesWereSend, bytesToSend - bytesWereSend, 0);
     if(sendLen <= 0)
     {
        psvDebugScreenPrintf("psvemmc: failed to send data\n");
        return - 1;
     }
     
     bytesWereSend = bytesWereSend + sendLen;
  }
  
  return 0;
}

int handle_command_14() //write sector
{
  command_14_response resp;
  memset(&resp, 0, sizeof(command_14_response));
  resp.command = PSVEMMC_COMMAND_WRITE_SECTOR_GC;
  
  int expLen = sizeof(command_14_request) - sizeof(int); //receive rest of the request
  
  command_14_request req;
  req.command = PSVEMMC_COMMAND_WRITE_SECTOR_GC;
  
  int bytesToReceive = expLen;
  int bytesWereReceived = 0;
  while(bytesToReceive != bytesWereReceived)
  {
    int recvLen = sceNetRecv(_cli_sock, ((char*)&req) + sizeof(int) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
    if(recvLen <= 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to receive command 14\n");
      resp.vita_err = -1;
    
      sceNetSend(_cli_sock, &resp, sizeof(command_14_response), 0);
      return -1;
    }
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 14\n");

  resp.proxy_err = writeSectorGc(req.sector, req.data);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 14\n");
  }
  
  return sceNetSend(_cli_sock, &resp, sizeof(command_14_response), 0);
}

int handle_command_15() //write cluster
{
  command_15_response resp;
  memset(&resp, 0, sizeof(command_15_response));
  resp.command = PSVEMMC_COMMAND_WRITE_CLUSTER_GC;
  
  int expLen = sizeof(command_15_request) - sizeof(int); //receive rest of the request
  
  command_15_request req;
  req.command = PSVEMMC_COMMAND_WRITE_CLUSTER_GC;
  
  //TODO: I know that request should be properly received in cycle, however size of this request is small so this should be ok
  int recvLen = sceNetRecv(_cli_sock, &req.cluster, expLen, 0);
  if(recvLen != expLen)
  {
    psvDebugScreenPrintf("psvemmc: failed to receive command 15\n");
    resp.vita_err = -1;
    
    sceNetSend(_cli_sock, &resp, sizeof(command_15_response), 0);
    return -1;
  }
  
  //now receive data to write into the pool
  
  int bytesToReceive = g_bytesPerSector * g_sectorsPerCluster;
  int bytesWereReceived = 0;
  while(bytesToReceive != bytesWereReceived)
  {
    int recvLen = sceNetRecv(_cli_sock, ((char*)g_clusterPoolPtr) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
    if(recvLen <= 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to receive command 15\n");
      resp.vita_err = -1;
    
      sceNetSend(_cli_sock, &resp, sizeof(command_15_response), 0);
      return -1;
    }
  }
  
  psvDebugScreenPrintf("psvemmc: execute command 15\n");
  
  resp.proxy_err = writeClusterGc(req.cluster, g_clusterPoolPtr);
    
  if(resp.proxy_err != 0)
  {
    psvDebugScreenPrintf("psvemmc: failed to execute command 15\n");
  }
  
  return sceNetSend(_cli_sock, &resp, sizeof(command_15_response), 0);
}

//-------------------------------------------------------------------------------

void receive_commands()
{
  while(1)
  {
    int command = -1;
    int recvLen = sceNetRecv(_cli_sock, &command, sizeof(int), 0);
    if(recvLen <= 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to receive data\n");
      return;
    }
    
    switch(command)
    {
    case PSVEMMC_COMMAND_PING:
      if(handle_command_0() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 0\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_TERM:
      if(handle_command_1() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 1\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_INIT:
      if(handle_command_2() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 2\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_DEINIT:
      if(handle_command_3() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 3\n");
        return;
      }
      break;
      
    case PSVEMMC_COMMAND_READ_SECTOR:
      if(handle_command_4() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 4\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_READ_CLUSTER:
      if(handle_command_5() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 5\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_WRITE_SECTOR:
      if(handle_command_6() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 6\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_WRITE_CLUSTER:
      if(handle_command_7() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 7\n");
        return;
      }
      break;
      
    case PSVEMMC_COMMAND_READ_SECTOR_MS:
      if(handle_command_8() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 8\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_READ_CLUSTER_MS:
      if(handle_command_9() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 9\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_WRITE_SECTOR_MS:
      if(handle_command_10() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 10\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_WRITE_CLUSTER_MS:
      if(handle_command_11() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 11\n");
        return;
      }
      break;
      
    case PSVEMMC_COMMAND_READ_SECTOR_GC:
      if(handle_command_12() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 12\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_READ_CLUSTER_GC:
      if(handle_command_13() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 13\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_WRITE_SECTOR_GC:
      if(handle_command_14() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 14\n");
        return;
      }
      break;
    case PSVEMMC_COMMAND_WRITE_CLUSTER_GC:
      if(handle_command_15() < 0)
      {
        psvDebugScreenPrintf("psvemmc: failed to handle command 15\n");
        return;
      }
      break;
      
    default:
      psvDebugScreenPrintf("psvemmc: unknown command\n");
      return;
    }
  }
}

void accept_single_connection()
{
  while(1)
  {
    SceNetSockaddrIn client;
    memset(&client, 0, sizeof(client));
    client.sin_len = sizeof(client);

    unsigned int sin_size = sizeof(client);
    _cli_sock = sceNetAccept(_emmc_sock, (SceNetSockaddr*)&client, &sin_size);
    if(_cli_sock < 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to accept socket'\n");
      return;
    }

    char ipstr[16];
    psvDebugScreenPrintf("psvemmc: Accepted connection from %s:%d\n", sceNetInetNtop(SCE_NET_AF_INET, &client.sin_addr, ipstr, 16), sceNetNtohs(client.sin_port));

    receive_commands();
    
    if(_cli_sock)
    {
      if(sceNetSocketClose(_cli_sock) < 0)
      psvDebugScreenPrintf("psvemmc: failed to close client socket\n");
      _cli_sock = 0;
    }
    
    psvDebugScreenPrintf("psvemmc: closed client socket\n");
  }
}


int main(int argc, char *argv[]) 
{
  psvDebugScreenInit();

  psvDebugScreenPrintf("psvemmc: server started\n");

  if (sceSysmoduleIsLoaded(SCE_SYSMODULE_NET) != SCE_SYSMODULE_LOADED)
  {
    if(sceSysmoduleLoadModule(SCE_SYSMODULE_NET) < 0)
    {
      psvDebugScreenPrintf("psvemmc: failed to load net module\n");
      return 1;
    }
  }
  
  if(init_net() >= 0)
  {
    accept_single_connection();
  }
  
  deinit_net();


  psvDebugScreenPrintf("psvemmc: server stopped\n");

  sceKernelDelayThread(10*1000*1000);

  sceKernelExitProcess(0);
  return 0;
}
