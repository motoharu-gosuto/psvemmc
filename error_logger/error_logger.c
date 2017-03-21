#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>

#include <psp2kern/kernel/sysmem.h>

#include <taihen.h>
#include <error.h>

#include <stdio.h>
#include <string.h>

#include "glog.h"
#include "mtable.h"
#include "stacktrace.h"

char sprintfBuffer[256];

//---------------

#define SceErrorUser_NID 0xD401318D
#define SceLibKernel_NID 0xCAE9ACE6

tai_hook_ref_t sceErrorGetExternalString_hook_ref;
SceUID sceErrorGetExternalString_hook_id = -1;

tai_hook_ref_t sceKernelCreateThread_hook_ref;
SceUID sceKernelCreateThread_hook_id = -1;

tai_hook_ref_t param_sfo_verifySpsfo_hook_ref;
SceUID param_sfo_verifySpsfo_hook_id = -1;

tai_hook_ref_t sceKernelLoadModule_hook_ref;
SceUID sceKernelLoadModule_hook_id = -1;

tai_hook_ref_t sceKernelLoadStartModule_hook_ref;
SceUID sceKernelLoadStartModule_hook_id = -1;

tai_hook_ref_t param_sfo_verifySpsfo_shell_hook_ref;
SceUID param_sfo_verifySpsfo_shell_hook_id = -1;

tai_hook_ref_t proc_gc_param_sfo_83F2CEA0_hook_ref;
SceUID proc_gc_param_sfo_83F2CEA0_hook_id = -1;

tai_hook_ref_t proc_gc_param_sfo_83F6B554_hook_ref;
SceUID proc_gc_param_sfo_83F6B554_hook_id = -1;

tai_hook_ref_t proc_gc_param_sfo_83F74B30_hook_ref;
SceUID proc_gc_param_sfo_83F74B30_hook_id = -1;

tai_hook_ref_t proc_83F258F8_hook_ref;
SceUID proc_83F258F8_hook_id = -1;

tai_hook_ref_t proc_83F27424_hook_ref;
SceUID proc_83F27424_hook_id = -1;

tai_hook_ref_t proc_83F2407A_hook_ref;
SceUID proc_83F2407A_hook_id = -1;

tai_hook_ref_t proc_83F25592_hook_ref;
SceUID proc_83F25592_hook_id = -1;

tai_hook_ref_t proc_83F24534_hook_ref;
SceUID proc_83F24534_hook_id = -1;

tai_hook_ref_t proc_83F24D96_hook_ref;
SceUID proc_83F24D96_hook_id = -1;

tai_hook_ref_t proc_8430F028_hook_ref;
SceUID proc_8430F028_hook_id = -1;

tai_hook_ref_t queue_worker_entry2_834DED94_hook_ref;
SceUID queue_worker_entry2_834DED94_hook_id = -1;

void* g_dest_user;
int g_unk;
int g_sceErrorGetExternalString_hook_called = 0;
SceKernelThreadInfo* g_t_info;
uint32_t g_addresses[256];
uint32_t g_adress_num;

uint32_t g_stack_dump[256];

uint32_t* g_stackPtr;

//Warning
//Hooks should not use logging directly in the hook - this is recomendation
//by some unknown reason sometimes this can cause segfaults or some other undefined behavior
//that is why there is a separate thread that does the logging
//TODO:
//it would be better if mutex is added to the listener thread to avoid corruption of data that is logged if listener is not fast enough

int sceErrorGetExternalString_hook(void* dest_user, int unk)
{
  int res = TAI_CONTINUE(int, sceErrorGetExternalString_hook_ref, dest_user, unk);

  g_dest_user = dest_user;
  g_unk = unk;
  g_sceErrorGetExternalString_hook_called = 1;
  
  //does not work
  //get_current_thread_info(g_t_info);
  
  //does not work
  //stacktrace_from_here_global("", 0, 20, 1, g_addresses, &g_adress_num);
  
  //does not work
  //sceClibMemcpy(g_stack_dump, &res, 1 * sizeof(uint32_t));
  
  //does not work
  /*
  g_stackPtr = (uint32_t*)&res;
  g_stack_dump[0] = *g_stackPtr;
  g_stackPtr++;
  g_stack_dump[1] = *g_stackPtr;
  g_stackPtr++;
  */
  
  //does not work
  //ksceKernelMemcpyKernelToUser((uintptr_t)g_stack_dump, &res, 2 * sizeof(uint32_t));
  
  return res;
}

SceUID g_sceKernelCreateThreadMutex;

SceUID sceKernelCreateThread_hook(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, int cpuAffinityMask, const SceKernelThreadOptParam *option)
{
  SceUID res = TAI_CONTINUE(SceUID, sceKernelCreateThread_hook_ref, name, entry, initPriority, stackSize, attr, cpuAffinityMask, option);

  /*
  uint32_t timeout = 0;
  sceKernelLockMutex(g_sceKernelCreateThreadMutex, 1, &timeout);

  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called sceKernelCreateThread_hook: name: %s res: %08x\n", name, res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();

  sceKernelUnlockMutex(g_sceKernelCreateThreadMutex, 1);
  */

  return res;
}

int param_sfo_verifySpsfo_hook_called = 0;
char* g_param_sfo_verifySpsfo_hook_file = 0;
char g_param_sfo_verifySpsfo_hook_path[1024];
int g_param_sfo_verifySpsfo_hook_res = 0;

int param_sfo_verifySpsfo_hook(char *file, void *ctx, int verifySpsfo)
{
  int res = TAI_CONTINUE(int, param_sfo_verifySpsfo_hook_ref, file, ctx, verifySpsfo);

  int len = sceClibStrnlen(file, 1024);
  sceClibMemcpy(g_param_sfo_verifySpsfo_hook_path, file, len);
  g_param_sfo_verifySpsfo_hook_path[len] = 0;

  g_param_sfo_verifySpsfo_hook_file = file;
  g_param_sfo_verifySpsfo_hook_res = res;
  param_sfo_verifySpsfo_hook_called = 1;

  return res;
}

SceUID sceKernelLoadModule_hook(char *path, int flags, SceKernelLMOption *option)
{
  SceUID res = TAI_CONTINUE(SceUID, sceKernelLoadModule_hook_ref, path, flags, option);

  /*
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called sceKernelLoadModule_hook:\npath: %s res: %08x\n", path, res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  */

  return res;
}

int sceKernelLoadStartModule_hook_called = 0;
SceSize g_args_ld = 0;
void* g_argp_ld = 0;
int g_flags_ld = 0;

SceUID sceKernelLoadStartModule_hook(char *path, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status)
{
  /*
  open_global_log();
  {
    //sceClibSnprintf(sprintfBuffer, 256, "called sceKernelLoadStartModule:\npath: %s res: %08x\n", path, 0);
    //FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    //FILE_WRITE(global_log_fd, "called sceKernelLoadStartModule_hook\n");
  }
  close_global_log();
  */

  SceUID res = TAI_CONTINUE(SceUID, sceKernelLoadStartModule_hook_ref, path, args, argp, flags, option, status);

  /*
  g_args_ld = args;
  g_argp_ld = argp;
  g_flags_ld = flags;
  sceKernelLoadStartModule_hook_called = 1;
  */

  return res;
}

typedef struct copy_str_pair
{
  char* data;
  int32_t len;
}copy_str_pair;

int param_sfo_verifySpsfo_shell_hook_called = 0;
char* g_param_sfo_verifySpsfo_shell_hook_data = 0;
char g_param_sfo_verifySpsfo_shell_hook_path[1024];
int g_param_sfo_verifySpsfo_shell_hook_res = 0;

int param_sfo_verifySpsfo_shell_hook(void *ctx, copy_str_pair *path_ctx)
{
  int res = TAI_CONTINUE(int, param_sfo_verifySpsfo_shell_hook_ref, ctx, path_ctx);

  sceClibMemcpy(g_param_sfo_verifySpsfo_shell_hook_path, path_ctx->data, path_ctx->len);
  g_param_sfo_verifySpsfo_shell_hook_path[path_ctx->len] = 0;

  g_param_sfo_verifySpsfo_shell_hook_data = path_ctx->data;
  g_param_sfo_verifySpsfo_shell_hook_res = res;
  param_sfo_verifySpsfo_shell_hook_called = 1;

  return res;
}

int proc_gc_param_sfo_83F2CEA0_hook_called = 0;
int g_proc_gc_param_sfo_83F2CEA0_hook_unk = 0;
int g_proc_gc_param_sfo_83F2CEA0_hook_res = 0;

int proc_gc_param_sfo_83F2CEA0_hook(int unk)
{
  int res = TAI_CONTINUE(int, proc_gc_param_sfo_83F2CEA0_hook_ref, unk);

  g_proc_gc_param_sfo_83F2CEA0_hook_unk = unk;
  g_proc_gc_param_sfo_83F2CEA0_hook_res = res;
  proc_gc_param_sfo_83F2CEA0_hook_called = 1;

  return res;
}

int proc_gc_param_sfo_83F6B554_hook(int unk) // not called
{
  int res = TAI_CONTINUE(int, proc_gc_param_sfo_83F6B554_hook_ref, unk);

  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_gc_param_sfo_83F6B554_hook:\narg: %08x res: %08x\n", unk, res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();

  return res;
}

int proc_gc_param_sfo_83F74B30_hook(int unk) // not called
{
  int res = TAI_CONTINUE(int, proc_gc_param_sfo_83F74B30_hook_ref, unk);

  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_gc_param_sfo_83F74B30_hook:\narg: %08x res: %08x\n", unk, res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();

  return res;
}

//---------

typedef struct input_83F23578
{
  uint32_t unk_0;
  uint32_t unk_4;
  uint32_t unk_8;
  uint32_t unk_C;
  uint32_t unk_10;
  uint32_t unk_14;
} input_83F23578;

int proc_83F2407A_hook_called = 0;
int g_proc_83F2407A_hook_unk0 = 0;
int g_proc_83F2407A_hook_unk1 = 0;
int g_proc_83F2407A_hook_unk2 = 0;
int g_proc_83F2407A_hook_res = 0;

int proc_83F2407A_hook(int unk0, int unk1, int unk2) //works
{
  int res = TAI_CONTINUE(int, proc_83F2407A_hook_ref, unk0, unk1, unk2);

  /*
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_83F2407A_hook:\nres: %08x\n", res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  */

  g_proc_83F2407A_hook_unk0 = unk0;
  g_proc_83F2407A_hook_unk1 = unk1;
  g_proc_83F2407A_hook_unk2 = unk2;
  g_proc_83F2407A_hook_res = res;
  proc_83F2407A_hook_called = 1;

  return res;
}

int proc_83F25592_hook_called = 0;
int g_proc_83F25592_hook_arg0 = 0;
int g_proc_83F25592_hook_arg1 = 0;
int g_proc_83F25592_hook_res = 0;
int g_proc_83F25592_hook_ctx_14 = 0;

int proc_83F25592_hook(input_83F23578 *ctx, int unk1) //works
{
  g_proc_83F25592_hook_ctx_14 = ctx->unk_14;
  
  int res = TAI_CONTINUE(int, proc_83F25592_hook_ref, ctx, unk1);

  /*
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_83F25592_hook:\nres: %08x\n", res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  */

  g_proc_83F25592_hook_arg0 = (int)ctx;
  g_proc_83F25592_hook_arg1 = unk1;
  g_proc_83F25592_hook_res = res;
  proc_83F25592_hook_called = 1;

  return res;
}

int proc_83F24534_hook_called = 0;
int g_proc_83F24534_hook_arg0 = 0;
int g_proc_83F24534_hook_arg1 = 0;
int g_proc_83F24534_hook_arg2 = 0;
int g_proc_83F24534_hook_arg3 = 0;
int g_proc_83F24534_hook_arg4 = 0;
int g_proc_83F24534_hook_res = 0;
int g_proc_83F24534_hook_ctx_14 = 0;

int proc_83F24534_hook(input_83F23578 *ctx, int unk1, int unk2, int unk3, int unk4)
{
  g_proc_83F24534_hook_ctx_14 = ctx->unk_14;
  
  int res = TAI_CONTINUE(int, proc_83F24534_hook_ref, ctx, unk1, unk2, unk3, unk4);

  /*
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_83F24534_hook:\nres: %08x\n", res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  */

  g_proc_83F24534_hook_arg0 = (int)ctx;
  g_proc_83F24534_hook_arg1 = unk1;
  g_proc_83F24534_hook_arg2 = unk2;
  g_proc_83F24534_hook_arg3 = unk3;
  g_proc_83F24534_hook_arg4 = unk4;
  g_proc_83F24534_hook_res = res;
  proc_83F24534_hook_called = 1;

  return res;
}

int proc_83F24D96_hook_called = 0;
int g_proc_83F24D96_hook_arg0 = 0;
int g_proc_83F24D96_hook_arg1 = 0;
int g_proc_83F24D96_hook_arg2 = 0;
int g_proc_83F24D96_hook_arg3 = 0;
int g_proc_83F24D96_hook_arg4 = 0;
int g_proc_83F24D96_hook_res = 0;
int g_proc_83F24D96_hook_ctx_14 = 0;

int proc_83F24D96_hook(input_83F23578 *ctx, int unk1, int unk2, int unk3, int unk4)
{
  g_proc_83F24D96_hook_ctx_14 = ctx->unk_14;
  
  int res = TAI_CONTINUE(int, proc_83F24D96_hook_ref, ctx, unk1, unk2, unk3, unk4);

  /*
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_83F24D96_hook:\nres: %08x\n", res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  */

  g_proc_83F24D96_hook_arg0 = (int)ctx;
  g_proc_83F24D96_hook_arg1 = unk1;
  g_proc_83F24D96_hook_arg2 = unk2;
  g_proc_83F24D96_hook_arg3 = unk3;
  g_proc_83F24D96_hook_arg4 = unk4;
  g_proc_83F24D96_hook_res = res;
  proc_83F24D96_hook_called = 1;

  return res;
}

//--------------------

int proc_83F258F8_hook_called = 0;
int g_proc_83F258F8_hook_arg0 = 0;
int g_proc_83F258F8_hook_arg1 = 0;
int g_proc_83F258F8_hook_arg2 = 0;
int g_proc_83F258F8_hook_res = 0;
int g_proc_83F258F8_hook_ctx_14 = 0;

//volatile int g_proc_83F258F8_stack[10];

int proc_83F258F8_hook(int unk0, input_83F23578 *ctx, int unk2) //works
{
  g_proc_83F258F8_hook_arg0 = unk0;
  g_proc_83F258F8_hook_arg1 = (int)ctx;
  g_proc_83F258F8_hook_arg2 = unk2;
  g_proc_83F258F8_hook_ctx_14 = ctx->unk_14;
  
  int res = TAI_CONTINUE(int, proc_83F258F8_hook_ref, unk0, ctx, unk2);

  //sceClibMemcpy((int*)g_proc_83F258F8_stack, &res, 10 * sizeof(int));

  /*
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_83F258F8_hook:\narg0: %08x arg1: %08x arg2: %08x res: %08x\n", unk0, unk1, unk2, res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  */

  g_proc_83F258F8_hook_res = res;
  proc_83F258F8_hook_called = 1;

  return res;
}

int proc_83F27424_hook_called = 0;
int g_proc_83F27424_hook_arg0 = 0;
int g_proc_83F27424_hook_arg1 = 0;
int g_proc_83F27424_hook_res = 0;

int proc_83F27424_hook(int unk0, int unk1)  //not sure if it is called so probably does not work
{
  int res = TAI_CONTINUE(int, proc_83F27424_hook_ref, unk0, unk1);

  /*
  open_global_log();
  {
    sceClibSnprintf(sprintfBuffer, 256, "called proc_83F27424_hook:\narg0: %08x arg1: %08x res: %08x\n", unk0, unk1, res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
  }
  close_global_log();
  */

  proc_83F27424_hook_called = 1;
  g_proc_83F27424_hook_arg0 = unk0;
  g_proc_83F27424_hook_arg1 = unk1;
  g_proc_83F27424_hook_res = res;

  return res;
}

//---------------

struct queue_worker_ctx;
struct sema_res_ctx;
struct sema_atomic_ctx;

typedef int queue_worker_entry(struct queue_worker_ctx *ctx);

typedef struct queue_worker_call_ctx
{
  queue_worker_entry *entry0;
  queue_worker_entry *entry1;
  queue_worker_entry *entry2;
} queue_worker_call_ctx;

typedef int shell_queue_callback(struct queue_worker_ctx *ctx, struct input_83F23578 *arg);

typedef struct queue_worker_ctx
{
  struct queue_worker_call_ctx *call_ctx;
  SceUID thid;
  uint8_t unk_8;
  uint8_t unk_9;
  uint8_t unk_A;
  uint8_t unk_B;
  SceUID semaid;
  struct sema_res_ctx *sema_res;
  struct sema_atomic_ctx *sema_atomic;
  uint32_t unk_18;
  shell_queue_callback *unk_1C;
  uint32_t unk_20;
  uint32_t unk_24;
  input_83F23578 *queue_callback_arg;
} queue_worker_ctx;

typedef struct queue_worker_arg
{
  struct queue_worker_ctx *ctx;
} queue_worker_arg;

typedef int sema_res_worker_entry(struct sema_res_ctx *ctx);

typedef struct sema_res_worker_ctx
{
  uint32_t unk_0;
  uint32_t unk_4;
  uint32_t unk_8;
  sema_res_worker_entry *entry;
} sema_res_worker_ctx;

typedef struct sema_res_ctx
{
  struct sema_res_worker_ctx *ctx;
} sema_res_ctx;

struct sema_atomic_ctx
{
  uint32_t unk_0;
  uint32_t atomic_ctr1;
  uint32_t atomic_ctr2;
  sema_res_worker_entry *entry;
};
  
int proc_8430F028_hook_called = 0;
queue_worker_ctx* g_proc_8430F028_hook_ctx = 0;
input_83F23578* g_proc_8430F028_hook_arg = 0;
uint32_t g_proc_8430F028_hook_ctx_14 = 0;
int g_proc_8430F028_hook_res = 0;

//volatile int g_proc_8430F028_hook_stack[4];

int proc_8430F028_hook(queue_worker_ctx *ctx)
{
  g_proc_8430F028_hook_ctx = ctx;
  g_proc_8430F028_hook_arg = ctx->queue_callback_arg;
  g_proc_8430F028_hook_ctx_14 = ctx->queue_callback_arg->unk_14;

  int res = TAI_CONTINUE(int, proc_8430F028_hook_ref, ctx);

  //sceClibMemcpy((int*)g_proc_8430F028_hook_stack, &res, 4 * sizeof(int));

  g_proc_8430F028_hook_res = res;

  proc_8430F028_hook_called = 1;

  return res;
}

//---------------

int queue_worker_entry2_834DED94_hook_called = 0;
queue_worker_ctx* g_queue_worker_entry2_834DED94_hook_ctx = 0;
input_83F23578* g_queue_worker_entry2_834DED94_hook_arg = 0;
uint32_t g_queue_worker_entry2_834DED94_hook_ctx_14 = 0;
int g_queue_worker_entry2_834DED94_hook_res = 0;

int queue_worker_entry2_834DED94_hook(queue_worker_ctx *ctx)
{
  int res = TAI_CONTINUE(int, queue_worker_entry2_834DED94_hook_ref, ctx);

  g_queue_worker_entry2_834DED94_hook_ctx = ctx;
  g_queue_worker_entry2_834DED94_hook_arg = ctx->queue_callback_arg;
  g_queue_worker_entry2_834DED94_hook_ctx_14 = ctx->queue_callback_arg->unk_14;
  g_queue_worker_entry2_834DED94_hook_res = res;

  queue_worker_entry2_834DED94_hook_called = 1;
 
  return res;
}

//---------------

int print_error(int err)
{
  //open_global_log();
  
  switch(err)
  {
    case TAI_ERROR_SYSTEM:
      FILE_WRITE(global_log_fd, "TAI_ERROR_SYSTEM\n");
      break;
    case TAI_ERROR_MEMORY:
      FILE_WRITE(global_log_fd, "TAI_ERROR_MEMORY\n");
      break;
    case TAI_ERROR_NOT_FOUND:
      FILE_WRITE(global_log_fd, "TAI_ERROR_NOT_FOUND\n");
      break;
    case TAI_ERROR_INVALID_ARGS:
      FILE_WRITE(global_log_fd, "TAI_ERROR_INVALID_ARGS\n");
      break;
    case TAI_ERROR_INVALID_KERNEL_ADDR:
      FILE_WRITE(global_log_fd, "TAI_ERROR_INVALID_KERNEL_ADDR\n");
      break;
    case TAI_ERROR_PATCH_EXISTS:
      FILE_WRITE(global_log_fd, "TAI_ERROR_PATCH_EXISTS\n");
      break;
    case TAI_ERROR_HOOK_ERROR:
      FILE_WRITE(global_log_fd, "TAI_ERROR_HOOK_ERROR\n");
      break;
    case TAI_ERROR_NOT_IMPLEMENTED:
      FILE_WRITE(global_log_fd, "TAI_ERROR_NOT_IMPLEMENTED\n");
      break;
    case TAI_ERROR_USER_MEMORY:
      FILE_WRITE(global_log_fd, "TAI_ERROR_USER_MEMORY\n");
      break;
    case TAI_ERROR_NOT_ALLOWED:
      FILE_WRITE(global_log_fd, "TAI_ERROR_NOT_ALLOWED\n");
      break;
    case TAI_ERROR_STUB_NOT_RESOLVED:
      FILE_WRITE(global_log_fd, "TAI_ERROR_STUB_NOT_RESOLVED\n");
      break;
    case TAI_ERROR_INVALID_MODULE:
      FILE_WRITE(global_log_fd, "TAI_ERROR_INVALID_MODULE\n");
      break;
  }
  
  //close_global_log();

  return 0;
}

int ListenerThread(SceSize args, void *argp)
{
  open_global_log();
  FILE_WRITE(global_log_fd, "entered ListenerThread\n");
  close_global_log();
  
  while(1)
  {
    sceKernelDelayThread(100);
    
    /*
    if(g_sceErrorGetExternalString_hook_called == 1)
    {
      open_global_log();
      {
	      sceClibSnprintf(sprintfBuffer, 256, "called sceErrorGetExternalString_hook: dest_user: %p unk: %08x\n", g_dest_user, g_unk);
	      FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();
      
      print_current_thread_info_global_base(g_t_info);
      
      g_sceErrorGetExternalString_hook_called = 0;
      break;
    }
    */
    
    /*
    if(sceKernelLoadStartModule_hook_called == 1)
    {
      open_global_log();
      {
	      sceClibSnprintf(sprintfBuffer, 256, "called sceKernelLoadStartModule_hook: args: %08x argp: %p flags: %08x\n", g_args_ld, g_argp_ld, g_flags_ld);
	      FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      sceKernelLoadStartModule_hook_called = 0;
      break;
    }
    */

    /*
    tai_module_info_t gcip_info;
    gcip_info.size = sizeof(tai_module_info_t);
    int gcip_info_res = taiGetModuleInfo("SceGameCardInstallerPlugin", &gcip_info);
    if(gcip_info_res >= 0) 
    {
      open_global_log();
      FILE_WRITE(global_log_fd, "found SceGameCardInstallerPlugin\n");
      close_global_log();
    }
    else
    {
      open_global_log();
      FILE_WRITE(global_log_fd, "failed to find SceGameCardInstallerPlugin\n");
      close_global_log();
    }
    */

    if(proc_83F258F8_hook_called == 1)
    {
      open_global_log();
      {
	      sceClibSnprintf(sprintfBuffer, 256, "called proc_83F258F8_hook:\narg0: %08x arg1: %08x arg2: %08x ctx14: %08x res: %08x\n", g_proc_83F258F8_hook_arg0, g_proc_83F258F8_hook_arg1, g_proc_83F258F8_hook_arg2, g_proc_83F258F8_hook_ctx_14, g_proc_83F258F8_hook_res);
	      FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();
      
      /*
      int segidx = find_in_segments(g_proc_83F258F8_hook_ctx_14);
      if(segidx >= 0)
      {
        open_global_log();
        {
          sceClibSnprintf(sprintfBuffer, 256, "ctx14 is at: %s %d %08x %08x\n", g_segList[segidx].moduleName, g_segList[segidx].seg, g_segList[segidx].range.start, (g_proc_83F258F8_hook_ctx_14 - g_segList[segidx].range.start));
          FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
        }
        close_global_log();
      }
      */

      /*
      open_global_log();
      FILE_WRITE(global_log_fd, "stacktrace:\n");
      close_global_log(); 

      uint32_t addresses[10];
      uint32_t addressNum = 0;
      stacktrace_global(g_proc_83F258F8_stack, "", 0, 10, 1, addresses, &addressNum);
      */

      proc_83F258F8_hook_called = 0;
    }

    if(proc_83F27424_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called proc_83F27424_hook:\narg0: %08x arg1: %08x res: %08x\n", g_proc_83F27424_hook_arg0, g_proc_83F27424_hook_arg1, g_proc_83F27424_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      proc_83F27424_hook_called = 0;
    }

    if(proc_83F2407A_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called proc_83F2407A_hook:\n arg0: %08x arg1: %08x arg2: %08x res: %08x\n", g_proc_83F2407A_hook_unk0, g_proc_83F2407A_hook_unk1, g_proc_83F2407A_hook_unk2, g_proc_83F2407A_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      proc_83F2407A_hook_called = 0; 
    }

    if(proc_83F25592_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called proc_83F25592_hook:\n arg0: %08x arg1: %08x ctx14: %08x res: %08x\n", 
          g_proc_83F25592_hook_arg0, g_proc_83F25592_hook_arg1, g_proc_83F25592_hook_ctx_14, g_proc_83F25592_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      proc_83F25592_hook_called = 0;
    }

    if(proc_83F24534_hook_called == 1)
    {      
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called proc_83F24534_hook:\n arg0: %08x arg1: %08x arg2: %08x arg3: %08x arg4: %08x ctx14: %08x res: %08x\n", 
          g_proc_83F24534_hook_arg0, g_proc_83F24534_hook_arg1, g_proc_83F24534_hook_arg2, g_proc_83F24534_hook_arg3, g_proc_83F24534_hook_arg4, g_proc_83F24534_hook_ctx_14, g_proc_83F24534_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      proc_83F24534_hook_called = 0;
    }

    if(proc_83F24D96_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called proc_83F24D96_hook:\n arg0: %08x arg1: %08x arg2: %08x arg3: %08x arg4: %08x ctx14: %08x res: %08x\n", 
          g_proc_83F24D96_hook_arg0, g_proc_83F24D96_hook_arg1, g_proc_83F24D96_hook_arg2, g_proc_83F24D96_hook_arg3, g_proc_83F24D96_hook_arg4, g_proc_83F24D96_hook_ctx_14, g_proc_83F24D96_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      proc_83F24D96_hook_called = 0;
    }

    if(proc_8430F028_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called proc_8430F028_hook:\nctx: %08x arg: %08x ctx14: %08x res: %08x\n", 
          g_proc_8430F028_hook_ctx, g_proc_8430F028_hook_arg, g_proc_8430F028_hook_ctx_14, g_proc_8430F028_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      /*
      open_global_log();
      FILE_WRITE(global_log_fd, "stacktrace:\n");
      close_global_log(); 

      uint32_t addresses[4];
      uint32_t addressNum = 0;
      stacktrace_global(g_proc_8430F028_hook_stack, "", 0, 4, 1, addresses, &addressNum);
      */

      proc_8430F028_hook_called = 0;
    }

    if(queue_worker_entry2_834DED94_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called queue_worker_entry2_834DED94_hook:\nctx: %08x arg: %08x ctx14: %08x res: %08x\n", 
          g_queue_worker_entry2_834DED94_hook_ctx, g_queue_worker_entry2_834DED94_hook_arg, g_queue_worker_entry2_834DED94_hook_ctx_14, g_queue_worker_entry2_834DED94_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      queue_worker_entry2_834DED94_hook_called = 0;
    }

    if(param_sfo_verifySpsfo_shell_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called param_sfo_verifySpsfo_shell_hook:\npath: %s res: %08x\n", g_param_sfo_verifySpsfo_shell_hook_path, g_param_sfo_verifySpsfo_shell_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      param_sfo_verifySpsfo_shell_hook_called = 0;
    }

    if(proc_gc_param_sfo_83F2CEA0_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called proc_gc_param_sfo_83F2CEA0_hook:\narg: %08x res: %08x\n", g_proc_gc_param_sfo_83F2CEA0_hook_unk, g_proc_gc_param_sfo_83F2CEA0_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      proc_gc_param_sfo_83F2CEA0_hook_called = 0;
    }

    if(param_sfo_verifySpsfo_hook_called == 1)
    {
      open_global_log();
      {
        sceClibSnprintf(sprintfBuffer, 256, "called param_sfo_verifySpsfo_hook:\npath: %s res: %08x\n", g_param_sfo_verifySpsfo_hook_path, g_param_sfo_verifySpsfo_hook_res);
        FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
      }
      close_global_log();

      param_sfo_verifySpsfo_hook_called = 0;
    }

  }

  open_global_log();
  FILE_WRITE(global_log_fd, "exited ListenerThread\n");
  close_global_log();
  
  return 0;
}

SceUID g_listenerThread;

int initialize_all_hooks()
{
  g_sceKernelCreateThreadMutex = sceKernelCreateMutex("CreateThreadMutex", 0, 0, 0);
  if(g_sceKernelCreateThreadMutex < 0)
  {
    open_global_log();
    FILE_WRITE(global_log_fd, "failed to create CreateThreadMutex\n");
    close_global_log();
  }

  tai_module_info_t sceshell_info;
  sceshell_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfo("SceShell", &sceshell_info) >= 0) 
  {
    param_sfo_verifySpsfo_shell_hook_id = taiHookFunctionOffset(&param_sfo_verifySpsfo_shell_hook_ref, sceshell_info.modid, 0, 0x32EEC, 1, param_sfo_verifySpsfo_shell_hook);

    proc_gc_param_sfo_83F2CEA0_hook_id = taiHookFunctionOffset(&proc_gc_param_sfo_83F2CEA0_hook_ref, sceshell_info.modid, 0, 0x2C660, 1, proc_gc_param_sfo_83F2CEA0_hook);
    proc_gc_param_sfo_83F6B554_hook_id = taiHookFunctionOffset(&proc_gc_param_sfo_83F6B554_hook_ref, sceshell_info.modid, 0, 0x6AD14, 1, proc_gc_param_sfo_83F6B554_hook);
    proc_gc_param_sfo_83F74B30_hook_id = taiHookFunctionOffset(&proc_gc_param_sfo_83F74B30_hook_ref, sceshell_info.modid, 0, 0x742F0, 1, proc_gc_param_sfo_83F74B30_hook);

    proc_83F258F8_hook_id = taiHookFunctionOffset(&proc_83F258F8_hook_ref, sceshell_info.modid, 0, 0x250B8, 1, proc_83F258F8_hook);
    //proc_83F27424_hook_id = taiHookFunctionOffset(&proc_83F27424_hook_ref, sceshell_info.modid, 0, 0x26BE4, 1, proc_83F27424_hook);

    //proc_83F2407A_hook_id = taiHookFunctionOffset(&proc_83F2407A_hook_ref, sceshell_info.modid, 0, 0x2383A, 1, proc_83F2407A_hook);
    //proc_83F25592_hook_id = taiHookFunctionOffset(&proc_83F25592_hook_ref, sceshell_info.modid, 0, 0x24D52, 1, proc_83F25592_hook);
    //proc_83F24534_hook_id = taiHookFunctionOffset(&proc_83F24534_hook_ref, sceshell_info.modid, 0, 0x23CF4, 1, proc_83F24534_hook);
    //proc_83F24D96_hook_id = taiHookFunctionOffset(&proc_83F24D96_hook_ref, sceshell_info.modid, 0, 0x24556, 1, proc_83F24D96_hook);

    proc_8430F028_hook_id = taiHookFunctionOffset(&proc_8430F028_hook_ref, sceshell_info.modid, 0, 0x40E7E8, 1, proc_8430F028_hook);
  }

  tai_module_info_t paf_info;
  paf_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfo("ScePaf", &paf_info) >= 0)
  {
    //looks like this hook causes segfaults
    //queue_worker_entry2_834DED94_hook_id = taiHookFunctionOffset(&queue_worker_entry2_834DED94_hook_ref, paf_info.modid, 0, 0x1DE364, 1, queue_worker_entry2_834DED94_hook);
  }

  tai_module_info_t driverUser_info;
  driverUser_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfo("SceDriverUser", &driverUser_info) >= 0) 
  {
    //does not work
    //sceErrorGetExternalString_hook_id = taiHookFunctionExport(&sceErrorGetExternalString_hook_ref, "SceDriverUser", SceErrorUser_NID, 0xA4DE5B69, sceErrorGetExternalString_hook);
    
    //sceErrorGetExternalString_hook_id = taiHookFunctionImport(&sceErrorGetExternalString_hook_ref, "SceShell", SceErrorUser_NID, 0xA4DE5B69, sceErrorGetExternalString_hook);
  }

  tai_module_info_t libKernel_info;
  libKernel_info.size = sizeof(tai_module_info_t);
  if (taiGetModuleInfo("SceLibKernel", &libKernel_info) >= 0) 
  {
    //sceKernelCreateThread_hook_id = taiHookFunctionImport(&sceKernelCreateThread_hook_ref, "SceShell", SceLibKernel_NID, 0xC5C11EE7, sceKernelCreateThread_hook);

    //sceKernelLoadModule_hook_id = taiHookFunctionExport(&sceKernelLoadModule_hook_ref, "SceLibKernel", SceLibKernel_NID, 0xBBE82155, sceKernelLoadModule_hook);

    //sceKernelLoadStartModule_hook_id = taiHookFunctionExport(&sceKernelLoadStartModule_hook_ref, "SceLibKernel", SceLibKernel_NID, 0x2DCC4AFA, sceKernelLoadStartModule_hook);
  }

  tai_module_info_t gcip_info;
  gcip_info.size = sizeof(tai_module_info_t);
  int gcip_info_res = taiGetModuleInfo("SceGameCardInstallerPlugin", &gcip_info);
  if(gcip_info_res >= 0) 
  {
    param_sfo_verifySpsfo_hook_id = taiHookFunctionOffset(&param_sfo_verifySpsfo_hook_ref, gcip_info.modid, 0, 0x7892, 1, param_sfo_verifySpsfo_hook);
  }
  else
  {
    open_global_log();
    sceClibSnprintf(sprintfBuffer, 256, "failed to get SceGameCardInstallerPlugin module info : %08x\n", gcip_info_res);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(gcip_info_res);
    close_global_log();
  }

  open_global_log();
  
  if(sceErrorGetExternalString_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceErrorGetExternalString_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set sceErrorGetExternalString_hook: %08x\n", sceErrorGetExternalString_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(sceErrorGetExternalString_hook_id);
  }

  if(sceKernelCreateThread_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceKernelCreateThread_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set sceKernelCreateThread_hook: %08x\n", sceKernelCreateThread_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(sceKernelCreateThread_hook_id);
  }

  if(param_sfo_verifySpsfo_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set param_sfo_verifySpsfo_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set param_sfo_verifySpsfo_hook: %08x\n", param_sfo_verifySpsfo_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(param_sfo_verifySpsfo_hook_id);
  }

  if(sceKernelLoadModule_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceKernelLoadModule_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set sceKernelLoadModule_hook: %08x\n", sceKernelLoadModule_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(sceKernelLoadModule_hook_id);
  }

  if(sceKernelLoadStartModule_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set sceKernelLoadStartModule_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set sceKernelLoadStartModule_hook: %08x\n", sceKernelLoadStartModule_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(sceKernelLoadStartModule_hook_id);
  }

  if(param_sfo_verifySpsfo_shell_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set param_sfo_verifySpsfo_shell_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set param_sfo_verifySpsfo_shell_hook: %08x\n", param_sfo_verifySpsfo_shell_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(param_sfo_verifySpsfo_shell_hook_id);
  }

  if(proc_gc_param_sfo_83F2CEA0_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_gc_param_sfo_83F2CEA0_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_gc_param_sfo_83F2CEA0_hook: %08x\n", proc_gc_param_sfo_83F2CEA0_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_gc_param_sfo_83F2CEA0_hook_id);
  }

  if(proc_gc_param_sfo_83F6B554_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_gc_param_sfo_83F6B554_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_gc_param_sfo_83F6B554_hook: %08x\n", proc_gc_param_sfo_83F6B554_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_gc_param_sfo_83F6B554_hook_id);
  }

  if(proc_gc_param_sfo_83F74B30_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_gc_param_sfo_83F74B30_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_gc_param_sfo_83F74B30_hook: %08x\n", proc_gc_param_sfo_83F74B30_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_gc_param_sfo_83F74B30_hook_id);
  }

  if(proc_83F258F8_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_83F258F8_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_83F258F8_hook: %08x\n", proc_83F258F8_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_83F258F8_hook_id);
  }

  if(proc_83F27424_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_83F27424_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_83F27424_hook: %08x\n", proc_83F27424_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_83F27424_hook_id);
  }

  if(proc_83F2407A_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_83F2407A_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_83F2407A_hook: %08x\n", proc_83F2407A_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_83F2407A_hook_id);
  }

  if(proc_83F25592_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_83F25592_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_83F25592_hook: %08x\n", proc_83F25592_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_83F25592_hook_id);
  }

  if(proc_83F24534_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_83F24534_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_83F24534_hook: %08x\n", proc_83F24534_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_83F24534_hook_id);
  }

  if(proc_83F24D96_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_83F24D96_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_83F24D96_hook: %08x\n", proc_83F24D96_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_83F24D96_hook_id);
  }

  if(proc_8430F028_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set proc_8430F028_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set proc_8430F028_hook: %08x\n", proc_8430F028_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(proc_8430F028_hook_id);
  }

  if(queue_worker_entry2_834DED94_hook_id >= 0)
  {
    FILE_WRITE(global_log_fd, "set queue_worker_entry2_834DED94_hook\n");
  }
  else
  {
    sceClibSnprintf(sprintfBuffer, 256, "failed to set queue_worker_entry2_834DED94_hook: %08x\n", queue_worker_entry2_834DED94_hook_id);
    FILE_WRITE_LEN(global_log_fd, sprintfBuffer);
    print_error(queue_worker_entry2_834DED94_hook_id);
  }

  close_global_log();
  
  //--------------------------
  
  construct_module_range_table();
  sort_segment_table();
  //print_segment_table();
  
  //--------------------------
  
  g_listenerThread = sceKernelCreateThread("ListenerThread", ListenerThread, 0x64, 0x1000, 0, 0, 0);
  
  if(g_listenerThread >= 0)
  {
    sceKernelStartThread(g_listenerThread, 0, 0);
  }
  else
  {
    open_global_log();
    FILE_WRITE(global_log_fd, "failed to create ListenerThread\n");
    close_global_log();
  }

  return 0;
}

int deinitialize_all_hooks()
{
  if(sceErrorGetExternalString_hook_id >= 0)
     taiHookRelease(sceErrorGetExternalString_hook_id, sceErrorGetExternalString_hook_ref);

  if(sceKernelCreateThread_hook_id >= 0)
     taiHookRelease(sceKernelCreateThread_hook_id, sceKernelCreateThread_hook_ref);

  if(param_sfo_verifySpsfo_hook_id >= 0)
    taiHookRelease(param_sfo_verifySpsfo_hook_id, param_sfo_verifySpsfo_hook_ref);

  if(sceKernelLoadModule_hook_id >= 0)
    taiHookRelease(sceKernelLoadModule_hook_id, sceKernelLoadModule_hook_ref);

  if(sceKernelLoadStartModule_hook_id >= 0)
    taiHookRelease(sceKernelLoadStartModule_hook_id, sceKernelLoadStartModule_hook_ref);

  if(param_sfo_verifySpsfo_shell_hook_id >= 0)
    taiHookRelease(param_sfo_verifySpsfo_shell_hook_id, param_sfo_verifySpsfo_shell_hook_ref);

  if(proc_gc_param_sfo_83F2CEA0_hook_id >= 0)
    taiHookRelease(proc_gc_param_sfo_83F2CEA0_hook_id, proc_gc_param_sfo_83F2CEA0_hook_ref);

  if(proc_gc_param_sfo_83F6B554_hook_id >= 0)
    taiHookRelease(proc_gc_param_sfo_83F6B554_hook_id, proc_gc_param_sfo_83F6B554_hook_ref);

  if(proc_gc_param_sfo_83F74B30_hook_id >= 0)
    taiHookRelease(proc_gc_param_sfo_83F74B30_hook_id, proc_gc_param_sfo_83F74B30_hook_ref);

  if(proc_83F258F8_hook_id >= 0)
    taiHookRelease(proc_83F258F8_hook_id, proc_83F258F8_hook_ref);

  if(proc_83F27424_hook_id >= 0)
    taiHookRelease(proc_83F27424_hook_id, proc_83F27424_hook_ref);

  if(proc_83F2407A_hook_id >= 0)
    taiHookRelease(proc_83F2407A_hook_id, proc_83F2407A_hook_ref);

  if(proc_83F25592_hook_id >= 0)
    taiHookRelease(proc_83F25592_hook_id, proc_83F25592_hook_ref);

  if(proc_83F24D96_hook_id >= 0)
    taiHookRelease(proc_83F24D96_hook_id, proc_83F24D96_hook_ref);

  if(proc_83F24534_hook_id >=0)
    taiHookRelease(proc_83F24534_hook_id, proc_83F24534_hook_ref);

  if(proc_8430F028_hook_id >= 0)
    taiHookRelease(proc_8430F028_hook_id, proc_8430F028_hook_ref);

  if(queue_worker_entry2_834DED94_hook_id >= 0)
    taiHookRelease(queue_worker_entry2_834DED94_hook_id, queue_worker_entry2_834DED94_hook_ref); 
    
  int stat = 0;
  SceUInt timeout = 0;
  sceKernelWaitThreadEnd(g_listenerThread, &stat, &timeout);
  
  sceKernelDeleteThread(g_listenerThread);

  sceKernelDeleteMutex(g_sceKernelCreateThreadMutex);

  return 0;
}

//---------------

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) 
{
  open_global_log();
  FILE_WRITE(global_log_fd, "Starting error_logger suprx!\n");
  close_global_log(); 

  initialize_all_hooks();
  
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) 
{

  deinitialize_all_hooks();
  
  return SCE_KERNEL_STOP_SUCCESS;
}
 
