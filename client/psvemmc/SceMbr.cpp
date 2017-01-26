#include "SceMbr.h"

std::string partitionCodeToString(PartitionCodes c)
{
   switch(c)
   {
   case empty_c:
      return "empty";
   case eMMC:
      return "eMMC";
   case SLB2:
      return "SLB2";
   case os0:
      return "os0";
   case vs0:
      return "vs0";
   case vd0:
      return "vd0";
   case tm0:
      return "tm0";
   case ur0:
      return "ur0";
   case ux0:
      return "ux0";
   case gro0:
      return "gro0";
   case grw0:
      return "grw0";
   case ud0:
      return "ud0";
   case sa0:
      return "sa0";
   case cardsExt:
      return "cardsExt";
   case pd0:
      return "pd0";
   default:
      return "";
   }
}  

std::string PartitionTypeToString(PartitionTypes t)
{
   switch(t)
   {
   case empty_t:
      return "empty";
   case fat16:
      return "fat16";
   case exfat:
      return "exfat";
   case raw:
      return "raw";
   default:
      return "";
   }
}

int validateSceMbr(const MBR& mbr)
{
   if(strncmp(mbr.header, SCEHeader, 0x20) != 0)
   {
      std::cout << "Invalid header" << std::endl;
      return -1;
   }

   if(mbr.version != 3)
   {
      std::cout << "Invalid version" << std::endl;
      return -1;
   }

   if(mbr.signature != 0xAA55)
   {
      std::cout << "Invalid signature" << std::endl;
      return -1;
   }

   return 0;
}