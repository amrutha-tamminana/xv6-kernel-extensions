#ifndef RAID_H
#define RAID_H
#include "types.h"
#define NDISKS 4
// RAID modes
#define RAID0 0
#define RAID1 1
#define RAID5 5
extern int raid_mode;
extern int from_raid;
// RAID entry points used by vm.c swap code
void raid_read(uint blockno, char *data);
void raid_write(uint blockno, char *data);

#endif