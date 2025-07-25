#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <cstring>
#include <iostream>

int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  // StaticBuffer buffer;
  // OpenRelTable cache;
  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer, 7000);
  char message[]="hello";
  memcpy(buffer+20,message,6);
  Disk::writeBlock(buffer,7000);
  unsigned char buffer2[BLOCK_SIZE];
  char message2[6];
  Disk::readBlock(buffer2,7000);
  memcpy(message2,buffer2+20,6);
  std::cout << message2 << std::endl;
  //Exercise Q1
  unsigned char buffer3[BLOCK_SIZE];
  for (int block=0;block<4;block++) //Block Allocation Map is stored at Blocks 0 to 3
  {
    Disk::readBlock(buffer,block);
    std::cout << "BAM Block " << block << std::endl;
    for (int i=0;i<BLOCK_SIZE;i++)
      std::cout << (int)buffer[i] << " ";
    std::cout << std::endl;
  }

  //return FrontendInterface::handleFrontend(argc, argv);
  return 0;
}
