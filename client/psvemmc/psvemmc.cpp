#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "EmmcClient.h"

#include <iostream>
#include <sstream>
#include <string>

int emmc_ping(SOCKET socket)
{
   command_0_request cmd0;
   cmd0.command = PSVEMMC_COMMAND_PING;

   int iResult = send(socket, (const char*)&cmd0, sizeof(command_0_request), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   command_0_response resp0;
   iResult = recv(socket, (char*)&resp0, sizeof(command_0_response), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   if(resp0.base.command != PSVEMMC_COMMAND_PING || resp0.base.vita_err < 0 || resp0.base.proxy_err < 0)
   {
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   if(strncmp(resp0.data, "emmcproxy", 10) != 0)
      return -1;

   return 0;
}

int emmc_init(SOCKET socket, int bytesPerSector, int sectorsPerCluster)
{
   command_2_request cmd2;
   cmd2.command = PSVEMMC_COMMAND_INIT;
   cmd2.bytesPerSector = bytesPerSector;
   cmd2.sectorsPerCluster = sectorsPerCluster;

   int iResult = send(socket, (const char*)&cmd2, sizeof(command_2_request), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   command_2_response resp2;
   iResult = recv(socket, (char*)&resp2, sizeof(command_2_response), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   if(resp2.base.command != PSVEMMC_COMMAND_INIT || resp2.base.vita_err < 0 || resp2.base.proxy_err < 0)
   {
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   return 0;
}

int emmc_deinit(SOCKET socket)
{
   command_3_request cmd3;
   cmd3.command = PSVEMMC_COMMAND_DEINIT;

   int iResult = send(socket, (const char*)&cmd3, sizeof(command_3_request), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   command_3_response resp3;
   iResult = recv(socket, (char*)&resp3, sizeof(command_3_response), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   if(resp3.base.command != PSVEMMC_COMMAND_DEINIT || resp3.base.vita_err < 0 || resp3.base.proxy_err < 0)
   {
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   return 0;
}

int dump_emmc(SOCKET emmc_socket)
{
   if(emmc_ping(emmc_socket) < 0)
      return -1;

   if(emmc_init(emmc_socket, 0x200, 1) < 0)
      return -1;

   //dump code here

   if(emmc_deinit(emmc_socket) < 0)
      return -1;

   return 0;
}

int main(int argc, char* argv[])
{
   SOCKET emmc_socket = 0;
   if(initialize_emmc_proxy_connection(emmc_socket) < 0)
      return 1;

   dump_emmc(emmc_socket);

   if(deinitialize_emmc_proxy_connection(emmc_socket) < 0)
      return 1;

	return 0;
}
