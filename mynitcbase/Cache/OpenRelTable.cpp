#include "OpenRelTable.h"

#include <cstring>
#include <stdlib.h>


OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
  RelCacheEntry attrCatRelCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &attrCatRelCacheEntry.relCatEntry);
  attrCatRelCacheEntry.recId.block = RELCAT_BLOCK;
  attrCatRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
  RelCacheTable::relCache[ATTRCAT_RELID] = (RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatRelCacheEntry;

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  
  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  
  AttrCacheEntry *head = nullptr, *prev = nullptr;
  
  for (int i=0;i<=5;i++)
  {
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry *newptr = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newptr->attrCatEntry);
    newptr->recId.slot = i;
    newptr->recId.block = ATTRCAT_BLOCK;
    newptr->next = nullptr;
    if (head == nullptr)
      head = newptr;
    else
      prev->next = newptr;
    prev = newptr;
  }

  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = head; /* head of the linked list */

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  
  
  AttrCacheEntry *headAttr = nullptr, *prevAttr = nullptr;
  for (int i=6;i<=11;i++)
  {
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry *newEntry = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newEntry->attrCatEntry);
    newEntry->recId.block = ATTRCAT_BLOCK;
    newEntry->recId.slot = i;
    newEntry->next = nullptr;
    if (!headAttr) headAttr = newEntry;
    else prevAttr->next = newEntry;
    prevAttr = newEntry;
  }
  
  AttrCacheTable::attrCache[ATTRCAT_RELID] = headAttr;
  
  AttrCacheEntry *headNext = nullptr, *prevNext = nullptr;
  relCatBlock.getRecord(relCatRecord, 2);
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = 2;
  RelCacheTable::relCache[2] = (RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[2]) = relCacheEntry;
  
  for (int i=12;i<12+relCacheEntry.relCatEntry.numAttrs;i++)
  {
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry *newEntry=(AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newEntry->attrCatEntry);
    newEntry->recId.block=ATTRCAT_BLOCK;
    newEntry->recId.slot=i;
    newEntry->next=NULL;
    if(!headNext) headNext=newEntry;
    else prevNext->next=newEntry;
    prevNext=newEntry;
  }
  
  AttrCacheTable::attrCache[2] = headNext;

}

OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor
  for (int i=0;i<MAX_OPEN;i++)
  {
    if (RelCacheTable::relCache[i])
    {
      free(RelCacheTable::relCache[i]);
      RelCacheTable::relCache[i]=nullptr;
    }
  }
  
  for (int i=0;i<MAX_OPEN;i++)
  {
    AttrCacheEntry* curr = AttrCacheTable::attrCache[i];
    while (curr)
    {
      AttrCacheEntry *temp = curr;
      curr = curr->next;
      free(temp);
    }
    AttrCacheTable::attrCache[i] = nullptr;
  }
  
}
