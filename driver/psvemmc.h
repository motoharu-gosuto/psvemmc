#pragma once

int psvemmcInitialize(int bytesPerSector, int sectorsPerCluster);

int readSector(int sector, char* buffer);

int writeSector(int sector, char* buffer);

int readCluster(int cluster, char* buffer);

int writeCluster(int cluster, char* buffer);

int readSectorMs(int sector, char* buffer);

int writeSectorMs(int sector, char* buffer);

int readClusterMs(int cluster, char* buffer);

int writeClusterMs(int cluster, char* buffer);

int psvemmcDeinitialize();