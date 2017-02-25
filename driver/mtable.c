#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <module.h>

#include "mtable.h"
#include "qsort.h" //since there are no exports in kernel that are documented, had to use source of qsort

#include "glog.h"

char sprintfBuffer[256];

SceUID g_modlist[MOD_LIST_SIZE];

int moduleListIsConstructed = 0;

int construct_module_range_table()
{
  if(moduleListIsConstructed > 0)
    return 0;

  moduleListIsConstructed = 1;

  open_global_log();
  FILE_WRITE(global_log_fd, "constructing module range table\n");
  close_global_log();

  size_t count = MOD_LIST_SIZE;

  memset(g_modlist, 0, sizeof(g_modlist));

  memset(g_segList, -1, sizeof(g_segList));
  
  int ret = ksceKernelGetModuleList(KERNEL_PID, 0x80000001, 1, g_modlist, &count);
  if(ret < 0)
    return ret;

  open_global_log();
  snprintf(sprintfBuffer, 256, "number of modules: %d\n", count);
  FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  close_global_log();

  int segInfoIndex = 0;

  for (int m = 0; m < count; m++) 
  {
    SceKernelModuleInfo minfo;
    minfo.size = sizeof(SceKernelModuleInfo);
    ret = ksceKernelGetModuleInfo(KERNEL_PID, g_modlist[m], &minfo);
    if(ret < 0)
      return ret;

    for (int s = 0; s < 4; ++s) 
    {
      if (minfo.segments[s].vaddr == NULL) 
        continue;
      
      memset(g_segList[segInfoIndex].moduleName, 0, 30);
      memcpy(g_segList[segInfoIndex].moduleName, minfo.module_name, 28);

      g_segList[segInfoIndex].seg = s;

      g_segList[segInfoIndex].range.start = (uintptr_t)minfo.segments[s].vaddr;
      g_segList[segInfoIndex].range.end = (uintptr_t)minfo.segments[s].vaddr + minfo.segments[s].memsz;

      segInfoIndex++;
    }
  }

  //set final item
  g_segList[segInfoIndex].range.start = -1;
  g_segList[segInfoIndex].range.end = -1;

  return 0;
}

int compare_segments(const void *p, const void *q) 
{
    const segment_info* x = (const segment_info*)p;
    const segment_info* y = (const segment_info*)q;
    if (x->range.start < y->range.start)
        return -1;  // Return -1 if you want ascending, 1 if you want descending order. 
    else if (x->range.start > y->range.start)
        return 1;   // Return 1 if you want ascending, -1 if you want descending order. 

    return 0;
}

int sort_segment_table()
{
  if(moduleListIsConstructed == 0)
    return -1;

  qsort_imp(g_segList, SEG_LIST_SIZE, sizeof(segment_info), compare_segments);

  return 0; 
}

int find_in_segments(uintptr_t item)
{
  if(moduleListIsConstructed == 0)
      return -1;

  for (int s = 0; s < SEG_LIST_SIZE; s++) 
  {
    if(item >= g_segList[s].range.start && item < g_segList[s].range.end)
      return s;
  }

  return -1;  
}

int print_segment_table()
{
  if(moduleListIsConstructed == 0)
      return -1;

  for (int s = 0; s < SEG_LIST_SIZE; s++) 
  {
    if(g_segList[s].range.start == -1 || g_segList[s].range.end == -1)
      break;

    open_global_log();
    snprintf(sprintfBuffer, 256, "%s %d %08x %08x\n", g_segList[s].moduleName, g_segList[s].seg, g_segList[s].range.start, g_segList[s].range.end);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    close_global_log();
  }

  return 0; 
} 
