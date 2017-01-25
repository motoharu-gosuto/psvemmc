#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "EmmcClient.h"

int dump_emmc(SOCKET emmc_socket)
{

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
