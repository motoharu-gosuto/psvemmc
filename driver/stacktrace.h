#pragma once

#include <psp2kern/kernel/threadmgr.h>

SceUID get_currentThreadId();
int get_current_thread_info(SceKernelThreadInfo* t_info);

typedef void(log_open_t)(void);
typedef void(log_close_t)(void);

int print_current_thread_info_global();
int stacktrace_from_here_global(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose);

int print_current_thread_info_sd();
int stacktrace_from_here_sd(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose);
