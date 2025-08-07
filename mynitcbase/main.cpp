#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <cstring>
#include <iostream>

int main(int argc, char *argv[]) {
  Disk disk_run;
  
  // create objects for the relation catalog and attribute catalog
  RecBuffer relCatBuffer(RELCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  // load the headers of both the blocks into relCatHeader and attrCatHeader.
  // (we will implement these functions later)
  relCatBuffer.getHeader(&relCatHeader);
  
  /* i = 0 to total relation count */
  for (int i=0;i<relCatHeader.numEntries;i++) {

    Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

    relCatBuffer.getRecord(relCatRecord, i);

    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
    
    int curr = ATTRCAT_BLOCK;
    while (curr!=-1)
    {
      
      RecBuffer attrCatBuffer(curr);
      attrCatBuffer.getHeader(&attrCatHeader);
      
      /* j = 0 to number of entries in the attribute catalog */
      for (int j=0;j<attrCatHeader.numEntries;j++) {

        // declare attrCatRecord and load the attribute catalog entry into it
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRecord, j);

        /* attribute catalog entry corresponds to the current relation */
        if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0) {
          if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,"Students")==0 && strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class")==0)
          {
            strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Batch");
            attrCatBuffer.setRecord(attrCatRecord,j);
          }
          const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        }
      }
    
      curr = attrCatHeader.rblock;
    }
    
    printf("\n");
    
  }

  return 0;
}
