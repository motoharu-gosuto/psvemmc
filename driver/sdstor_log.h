#pragma once

#include <psp2kern/io/fcntl.h>

void init_sdstor_log();
void deinit_sdstor_log();

void FILE_SDSTOR_WRITE_LEN(char* msg);

void open_sdstor_dev_fs_log();
void close_sdstor_dev_fs_log();