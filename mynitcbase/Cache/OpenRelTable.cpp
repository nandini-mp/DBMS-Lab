#include "OpenRelTable.h"
#include <cstring>
#include<stdlib.h>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

AttrCacheEntry* create(int length){
  AttrCacheEntry *head=(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  AttrCacheEntry *tail=head;
  for(int i=1;i<length;i++){
    tail->next=(AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    tail=tail->next;
  }
  tail->next=nullptr;
  return head;
}

OpenRelTable::OpenRelTable() {
  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

 
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
    RelCacheEntry attrcatRelCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &attrcatRelCacheEntry.relCatEntry);
    attrcatRelCacheEntry.recId.block = RELCAT_BLOCK;
    attrcatRelCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
    RelCacheTable::relCache[ATTRCAT_RELID] = (RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrcatRelCacheEntry;

  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  AttrCacheEntry *head=nullptr,*prev=nullptr;

  for(int i=0;i<=5;i++){
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry *newptr=(struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newptr->attrCatEntry);
    newptr->recId.slot=i;
    newptr->recId.block=ATTRCAT_BLOCK;
    newptr->next=nullptr;
    if(head==nullptr){
      head=newptr;
    }
    else{
      prev->next=newptr;
    }
    prev=newptr;
  }

  AttrCacheTable::attrCache[RELCAT_RELID] =head; 

  AttrCacheEntry *headattr=nullptr,*prevattr=nullptr;
  for(int i=6;i<=11;i++){
    attrCatBlock.getRecord(attrCatRecord,i);
    AttrCacheEntry *newentry=(AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newentry->attrCatEntry);
    newentry->recId.block=ATTRCAT_BLOCK;
    newentry->recId.slot=i;
    newentry->next=nullptr;
    if(!headattr)
      headattr=newentry;
    else
      prevattr->next=newentry;
    prevattr=newentry;
  }
AttrCacheTable::attrCache[ATTRCAT_RELID]=headattr;


AttrCacheEntry *head3=nullptr,*prev3=nullptr;
relCatBlock.getRecord(relCatRecord,2);
RelCacheTable::recordToRelCatEntry(relCatRecord,&relCacheEntry.relCatEntry);
relCacheEntry.recId.block=RELCAT_BLOCK;
relCacheEntry.recId.slot=2;
RelCacheTable::relCache[2]=(RelCacheEntry*)malloc(sizeof(RelCacheEntry));
*(RelCacheTable::relCache[2])=relCacheEntry;

for(int i=12;i<12+relCacheEntry.relCatEntry.numAttrs;i++){
  attrCatBlock.getRecord(attrCatRecord,i);
  AttrCacheEntry *newentry=(AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord,&newentry->attrCatEntry);
  newentry->recId.block=ATTRCAT_BLOCK;
  newentry->recId.slot=i;
  newentry->next=NULL;
  if(!head3)head3=newentry;
  else prev3->next=newentry;
  prev3=newentry;
}
AttrCacheTable::attrCache[2]=head3;


// initialise all values in relCache and attrCache to be nullptr and all entries
  // in tableMetaInfo to be free

  // load the relation and attribute catalog into the relation cache (we did this already)

  // load the relation and attribute catalog into the attribute cache (we did this already)

  /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID

  tableMetaInfo[RELCAT_RELID].free=false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName,RELCAT_RELNAME);
  tableMetaInfo[ATTRCAT_RELID].free=false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName,ATTRCAT_RELNAME);
  for(int i=2;i<MAX_OPEN;i++){
    tableMetaInfo[i].free=true;
  }

}

OpenRelTable::~OpenRelTable() {

  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }

  for(int i=0;i<MAX_OPEN;i++){
    if(RelCacheTable::relCache[i]){
      free(RelCacheTable::relCache[i]);
      RelCacheTable::relCache[i]=nullptr;
    }
  }
  for(int i=0;i<MAX_OPEN;i++){
    AttrCacheEntry* curr=AttrCacheTable::attrCache[i];
    while(curr){
      AttrCacheEntry *temp=curr;
      curr=curr->next;
      free(temp);
    }
    AttrCacheTable::attrCache[i]=nullptr;
  }

}


int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
  for(int i=0;i<MAX_OPEN;i++){
  if(strcmp(relName,tableMetaInfo[i].relName)==0)
    return i;
  }
  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/

  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
  return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry() {
  for(int i=2;i<MAX_OPEN;i++){
    if(tableMetaInfo[i].free){
      return i;
    }
  }
  return E_CACHEFULL;
  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/

  // if found return the relation id, else return E_CACHEFULL.
}


int OpenRelTable::closeRel(int relId) {
  if (relId==0 || relId==1) {/* rel-id corresponds to relation catalog or attribute catalog*/
    return E_NOTPERMITTED;
  }

  if (relId>=MAX_OPEN || relId<0) {/* 0 <= relId < MAX_OPEN */
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free) {/* rel-id corresponds to a free slot*/
    return E_RELNOTOPEN;
  }

  AttrCacheEntry* head=AttrCacheTable::attrCache[relId];
  for(AttrCacheEntry *it=head,*next;it!=NULL;it=next){
    next=it->next;
    free(it);
  }
  tableMetaInfo[relId].free=true;
  free(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId]=nullptr;
  AttrCacheTable::attrCache[relId]=nullptr;

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function

  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr

  return SUCCESS;
}


int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  
  int relId=OpenRelTable::getRelId(relName);
  if(relId != E_RELNOTOPEN){/* the relation `relName` already has an entry in the Open Relation Table */
    // (checked using OpenRelTable::getRelId())
    return relId;
    // return that relation id;
  }
  relId=OpenRelTable::getFreeOpenRelTableEntry();
  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */

  if (relId==E_CACHEFULL){/* free slot not available */
    return E_CACHEFULL;
  }

  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

    RecBuffer relCatBlock(RELCAT_BLOCK);
    char relNameAttrConst[]=RELCAT_ATTR_RELNAME;
    Attribute relNameAttr;
    memcpy(relNameAttr.sVal,relName,ATTR_SIZE);
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId=BlockAccess::linearSearch(RELCAT_RELID,relNameAttrConst,relNameAttr,EQ);

  if (relcatRecId.block==-1 && relcatRecId.slot==-1) {/* relcatRecId == {-1, -1} */
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  RecBuffer relCatBuffer(relcatRecId.block);
  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBuffer.getRecord(relCatRecord,relcatRecId.slot);

  RelCatEntry relCatEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord,&relCatEntry);
  RelCacheTable::relCache[relId]=(RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  RelCacheTable::relCache[relId]->relCatEntry=relCatEntry;
  RelCacheTable::relCache[relId]->recId=relcatRecId;
  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */

  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  int numAttrs=relCatEntry.numAttrs;
  AttrCacheEntry* listHead=create(numAttrs);
  AttrCacheEntry* node=listHead;

  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  while(true){
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId=BlockAccess::linearSearch(ATTRCAT_RELID,(char*)ATTRCAT_ATTR_RELNAME,relNameAttr,EQ);
    if(attrcatRecId.block !=-1 && attrcatRecId.slot!=-1){
      Attribute attrcatRecord[ATTRCAT_NO_ATTRS];
      RecBuffer attrRecBuffer(attrcatRecId.block);
      attrRecBuffer.getRecord(attrcatRecord,attrcatRecId.slot);

      AttrCatEntry attrcatEntry;
      AttrCacheTable::recordToAttrCatEntry(attrcatRecord,&attrcatEntry);

      node->recId=attrcatRecId;
      node->attrCatEntry=attrcatEntry;
      node=node->next;

    }
    else break;
      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId]=listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/
  tableMetaInfo[relId].free=false;
  strcpy(tableMetaInfo[relId].relName,relCatEntry.relName);
  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.

  return relId;
}
