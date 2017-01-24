#pragma once

int psvemmcIntialize(int sectorsPerCluster);

int readSectorAsync(int sector, char* buffer, int nSectors);

int writeSectorAsync(int sector, char* buffer, int nSectors);

int psvemmcDeinitialize();