#pragma once

#include <psp2kern/io/fcntl.h>

#define FILE_WRITE(f, msg) if(f >= 0) ksceIoWrite(f, msg, sizeof(msg))
#define FILE_WRITE_LEN(f, msg) if(f >= 0) ksceIoWrite(f, msg, strlen(msg))

void open_global_log();
void close_global_log(); 

extern SceUID global_log_fd;