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

segment_info g_segList[SEG_LIST_SIZE]; 

int construct_module_range_table();
int sort_segment_table();
int find_in_segments(uintptr_t item);
int print_segment_table();