#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "EmmcClient.h"

#include <iostream>
#include <sstream>
#include <string>
#include <array>
#include <iomanip>
#include <vector>

#include "SceMbr.h"

//defalut size of sector for SD MMC protocol
#define SD_DEFAULT_SECTOR_SIZE 0x200

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

int emmc_read_sector(SOCKET socket, int sector, std::array<char, SD_DEFAULT_SECTOR_SIZE>& result)
{
   command_4_request cmd4;
   cmd4.command = PSVEMMC_COMMAND_READ_SECTOR;
   cmd4.sector = sector;

   int iResult = send(socket, (const char*)&cmd4, sizeof(command_4_request), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   command_4_response resp4;

   int bytesToReceive = sizeof(command_4_response);
   int bytesWereReceived = 0;
   command_4_response* respcpy = &resp4;

   while(bytesToReceive != bytesWereReceived)
   {
      int iResult = recv(socket, ((char*)respcpy) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
      if (iResult == SOCKET_ERROR) 
      {
         std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
         closesocket(socket);
         WSACleanup();
         return -1;
      }

      bytesWereReceived = bytesWereReceived + iResult;
   }

   if(resp4.base.command != PSVEMMC_COMMAND_READ_SECTOR || resp4.base.vita_err < 0 || resp4.base.proxy_err != 0)
   {
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   memcpy(result.data(), resp4.data, SD_DEFAULT_SECTOR_SIZE);

   return 0;
}

int emmc_read_cluster(SOCKET socket, int cluster, int expectedSize, char* data)
{
   command_5_request cmd5;
   cmd5.command = PSVEMMC_COMMAND_READ_CLUSTER;
   cmd5.cluster = cluster;

   int iResult = send(socket, (const char*)&cmd5, sizeof(command_5_request), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   command_5_response resp5;

   iResult = recv(socket, (char*)&resp5, sizeof(command_5_response), 0);
   if (iResult == SOCKET_ERROR) 
   {
      std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   int bytesToReceive = expectedSize;
   int bytesWereReceived = 0;
   char* respcpy = data;

   while(bytesToReceive != bytesWereReceived)
   {
      int iResult = recv(socket, ((char*)respcpy) + bytesWereReceived, bytesToReceive - bytesWereReceived, 0);
      if (iResult == SOCKET_ERROR) 
      {
         std::cout << "send failed with error: %d\n" << WSAGetLastError() << std::endl;
         closesocket(socket);
         WSACleanup();
         return -1;
      }

      bytesWereReceived = bytesWereReceived + iResult;
   }

   if(resp5.base.command != PSVEMMC_COMMAND_READ_CLUSTER || resp5.base.vita_err < 0 || resp5.base.proxy_err != 0)
   {
      closesocket(socket);
      WSACleanup();
      return -1;
   }

   return 0;
}

int dump_partition(SOCKET emmc_socket, const PartitionEntry& partition)
{
   //valid approach is to take sectorsPerCluster value from partition VBR (both fat16 and exfat have this info)

   const int sectorsPerCluster = 8;

   if(emmc_init(emmc_socket, SD_DEFAULT_SECTOR_SIZE, sectorsPerCluster) < 0)
      return -1;

   int nClustersToRead = partition.partitionSize / sectorsPerCluster;
   int nSectorsToRead = partition.partitionSize % sectorsPerCluster;

   int clusterOffset = partition.partitionOffset / sectorsPerCluster;

   std::vector<char> clusterData(SD_DEFAULT_SECTOR_SIZE * sectorsPerCluster);
   for(size_t i = 0; i < nClustersToRead; i++)
   {
      if(emmc_read_cluster(emmc_socket, clusterOffset + i, clusterData.size(), clusterData.data()) < 0)
         return -1;
   }

   int tailOffset =  partition.partitionOffset + nClustersToRead * sectorsPerCluster;

   std::array<char, SD_DEFAULT_SECTOR_SIZE> sectorData;
   for(size_t i = 0; i < nSectorsToRead; i++)
   {
      if(emmc_read_sector(emmc_socket, tailOffset + i, sectorData) < 0)
         return -1;
   }

   if(emmc_deinit(emmc_socket) < 0)
      return -1;
}

int dump_emmc(SOCKET emmc_socket)
{
   if(emmc_ping(emmc_socket) < 0)
      return -1;
   
   std::array<char, SD_DEFAULT_SECTOR_SIZE> mbrSector;
   if(emmc_read_sector(emmc_socket, 0, mbrSector) < 0)
      return -1;

   MBR mbr;
   memcpy(&mbr, mbrSector.data(), mbrSector.size());
   if(validateSceMbr(mbr) < 0)
      return -1;

   std::cout << "Available partitions: " << std::endl;

   for(size_t i = 0; i < NPartitions; i++)
   {
      const PartitionEntry& partition = mbr.partitions[i];

      if(partition.partitionType == empty_t)
         break;

      std::cout << std::hex << std::setfill('0') << std::setw(8) << partition.partitionOffset << " " 
                << std::hex << std::setfill('0') << std::setw(8) << partition.partitionSize << " "
                << PartitionTypeToString(partition.partitionType) << " " 
                << partitionCodeToString(partition.partitionCode) << std::endl;
   }

   dump_partition(emmc_socket, mbr.partitions[0]);

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
