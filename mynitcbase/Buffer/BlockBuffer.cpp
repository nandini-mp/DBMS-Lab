#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer(int blockNum) {
  // initialise this.blockNum with the argument
  this->blockNum=blockNum;
}

RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader(struct HeadInfo *head) {
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret!=SUCCESS)
    return ret;

  // read the block at this.blockNum into the buffer
  //Disk::readBlock(bufferPtr,this->blockNum);

  // populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries, bufferPtr + 16, 4);
  memcpy(&head->numAttrs, bufferPtr + 20, 4);
  memcpy(&head->rblock, bufferPtr + 12, 4);
  memcpy(&head->lblock, bufferPtr + 8, 4);

  return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;

  // get the header using this.getHeader() function
  BlockBuffer::getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  // read the block at this.blockNum into a buffer
  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret!=SUCCESS)
    return ret;

  //Disk::readBlock(bufferPtr,this->blockNum);

  /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
     - each record will have size attrCount * ATTR_SIZE
     - slotMap will be of size slotCount
  */
    
  int recordSize = attrCount * ATTR_SIZE;
  int slotOffset = (HEADER_SIZE + slotCount) + (recordSize*slotNum); /*header + slot map + slotNumber * size of a record to get to the respective slot number*/
  /* calculate buffer + offset */
  unsigned char *slotPointer = bufferPtr+slotOffset; /*buffer array starting address + offset*/

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize); /* memcpy(dest,src,size) */

  return SUCCESS;
}


int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
       
    int startAddress = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    
    if (startAddress!=SUCCESS) return startAddress;

    /* get the header of the block using the getHeader() function */
    
    HeadInfo head;
    this->getHeader(&head);

    // get number of attributes in the block.
    int attrCount = head.numAttrs;

    // get the number of slots in the block.
    int slotCount = head.numSlots;

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.
    if (slotNum<0 || slotNum>=slotCount)
      return E_OUTOFBOUND;

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
    
    int recordSize = attrCount*ATTR_SIZE;
    int offset = HEADER_SIZE + slotCount + (recordSize*slotNum);
    memcpy(bufferPtr+offset,rec,recordSize);

    // update dirty bit using setDirtyBit()
    StaticBuffer::setDirtyBit(this->blockNum);

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    // return SUCCESS
    return SUCCESS;
}


/* NOTE: This function will NOT check if the block has been initialised as a
   record or an index block. It will copy whatever content is there in that
   disk block to the buffer.
   Also ensure that all the methods accessing and updating the block's data
   should call the loadBlockAndGetBufferPtr() function before the access or
   update is done. This is because the block might not be present in the
   buffer due to LRU buffer replacement. So, it will need to be bought back
   to the buffer before any operations can be done.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char ** buffPtr) {
    /* check whether the block is already present in the buffer
       using StaticBuffer.getBufferNum() */
    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    // if present (!=E_BLOCKNOTINBUFFER),
        // set the timestamp of the corresponding buffer to 0 and increment the
        // timestamps of all other occupied buffers in BufferMetaInfo.
        
    if (bufferNum != E_BLOCKNOTINBUFFER)
    {
      StaticBuffer::metainfo[bufferNum].timeStamp=0;
      for (int i=0;i<BUFFER_CAPACITY;i++)
        if (i!=bufferNum && StaticBuffer::metainfo[i].free==false)
          StaticBuffer::metainfo[i].timeStamp++;
    
    }

    // else
        // get a free buffer using StaticBuffer.getFreeBuffer()

        // if the call returns E_OUTOFBOUND, return E_OUTOFBOUND here as
        // the blockNum is invalid

        // Read the block into the free buffer using readBlock()

    // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
    
    else
    {
      bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);
      
      if (bufferNum == E_OUTOFBOUND)
        return E_OUTOFBOUND;
      
      Disk::readBlock(StaticBuffer::blocks[bufferNum],this->blockNum);
    }
    
    *buffPtr = StaticBuffer::blocks[bufferNum];

    // return SUCCESS;
    return SUCCESS;
}



/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  BlockBuffer::getHeader(&head);

  /* number of slots in block from header */
  int slotCount = head.numSlots;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap,slotMapInBuffer,slotCount);

  return SUCCESS;
}


int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {

    int diff;
    // if attrType == STRING
    //     diff = strcmp(attr1.sval, attr2.sval)

    // else
    //     diff = attr1.nval - attr2.nval

    if (attrType == STRING)
      diff = strcmp(attr1.sVal,attr2.sVal);
    else
      diff = attr1.nVal - attr2.nVal;

    /*
    if diff > 0 then return 1
    if diff < 0 then return -1
    if diff = 0 then return 0
    */
    
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}

int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;
    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.

    if (ret!=SUCCESS) return ret;

    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )

    bufferHeader->blockType = head->blockType;
    bufferHeader->lblock = head->lblock;
    bufferHeader->rblock = head->rblock;
    bufferHeader->pblock = head->pblock;
    bufferHeader->numAttrs = head->numAttrs;
    bufferHeader->numEntries = head->numEntries;
    bufferHeader->numSlots = head->numSlots;

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed, return the error code

    ret = StaticBuffer::setDirtyBit(this->blockNum);
    if (ret!=SUCCESS) return ret;

    // return SUCCESS;
    return SUCCESS;
}


int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */

    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret!=SUCCESS) return ret;

    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;
    *((int32_t *)bufferPtr) = blockType;


    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
    StaticBuffer::blockAllocMap[this->blockNum] = blockType;

    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed
        // return the returned value from the call

    ret = StaticBuffer::setDirtyBit(this->blockNum);
    if (ret!=SUCCESS) return ret;

    // return SUCCESS
    return SUCCESS;
}


int BlockBuffer::getFreeBlock(int blockType){

    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    int blockNumber = -1;
    for (int i=0;i<DISK_BLOCKS;i++)
    {
      if (StaticBuffer::blockAllocMap[i] == UNUSED_BLK)
      {
        blockNumber = i;
        break;
      }
    }

    // if no block is free, return E_DISKFULL.
    if (blockNumber == -1) return E_DISKFULL;
    
    // set the object's blockNum to the block number of the free block.
    this->blockNum = blockNumber;

    // find a free buffer using StaticBuffer::getFreeBuffer().
    int buffNum = StaticBuffer::getFreeBuffer(blockNumber);

    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
    struct HeadInfo head;
    head.pblock = -1;
    head.lblock = -1;
    head.rblock = -1;
    head.numEntries = 0;
    head.numAttrs = 0;
    head.numSlots = 0;
    BlockBuffer::setHeader(&head);

    // update the block type of the block to the input block type using setBlockType().
    BlockBuffer::setBlockType(blockType);

    // return block number of the free block.
    return blockNumber;
}


BlockBuffer::BlockBuffer(char blockType){
    // allocate a block on the disk and a buffer in memory to hold the new block of
    // given type using getFreeBlock function and get the return error codes if any.

    int blocktype;
    if (blockType=='R')
      blocktype = REC;
    else if (blockType=='I')
      blocktype = IND_INTERNAL;
    else
      blocktype = IND_LEAF;

    // set the blockNum field of the object to that of the allocated block
    // number if the method returned a valid block number,
    // otherwise set the error code returned as the block number.

    int freeBlock = getFreeBlock(blocktype);
    this->blockNum = freeBlock;

    // (The caller must check if the constructor allocatted block successfully
    // by checking the value of block number field.)
}


int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if (ret!=SUCCESS) return ret;

    // get the header of the block using the getHeader() function
    HeadInfo head;
    BlockBuffer::getHeader(&head);

    /* the number of slots in the block */
    int numSlots = head.numSlots;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`

    memcpy(bufferPtr+HEADER_SIZE,slotMap,numSlots);
    
    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call
    ret = StaticBuffer::setDirtyBit(this->blockNum);
    if (ret!=SUCCESS) return ret;

    // return SUCCESS
    return SUCCESS;
}

int BlockBuffer::getBlockNum(){
    //return corresponding block number.
    return this->blockNum;
}