#include <psp2kern/types.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/net/net.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <taihen.h>
#include <module.h>

#include "sector_api.h"

#include "glog.h"
#include "sdstor_log.h"
#include "dump.h"
#include "thread_test.h"
#include "net.h"
#include "mtable.h"
#include "stacktrace.h"

#include "hooks_vfs_funcs.h" 

char msg_buffer[MSG_SIZE];

tai_hook_ref_t sceVfsMount_hook_ref;
tai_hook_ref_t sceVfsAddVfs_hook_ref;
tai_hook_ref_t sceVfsUnmount_hook_ref;
tai_hook_ref_t sceVfsDeleteVfs_hook_ref;
tai_hook_ref_t sceVfsGetNewNode_hook_ref;

SceUID sceVfsMount_hook_id = -1;
SceUID sceVfsAddVfs_hook_id = -1;
SceUID sceVfsUnmount_hook_id = -1;
SceUID sceVfsDeleteVfs_hook_id = -1;
SceUID sceVfsGetNewNode_hook_id = -1;

int sceVfsMount_hook(vfs_mount_point_info_base* data)
{
  int res = TAI_CONTINUE(int, sceVfsMount_hook_ref, data);

  /*
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsMount");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsAddVfs_hook(vfs_add_data* data)
{
  int res = TAI_CONTINUE(int, sceVfsAddVfs_hook_ref, data);

  /*
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsAddVfs");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsUnmount_hook(vfs_unmount_data* data)
{
  int res = TAI_CONTINUE(int, sceVfsUnmount_hook_ref, data);

  /*
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsUnmount");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsDeleteVfs_hook(const char* name, void** deleted_node)
{
  int res = TAI_CONTINUE(int, sceVfsDeleteVfs_hook_ref, name, deleted_node);

  /*
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsDeleteVfs");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}

int sceVfsGetNewNode_hook(void* ctx, node_ops2* ops, int unused, vfs_node** node)
{
  int res = TAI_CONTINUE(int, sceVfsGetNewNode_hook_ref, ctx, ops, unused, node);

  /*
  memset(msg_buffer, 0, MSG_SIZE);
  snprintf(msg_buffer, MSG_SIZE, "called sceVfsGetNewNode");
  send_message_to_client(msg_buffer, MSG_SIZE);
  */

  return res;
}