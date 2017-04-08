#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "glog.h"
#include "net.h"

#include "defines.h"

SceUID g_connListenThid = -1;
SceUID g_connInitMutexId = -1;
int g_connInitialized = 0;

int _server_sock = 0;
int serv_port = 1332;
char* serv_sock_name = "emmcdebug";
int _client_sock = 0;

char sprintfBuffer[256];

//=============================

//since there are no exports in netps for ksceNetInetNtop and ksceNetNtohs I had to figure smth else

const char* inet_ntop4(const struct SceNetInAddr* addr, char* buf, unsigned int len)
{
	const uint8_t *ap = (const uint8_t *)&addr->s_addr;
	char tmp[16];
	int fulllen;
	 
	fulllen = snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", ap[0], ap[1], ap[2], ap[3]);
	if (fulllen >= (int)len) 
	   return NULL;
	
    memcpy(buf, tmp, fulllen + 1);
	
	return buf;
}

const char* ksceNetInetNtop(int af, const void *src, char *dst, unsigned int size)
{
  if(af==SCE_NET_AF_INET)
		return inet_ntop4(src, dst, size);
	return NULL;
}

unsigned short int ksceNetNtohs(unsigned short int net16)
{
  //implementation is copied from sceNetNtohs

  int r3 = net16 << 8;
  int r0 = r3 | (net16 >> 8);
  return r0;
}

//=============================

int lock_listen_mutex()
{
  unsigned int timeout = 0;
  int lockRes = ksceKernelLockMutex(g_connInitMutexId, 1, &timeout);
  /*
  if(lockRes >= 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("conn init mutex locked\n");
    close_global_log();
  }
  else
  {
    open_global_log();
    {
      snprintf(sprintfBuffer, 256, "failed to lock mutex %x\n", lockRes);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);    
    }
    close_global_log();
  }
  */

  return 0;
}

int unlock_listen_mutex()
{
  int unlockRes = ksceKernelUnlockMutex(g_connInitMutexId, 1);
  /*
  if(unlockRes >= 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("conn init mutex unlocked\n");
    close_global_log();
  }
  else
  {
    open_global_log();
    {
      snprintf(sprintfBuffer, 256, "failed to unlock mutex %x\n", unlockRes);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);    
    }
    close_global_log();
  }
  */

  return 0;
}

//=============================

void ping_debug_client();

void accept_single_connection()
{
  while(1)
  {
    lock_listen_mutex();

    if(g_connInitialized == 1)
    {
      ksceKernelDelayThread(5000); //delay for 5 seconds

      unlock_listen_mutex();

      continue;
    }

    unlock_listen_mutex();

    SceNetSockaddrIn client;
    memset(&client, 0, sizeof(client));
    client.sin_len = sizeof(client);

    unsigned int sin_size = sizeof(client);
    int clsock = ksceNetAccept(_server_sock, (SceNetSockaddr*)&client, &sin_size);
    if(clsock < 0)
    {
      open_global_log();
      {
        snprintf(sprintfBuffer, 256, "failed to accept socket %x\n", clsock);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);    
      }
      close_global_log();

      return;
    }

    _client_sock = clsock;

    char ipstr[16];

    open_global_log();
    {
      snprintf(sprintfBuffer, 256, "Accepted connection from %s:%d\n", ksceNetInetNtop(SCE_NET_AF_INET, &client.sin_addr, ipstr, 16), ksceNetNtohs(client.sin_port));
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer);    
    }
    close_global_log();

    //connection is now initialized 

    lock_listen_mutex();

    g_connInitialized = 1;

    unlock_listen_mutex();

    //try to send message to client

    ping_debug_client();
  }
}

int get_current_ip(char dest[16])
{
  //in userspace current ip can be retrieved with SceNetCtlInfo and sceNetCtlInetGetInfo
  //however I do not know how this can be done in kernel since these functions are not exported in kernel
  //for now I can only hardcode the value in this placeholder procedure

  memcpy(dest, "192.168.0.34", sizeof("192.168.0.34"));
  return 0;
}

int init_net_internal()
{
  SceNetSockaddrIn server;
    
  server.sin_len = sizeof(server);
  server.sin_family = SCE_NET_AF_INET;
  server.sin_addr.s_addr = SCE_NET_INADDR_ANY;
  server.sin_port = ksceNetHtons(serv_port);
  
  memset(server.sin_zero, 0, sizeof(server.sin_zero));

  _server_sock = ksceNetSocket(serv_sock_name, SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
  if(_server_sock < 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to create socket\n");
    close_global_log();
    return -1;
  }

  open_global_log();
  FILE_GLOBAL_WRITE_LEN("server socket created\n");
  close_global_log();

  int bind_res = ksceNetBind(_server_sock, (SceNetSockaddr*)&server, sizeof(server));
  if(bind_res < 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to bind socket\n");
    close_global_log();
    return -1;
  }

  char ip_address[16];
  get_current_ip(ip_address);

  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "server socket binded %s:%d\n", ip_address, serv_port);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);    
  }
  close_global_log();

  if(ksceNetListen(_server_sock, 128) < 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to listen socket\n");
    close_global_log();
    return -1;
  }
  
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("listening for connection\n");
  close_global_log();

  return 0;
}

//by some reason both client and server socket should be created in one thread
//if not then ksceNetAccept fails
//that is why all initialization is performed in separate thread

int ConnListenThread()
{
  if(init_net_internal() >= 0)
  {
    accept_single_connection();
  }

  open_global_log();
  FILE_GLOBAL_WRITE_LEN("exit listen thread\n");
  close_global_log();

  return 0;
}


//=============================

int init_listen_mutex()
{
  g_connInitMutexId = ksceKernelCreateMutex("ConnInitMutex", 0, 0, 0);

  if(g_connInitMutexId < 0)
  {
    open_global_log();
    {
      snprintf(sprintfBuffer, 256, "failed to create conn init mutex %x\n", g_connInitMutexId);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer); 
    }
    close_global_log();

    return -1;
  }
  else
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("created conn init mutex\n");
    close_global_log();
  }

  return 0;
}

int init_listen_thread()
{
  g_connListenThid = ksceKernelCreateThread("ConnListenThread", &ConnListenThread, 0x64, 0x1000, 0, 0, 0);
  if(g_connListenThid < 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to create thread\n");
    close_global_log();
    return -1;
  }
  
  open_global_log();
  {
    snprintf(sprintfBuffer, 256, "created thread %x\n", g_connListenThid);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  }
  close_global_log();

  return 0;
}

int init_net()
{
  if(init_listen_mutex() < 0)
     return -1;

  if(init_listen_thread() < 0)
    return -1;

  int ret = ksceKernelStartThread(g_connListenThid, 0, 0);  

  return 0;
}

//========================

void deinit_listen_mutex()
{
  int mutexRes = ksceKernelDeleteMutex(g_connInitMutexId);
  if(mutexRes < 0)
  {
    open_global_log();
    {
      snprintf(sprintfBuffer, 256, "failed to delete conn init mutex %x\n", mutexRes);
      FILE_GLOBAL_WRITE_LEN(sprintfBuffer); 
    }
    close_global_log();    
  }
  else
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("deleted conn init mutex\n");
    close_global_log();
  }
}

void deinit_listen_thread()
{
  int waitRet = 0;
  ksceKernelWaitThreadEnd(g_connListenThid, &waitRet, 0);
  
  int delret = ksceKernelDeleteThread(g_connListenThid);
  if(delret < 0)
  {
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to delete conn listen thread\n");
    close_global_log();
  }
  
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("deleted conn listen thread\n");
  close_global_log();
}

void close_client_sock()
{
  if(_client_sock)
  {
    if(ksceNetSocketClose(_client_sock) < 0)
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("failed to close client socket\n");
      close_global_log();
    }
    _client_sock = 0;
  }
  
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("closed client socket\n");
  close_global_log();
}

void close_server_sock()
{
  if(_server_sock)
  {
    if(ksceNetSocketClose(_server_sock) < 0)
    {
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("failed to close server socket\n");
      close_global_log();
    }
    _server_sock = 0;
  }
  
  open_global_log();
  FILE_GLOBAL_WRITE_LEN("closed server socket\n");
  close_global_log();
}

void deinit_net()
{
  deinit_listen_mutex();
  deinit_listen_thread();

  close_client_sock();
  close_server_sock();
}

//========================

//message send is locked with mutex so this buffer should be safe

char message_buffer[MSG_SIZE];

int send_message(char* msg_raw, int size)
{
  if(size > MSG_SIZE)
     return 0;

  memset(message_buffer, 0, MSG_SIZE);
  memcpy(message_buffer, msg_raw, size);

  int bytesToSend = MSG_SIZE;
  int bytesWereSend = 0;
  while(bytesWereSend != bytesToSend)
  {
     int sendLen = ksceNetSend(_client_sock, message_buffer + bytesWereSend, bytesToSend - bytesWereSend, 0);
     if(sendLen < 0)
     {
        #ifdef ENABLE_NETWORK_REPORT_SEND_FAILURE
        open_global_log();
        snprintf(sprintfBuffer, 256, "failed to send data %x\n", sendLen);
        FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
        close_global_log();
        #endif
        
        //return -1;
        
        return 0; // for debuging purpose. by unknown reason send_message fails from some hooks
     }
     
     bytesWereSend = bytesWereSend + sendLen;
  }

  return 0;
}

int send_message_to_client(char* msg, int size)
{
  if(size > MSG_SIZE)
  {
    #ifdef ENABLE_NETWORK_REPORT_SEND_FAILURE
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to send data: msg size is invalid\n");
    close_global_log();
    #endif
    return -1;
  }

  //message should only be sent if connection is initialized

  lock_listen_mutex();

  if(g_connInitialized == 1)
  {
    int res = send_message(msg, size);
    if(res < 0)
    {
      #ifdef ENABLE_NETWORK_REPORT_SEND_FAILURE
      open_global_log();
      FILE_GLOBAL_WRITE_LEN("failed to send message to client\n");
      close_global_log();
      #endif

      //if failed to send message this means that most likely connection was terminated
      //we need to reinitialize connection then (mutex is already locked)
      g_connInitialized = 0;

      //need to close socket as well
      close_client_sock();
    }
  }
  else
  {
    #ifdef ENABLE_NETWORK_REPORT_SEND_FAILURE
    open_global_log();
    FILE_GLOBAL_WRITE_LEN("failed to send message to client: connection is not initialized\n");
    close_global_log();
    #endif
  }

  unlock_listen_mutex();

  return 0;
} 

//========================

void ping_debug_client()
{
  char msg_buffer[100];
  memset(msg_buffer, 0, 100);
  snprintf(msg_buffer, 100, "ping debug client\n");
  send_message_to_client(msg_buffer, 100);
}