#pragma once

#include <psp2kern/kernel/threadmgr.h>

SceUID get_currentThreadId();
int get_current_thread_info(SceKernelThreadInfo* t_info);
int print_current_thread_info();
int stacktrace_from_here(char* moduleNameSearch, int segIndexSearch);
