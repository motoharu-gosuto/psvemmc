#pragma once

int dump_device_context_mem_blocks_1000(); 
int dump_device_context_mem_blocks_10000();
int dump_sdif_data();
int dump_sdstor_data();
int dump_exfatfs_data();
int dump_vfs_data();
int dump_vfs_node_info();
int call_proc_get_mount_data_C15B80(char* blockDeviceName);
int print_bytes(char* bytes, int size);