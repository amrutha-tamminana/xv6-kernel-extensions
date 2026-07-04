#include "types.h"
#include "param.h"
#include "riscv.h"   
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "raid.h"
#include "vm.h"

extern void panic(char*);
struct buf* bread(uint, uint);
void bwrite(struct buf*);
void brelse(struct buf*);
void memmove(void*, const void*, uint);
int raid_mode = RAID5;   // default
int from_raid=0;
// helper
static int is_swap_block(uint blockno) {
  return (blockno >= SWAP_START_BLOCK);
}

static uint raid0_map(uint blockno) {
  uint logical = blockno - SWAP_START_BLOCK;
  int disk = logical % NDISKS;
  uint offset = logical / NDISKS;
  return SWAP_START_BLOCK + offset * NDISKS + disk;
}
void raid_read(uint blockno, char *data)
{
    
  if(!is_swap_block(blockno)){
    
    struct buf *b = bread(ROOTDEV, blockno);
    memmove(data, b->data, BSIZE);
    brelse(b);
    return;
  }


  if(raid_mode == RAID0){
    blockno = raid0_map(blockno);
  }

  else if(raid_mode == RAID1){
    uint half=SWAP_BLOCKS/2;
    uint logical = blockno - SWAP_START_BLOCK;
    if(logical<half){
        uint primary = SWAP_START_BLOCK + logical;
        blockno = primary;
    }
    else if(logical >= half){
            uint primary = SWAP_START_BLOCK + logical -half;
            blockno = primary;
    }
    }

  else if(raid_mode == RAID5){
    
    uint logical = blockno - SWAP_START_BLOCK;
    int stripe = logical / (NDISKS - 1);
    int index  = logical % (NDISKS - 1);
    int parity_disk = stripe % NDISKS;

    int disk = index;
    if(disk >= parity_disk) disk++;

    uint base = SWAP_START_BLOCK + stripe * NDISKS;
    blockno = base + disk;
  }

   // or mirror / data_block
  struct buf *b = bread(ROOTDEV, blockno);
  memmove(data, b->data, BSIZE);
  brelse(b);
}
void raid_write(uint blockno, char *data)
{
 
  if(!is_swap_block(blockno)){
   // or mirror / data_block
    struct buf *b = bread(ROOTDEV, blockno);
    memmove(b->data, data, BSIZE);
    bwrite(b);
    brelse(b);
    
    return;
  }
   from_raid=1;

  if(raid_mode == RAID0){
    blockno = raid0_map(blockno);
     // or mirror / data_block
  struct buf *bb = bread(ROOTDEV, blockno);
  memmove(bb->data, data, BSIZE);
  bwrite(bb);
  brelse(bb);
  }

  else if(raid_mode == RAID1){

  uint logical = blockno - SWAP_START_BLOCK;
 uint half=SWAP_BLOCKS/2;
 if(logical<half){
  // primary: direct mapping
  uint primary = SWAP_START_BLOCK + logical;

  // mirror: interleaved offset (round-robin style)
  uint mirror  = SWAP_START_BLOCK + ((logical + (SWAP_BLOCKS/2)) );
  // write primary
   // or mirror / data_block
  struct buf *pb = bread(ROOTDEV, primary);
  memmove(pb->data, data, BSIZE);
  bwrite(pb);
  brelse(pb);

  // write mirror
   // or mirror / data_block
  struct buf *mb = bread(ROOTDEV, mirror);
  memmove(mb->data, data, BSIZE);
  bwrite(mb);
  brelse(mb);
  from_raid=0;
  return;}
  else if(logical>=half){
    panic("out of swap space");
  }
}

 else if(raid_mode == RAID5){
    
  uint logical = blockno - SWAP_START_BLOCK;
  int stripe = logical / (NDISKS - 1);
  int index  = logical % (NDISKS - 1);
  int parity_disk = stripe % NDISKS;

  int disk = index;
  if(disk >= parity_disk) disk++;

  uint base = SWAP_START_BLOCK + stripe * NDISKS;
  uint data_block   = base + disk;
  uint parity_block = base + parity_disk;
  // bread data block ONCE
   // or mirror / data_block
   //save old data
  struct buf *db = bread(ROOTDEV, data_block);
  char old_data[BSIZE];
  memmove(old_data, db->data, BSIZE);
  // bread parity block ONCE
  //save old parity
  struct buf *pb = bread(ROOTDEV, parity_block);
  char parity_data[BSIZE];
  memmove(parity_data, pb->data, BSIZE);

  // compute new parity
  for(int i = 0; i < BSIZE; i++){
    parity_data[i] ^= old_data[i];
    parity_data[i] ^= data[i];
  }

  // write parity using SAME pb
  memmove(pb->data, parity_data, BSIZE);
  bwrite(pb);
  brelse(pb);

  // write data using SAME db
  memmove(db->data, data, BSIZE);
  bwrite(db);
  brelse(db);
   from_raid=0;
  return;   
}
  
}