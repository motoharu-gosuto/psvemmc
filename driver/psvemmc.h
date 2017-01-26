#pragma once

int psvemmcInitialize(int bytesPerSector, int sectorsPerCluster);

int readSector(int sector, char* buffer);

int writeSector(int sector, char* buffer);

int readCluster(int cluster, char* buffer);

int writeCluster(int cluster, char* buffer);

int psvemmcDeinitialize();