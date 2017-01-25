#pragma once

int psvemmcIntialize(int bytesPerSector, int sectorsPerCluster);

int readSector(int sector, char* buffer, int nSectors);

int writeSector(int sector, char* buffer, int nSectors);

int readCluster(int cluster, char* buffer);

int writeCluster(int cluster, char* buffer);

int psvemmcDeinitialize();