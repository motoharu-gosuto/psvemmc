 
#pragma pack(push, 1)

struct sd_context_global;
struct cmd_input;

typedef struct cmd_input // size is 0x240
{
   uint32_t size; // 0x240
   uint32_t unk_4;
   uint32_t command;
   uint32_t argument;
   
   uint8_t data0[0x50];   
   
   struct cmd_input* next_cmd;
   uint32_t unk_64;
   uint32_t unk_68;
   uint32_t unk_6C;
   
   uint32_t unk_70;
   uint32_t unk_74;
   struct sd_context_global* gctx_ptr;
   uint32_t unk_7C;
   
   uint8_t data1[0x1C0];
} cmd_input;

typedef struct sd_context_data // size is 0xC0
{
    struct cmd_input* cmd_ptr;
    struct cmd_input* cmd_ptr_next;
    uint32_t unk_8;
    uint32_t unk_C;
    
    uint32_t sd_index;
    struct sd_context_part* ctx;
    uint32_t unk_18;
    uint32_t unk_1C;
       
    uint8_t data[0xA0];
} sd_context_data;

typedef struct sd_context_part // size is 0x398
{
   struct sd_context_global* gctx_ptr;
   
   uint8_t data[0x38C];
   
   void* unk_390;
   uint32_t unk_394;
} sd_context_part;

typedef struct sd_context_global // size is 0x24C0
{
    struct cmd_input commands[16];
    struct sd_context_data ctx_data;
} sd_context_global;

#pragma pack(pop)


sd_context_global* ksceSdifGetSdGlobalContextElement(int sd_ctx_idx);

sd_context_part* ksceSdifGetSdGlobalContextElement0Part(int sd_ctx_idx);
sd_context_part* ksceSdifGetSdGlobalContextElement1Part(int sd_ctx_idx);
sd_context_part* ksceSdifGetSdGlobalContextElement2Part(int sd_ctx_idx);

int ksceSdifGetGetCardInsertState1(int sd_ctx_idx);
int ksceSdifGetGetCardInsertState2(int sd_ctx_idx);

int ksceSdifInitializeSdDevice(int sd_ctx_index, sd_context_part** result);

#define SD_MODE_SINGLE_OP 1
//any othe number should be multiple op

//single mode means reading / writing single sector with CMD17 / CMD24
//multiple mode means reading / writing multiple sectors with CMD18 / CMD25

int ksceSdifReadSectorAsync(sd_context_part* ctx, int sector, char* buffer, int mode);
int ksceSdifReadSector(sd_context_part* ctx, int sector, char* buffer, int mode);
int ksceSdifWriteSectorAsync(sd_context_part* ctx, int sector, char* buffer, int mode);
int ksceSdifWriteSector(sd_context_part* ctx, int sector, char* buffer, int mode);

typedef struct output_23a4ef01
{
    int unk_0;
    int unk_4;
    int unk_8;
    int unk_C;
}output_23a4ef01;

int ksceSdifCopyCtx(sd_context_part* ctx, output_23a4ef01* unk0);

/*
int dump_sd_elements()
{
  sd_ctx_global* e0 = ksceSdifGetSdGlobalContextElement(0);
  sd_ctx_global* e1 = ksceSdifGetSdGlobalContextElement(1);
  sd_ctx_global* e2 = ksceSdifGetSdGlobalContextElement(2);
  sd_ctx_global* e3 = ksceSdifGetSdGlobalContextElement(3);
  
  sd_ctx_part* p1 = ksceSdifGetSdGlobalContextElement0Part(0);
  sd_ctx_part* p2 = ksceSdifGetSdGlobalContextElement1Part(1);
  sd_ctx_part* p3 = ksceSdifGetSdGlobalContextElement2Part(2);
  
  int is0_0 = ksceSdifGetGetCardInsertState1(0); //inserted
  int is0_1 = ksceSdifGetGetCardInsertState1(1); //not inserted
  int is0_2 = ksceSdifGetGetCardInsertState1(2); //inserted
  
  //int is1_0 = ksceSdifGetGetCardInsertState2(0);
  //int is1_1 = ksceSdifGetGetCardInsertState2(1);
  //int is1_2 = ksceSdifGetGetCardInsertState2(2);
  
  sd_ctx_part* einit0 = 0;
  int res0 = ksceSdifInitializeSdDevice(0, &einit0);
  //int is0_01 = ksceSdifGetGetCardInsertState1(0);
  
  int idx0 = *((int*)(((char*)e0) + 0x2400 + 0x10)); //get index of device
  int idx1 = *((int*)(((char*)e1) + 0x2400 + 0x10)); //get index of device
  int idx2 = *((int*)(((char*)e2) + 0x2400 + 0x10)); //get index of device
    
  sd_ctx_part* ctxpart0 = *((sd_ctx_part**)(((char*)e0) + 0x2400 + 0x14)); //check that context part pointer is same as if I get it directly without function
  sd_ctx_part* ctxpart1 = *((sd_ctx_part**)(((char*)e1) + 0x2400 + 0x14)); //check that context part pointer is same as if I get it directly without function
  sd_ctx_part* ctxpart2 = *((sd_ctx_part**)(((char*)e2) + 0x2400 + 0x14)); //check that context part pointer is same as if I get it directly without function
  
  //char buff[0x200];
  //memset(buff, 0, 0x200);
  
  //int res1 = ksceSdifReadSectorAsync(einit0, 0, buff, SD_MODE_SINGLE_OP);
  
  open_global_log();
  {
    char buffer[100];
    snprintf(buffer, 100, "%x %x %x %x\n", e0, e1, e2, e3);
    FILE_WRITE_LEN(global_log_fd, buffer);

    snprintf(buffer, 100, "%x %x %x\n", p1, p2, p3);
    FILE_WRITE_LEN(global_log_fd, buffer);
    
    //snprintf(buffer, 100, "%x %x %x\n", is0_0, is0_1, is0_2); // 1 0 1
    //FILE_WRITE_LEN(global_log_fd, buffer);
    
    //snprintf(buffer, 100, "%x %x %x\n", is1_0, is1_1, is1_2); // 1 0 1 
    //FILE_WRITE_LEN(global_log_fd, buffer);
    
    snprintf(buffer, 100, "%x %x %x\n", idx0, idx1, idx2);
    FILE_WRITE_LEN(global_log_fd, buffer);
    
    snprintf(buffer, 100, "%x %x %x\n", ctxpart0, ctxpart1, ctxpart2);
    FILE_WRITE_LEN(global_log_fd, buffer);
    
    //snprintf(buffer, 100, "%x %x %x\n", einit0, res0, is0_01);
    //FILE_WRITE_LEN(global_log_fd, buffer);
    
    snprintf(buffer, 100, "res0: %x \n", res0);
    FILE_WRITE_LEN(global_log_fd, buffer);
    
    snprintf(buffer, 100, "einit0: %x \n", einit0);
    FILE_WRITE_LEN(global_log_fd, buffer);
    
    //snprintf(buffer, 100, "res1: %x \n", res1);
    //FILE_WRITE_LEN(global_log_fd, buffer);
    
    //ksceIoWrite(global_log_fd, buff, 0x200);
    
  }
  close_global_log();
  
  return 0;
}
*/

#pragma pack(push, 1)

//these types is taken from my project psvcd
//https://github.com/motoharu-gosuto/psvcd
typedef struct FsSonyRoot
{
   uint8_t  SCEIid[32];
   uint32_t Unk0;
   uint32_t Unk1;
   uint64_t Unk2;
   uint64_t Unk3;
   uint64_t Unk4;
   uint64_t Unk5;
   uint64_t Unk6;
   uint32_t FsOffset;
   uint32_t VolumeLength;
   uint8_t BytesPerSectorShift; //not sure about this one TODO: not confirmed
   uint8_t unk70;
   uint8_t unk71;
   uint8_t unk72;
   uint32_t Unk8;
   uint32_t Unk9;
   uint32_t Unk10;
   uint32_t Unk11;
   uint32_t Unk12;
   uint8_t  BootCode[398];
   uint8_t  Signature[2];
} FsSonyRoot;

typedef struct VBR
{
   uint8_t    JumpBoot[3];
   uint8_t    FileSystemName[8];
   uint8_t    MustBeZero[53];
   uint64_t   PartitionOffset;
   uint64_t   VolumeLength;
   uint32_t    FatOffset; //sector address
   uint32_t    FatLength; // length in sectors
   uint32_t    ClusterHeapOffset; //sector address
   uint32_t    ClusterCount; //number of clusters
   uint32_t    RootDirFirstClust; //cluster address
   uint32_t    VolumeSerialNumber;
   uint8_t  FileSystemRevision2;
   uint8_t  FileSystemRevision1;
   uint8_t  VolumeFlags[2];
   uint8_t  BytesPerSectorShift;
   uint8_t  SectorsPerClusterShift;
   uint8_t  NumberOfFats;
   uint8_t  DriveSelect;
   uint8_t  PercentInUse;
   uint8_t  Reserved[7];
   uint8_t  BootCode[390];
   uint8_t  Signature[2];
} VBR;

#pragma pack(pop)

FsSonyRoot root_fs_sector;

VBR vbr_sector;


/*
int dump_sectors()
{
  sd_ctx_global* e0 = ksceSdifGetSdGlobalContextElement(0); //element from global context - size 0x24C0
  
  sd_ctx_part* p0 = ksceSdifGetSdGlobalContextElement0Part(0); //real sd context - at offset 0x2414 - size currently unknown
  
  sd_ctx_part* einit0 = 0;
  int res0 = ksceSdifInitializeSdDevice(0, &einit0); //initialize sd device
  if(res0 < 0)
    return res0;
  
  memset((void*)&root_fs_sector, 0, 0x200);
  int res1 = ksceSdifReadSectorAsync(einit0, 0, (void*)&root_fs_sector, SD_MODE_SINGLE_OP);
  
  open_global_log(); 
  ksceIoWrite(global_log_fd, (void*)&root_fs_sector, 0x200);
  close_global_log();
  
  if(strncmp("Sony Computer Entertainment Inc.", (char*)root_fs_sector.SCEIid, 0x20) != 0)
    return -1;

  memset((void*)&vbr_sector, 0, 0x200);
  int res2 = ksceSdifReadSectorAsync(einit0, root_fs_sector.FsOffset, (void*)&vbr_sector, SD_MODE_SINGLE_OP);
  if(res0 < 0)
    return res0;
  
  open_global_log(); 
  ksceIoWrite(global_log_fd, (void*)&vbr_sector, 0x200);
  close_global_log();
  
  return 0; 
}
*/

//char temp_sector[0x200];

char temp_sectors[0x200*10];

int dump_sectors2(int dev_index, int startIndex)
{
  sd_context_global* e0 = ksceSdifGetSdGlobalContextElement(dev_index); //element from global context - size 0x24C0
  
  sd_context_part* p0 = ksceSdifGetSdGlobalContextElement0Part(dev_index); //real sd context - at offset 0x2414 - size currently unknown
  
  sd_context_part* einit0 = 0;
  int res0 = ksceSdifInitializeSdDevice(dev_index, &einit0); //initialize sd device
  if(res0 < 0)
    return res0;
  
  /*
  output_23a4ef01 out;
  ksceSdifCopyCtx(einit0, &out);
  
  open_global_log();
  
  ksceIoWrite(global_log_fd, &out, sizeof(out));
  
  close_global_log();
  */
  
  /*
  open_global_log();
  
  ksceSdifReadSectorAsync(einit0, 0, temp_sectors, 10);
  ksceIoWrite(global_log_fd, temp_sectors, 0x200*10);
  
  close_global_log();
  */
  
  /*
  memset(temp_sector, 0, 0x200);
  
  open_global_log();
  
  int secIndex = startIndex;
  int endIndex = startIndex + 0x32000; //100MB
  while(secIndex < endIndex) 
  {
    int res1 = ksceSdifReadSectorAsync(einit0, secIndex, temp_sector, SD_MODE_SINGLE_OP);
    if(res1 < 0)
      break;
    
    ksceIoWrite(global_log_fd, temp_sector, 0x200);
    
    secIndex++;
  }
  
  close_global_log();
  */
  
  return 0;
}

int dump_sectors3()
{
  sd_context_global* e0 = ksceSdifGetSdGlobalContextElement(0);
  
  /*
  open_global_log();
  char buffer[100];
  snprintf(buffer, 100, "%x %x %x %x\n", sizeof(cmd_input), sizeof(sd_context_data), sizeof(sd_context_global), sizeof(sd_context_part));
  FILE_WRITE_LEN(global_log_fd, buffer);
  close_global_log();
  */
  
  /*
  {
    open_global_log();
    char buffer[100];
    snprintf(buffer, 100, "%x %x %x\n", e0[0].ctx_data.ctx->gctx_ptr, e0[1].ctx_data.ctx->gctx_ptr, e0[2].ctx_data.ctx->gctx_ptr);
    FILE_WRITE_LEN(global_log_fd, buffer);
    close_global_log();
  }
  
  {
    open_global_log();
    char buffer[100];
    snprintf(buffer, 100, "%x %x %x\n", e0[0].ctx_data.ctx, e0[1].ctx_data.ctx, e0[2].ctx_data.ctx);
    FILE_WRITE_LEN(global_log_fd, buffer);
    close_global_log();
  }
  */
  
  open_global_log();
  ksceIoWrite(global_log_fd, e0, ((int)e0[2].ctx_data.ctx + 0x400) - (int)e0);
  close_global_log();
  
  
  return 0;
}



//dump_sd_elements();
   
   //dump_sectors();
   
   //dump_sectors2(1, 0x32000 * 20); //ONLY USE INDEX 0 !
  
   dump_sectors3();