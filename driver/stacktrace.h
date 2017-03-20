#pragma once

#include <psp2kern/kernel/threadmgr.h>

void init_threadInfoMutex();
void deinit_threadInfoMutex();

SceUID get_currentThreadId();
int get_current_thread_info(SceKernelThreadInfo* t_info);

int print_current_thread_info_global();
int stacktrace_from_here_global(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose);

int print_current_thread_info_sd();
int stacktrace_from_here_sd(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose);
