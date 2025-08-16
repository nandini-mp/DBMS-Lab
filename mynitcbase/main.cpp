#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <cstring>
#include <iostream>

int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;
  
  for (int i=0;i<=2;i++) //for i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)
  {
    // get the relation catalog entry using RelCacheTable::getRelCatEntry()
    RelCatEntry relCatBuffer;
    RelCacheTable::getRelCatEntry(i,&relCatBuffer);
    
    printf("Relation: %s\n", relCatBuffer.relName);

    // for j = 0 to numAttrs of the relation - 1
    for (int j=0;j<relCatBuffer.numAttrs;j++)
    {
      AttrCatEntry attrCatBuffer;
      AttrCacheTable::getAttrCatEntry(i,j,&attrCatBuffer);
      // get the attribute catalog entry for (rel-id i, attribute offset j) in attrCatEntry using AttrCacheTable::getAttrCatEntry()
      const char* attrType = attrCatBuffer.attrType == NUMBER ? "NUM" : "STR";
      printf("  %s: %s\n", attrCatBuffer.attrName, attrType);
    }
  }
  return 0;
}
