#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include<iostream>
#include<cstring>


int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  // for (int i=0;i<=2;i++){ // i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)
  //   RelCatEntry relCatBuffer;
  //   RelCacheTable::getRelCatEntry(i,&relCatBuffer);
  //     //get the relation catalog entry using RelCacheTable::getRelCatEntry()
  //     printf("Relation: %s\n", relCatBuffer.relName);
  //   for(int j=0;j<relCatBuffer.numAttrs;j++){
  //     //for j = 0 to numAttrs of the relation - 1
  //     AttrCatEntry attrCatBuffer;
  //     AttrCacheTable::getAttrCatEntry(i,j,&attrCatBuffer);
  //       //  get the attribute catalog entry for (rel-id i, attribute offset j)
  //         // in attrCatEntry using AttrCacheTable::getAttrCatEntry()
  //     const char *attrType=attrCatBuffer.attrType==NUMBER ? "NUM":"STR";
  //     printf("  %s: %s\n", attrCatBuffer.attrName, attrType);
  //   }
  // }
  // return 0;
  return FrontendInterface::handleFrontend(argc, argv);
}
