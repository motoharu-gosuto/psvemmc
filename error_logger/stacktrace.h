#pragma once

#include <psp2/kernel/threadmgr.h>

SceUID get_currentThreadId();
int get_current_thread_info(SceKernelThreadInfo* t_info);

int print_current_thread_info_global_base(SceKernelThreadInfo* t_info);
int print_current_thread_info_global();

int stacktrace_global(volatile int* stackPtr, char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose, int print, uint32_t* addresses, uint32_t* addressNum);
int stacktrace_from_here_global(char* moduleNameSearch, int segIndexSearch, int stackSize, int verbose, int print, uint32_t* addresses, uint32_t* addressNum);
