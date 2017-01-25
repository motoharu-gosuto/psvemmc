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
