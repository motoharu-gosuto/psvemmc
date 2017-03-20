#pragma once

#include <psp2kern/io/fcntl.h>

void init_global_log();
void deinit_global_log();

void FILE_GLOBAL_WRITE_LEN(char* msg);

void open_global_log();
void close_global_log(); 