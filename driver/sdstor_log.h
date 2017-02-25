#pragma once

#include <psp2kern/io/fcntl.h>

extern SceUID sdstor_dev_fs_log_fd;

void open_sdstor_dev_fs_log();

void close_sdstor_dev_fs_log();