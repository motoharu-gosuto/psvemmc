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

SceUID g_modlistKernel[MOD_LIST_SIZE];
segment_info g_segListKernel[SEG_LIST_SIZE]; 

SceUID g_modlistUser[MOD_LIST_SIZE];
segment_info g_segListUser[SEG_LIST_SIZE]; 

int moduleListIsConstructedKernel = 0;

int moduleListIsConstructedUser = 0;

int construct_module_range_table(SceUID pid, int flags1, int flags2, SceUID* modlist, int32_t modlistLen, segment_info* segList, int32_t segListLen, int* moduleConstructured)
{
  if(*moduleConstructured > 0)
    return 0;

  *moduleConstructured = 1;

  open_global_log();
  FILE_GLOBAL_WRITE_LEN("constructing module range table\n");
  close_global_log();

  size_t count = modlistLen;

  memset(modlist, 0, modlistLen * sizeof(SceUID));

  memset(segList, -1, segListLen * sizeof(segment_info));
  
  int ret = ksceKernelGetModuleList(pid, flags1, flags2, modlist, &count);
  if(ret < 0)
    return ret;

  open_global_log();
  snprintf(sprintfBuffer, 256, "number of modules: %d\n", count);
  FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
  close_global_log();

  int segInfoIndex = 0;

  for (int m = 0; m < count; m++) 
  {
    SceKernelModuleInfo minfo;
    minfo.size = sizeof(SceKernelModuleInfo);
    ret = ksceKernelGetModuleInfo(KERNEL_PID, modlist[m], &minfo);
    if(ret < 0)
      return ret;

    for (int s = 0; s < 4; ++s) 
    {
      if (minfo.segments[s].vaddr == NULL) 
        continue;
      
      memset(segList[segInfoIndex].moduleName, 0, 30);
      memcpy(segList[segInfoIndex].moduleName, minfo.module_name, 28);

      segList[segInfoIndex].seg = s;

      segList[segInfoIndex].range.start = (uintptr_t)minfo.segments[s].vaddr;
      segList[segInfoIndex].range.end = (uintptr_t)minfo.segments[s].vaddr + minfo.segments[s].memsz;

      segInfoIndex++;
    }
  }

  //set final item
  segList[segInfoIndex].range.start = -1;
  segList[segInfoIndex].range.end = -1;

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

int sort_segment_table(segment_info* segList, int32_t segListSize, int* moduleConstructured)
{
  if(*moduleConstructured == 0)
    return -1;

  qsort_imp(segList, segListSize, sizeof(segment_info), compare_segments);

  return 0; 
}

int find_in_segments(segment_info* segList, int32_t segListSize, int* moduleConstructured, uintptr_t item)
{
  if(*moduleConstructured == 0)
      return -1;

  for (int s = 0; s < segListSize; s++) 
  {
    if(item >= segList[s].range.start && item < segList[s].range.end)
      return s;
  }

  return -1;  
}

int print_segment_table(segment_info* segList, int32_t segListSize, int* moduleConstructured)
{
  if(*moduleConstructured == 0)
      return -1;

  for (int s = 0; s < segListSize; s++) 
  {
    if(segList[s].range.start == -1 || segList[s].range.end == -1)
      break;

    open_global_log();
    snprintf(sprintfBuffer, 256, "%s %d %08x %08x\n", segList[s].moduleName, segList[s].seg, segList[s].range.start, segList[s].range.end);
    FILE_GLOBAL_WRITE_LEN(sprintfBuffer);
    close_global_log();
  }

  return 0; 
} 
