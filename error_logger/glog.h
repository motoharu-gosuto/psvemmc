#pragma once

#include <psp2/kernel/clib.h>

#define FILE_WRITE(f, msg) if(f >= 0) sceIoWrite(f, msg, sizeof(msg))
#define FILE_WRITE_LEN(f, msg) if(f >= 0) sceIoWrite(f, msg, sceClibStrnlen(msg, 256))

extern SceUID global_log_fd;

void open_global_log();

void close_global_log();