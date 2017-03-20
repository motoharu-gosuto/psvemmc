#pragma once

typedef struct range_pair
{
  uintptr_t start;
  uintptr_t end;
}range_pair;

typedef struct segment_info
{
  char moduleName[30];
  int seg;
  range_pair range;
}segment_info;

#define MOD_LIST_SIZE 0x80
#define SEG_LIST_SIZE MOD_LIST_SIZE * 4

extern SceUID g_modlistKernel[MOD_LIST_SIZE];
extern segment_info g_segListKernel[SEG_LIST_SIZE]; 

extern SceUID g_modlistUser[MOD_LIST_SIZE];
extern segment_info g_segListUser[SEG_LIST_SIZE]; 

extern int moduleListIsConstructedKernel;
extern int moduleListIsConstructedUser;

int construct_module_range_table(SceUID pid, int flags1, int flags2, SceUID* modlist, int32_t modlistLen, segment_info* segList, int32_t segListLen, int* moduleConstructured);
int sort_segment_table(segment_info* segList, int32_t segListSize, int* moduleConstructured);
int find_in_segments(segment_info* segList, int32_t segListSize, int* moduleConstructured, uintptr_t item);
int print_segment_table(segment_info* segList, int32_t segListSize, int* moduleConstructured);