#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define PAGES 100
#define PGSIZE 4096
int main(){
  printf("\nLARGE ALLOCATION TEST\n");
  printf("Expected constraints:\n");
  printf("page_faults >= %d\n", PAGES);
  printf("resident_pages <= frame_table_size\n\n");
  char *buf = sbrk(PAGES * PGSIZE);
  for(int i = 0; i < PAGES; i++)
    buf[i * PGSIZE] = 1;
  struct vmstats info;
  getvmstats(getpid(), &info);
  printf("\nActual values:\n");
  printf("page_faults = %d\n", info.page_faults);
  printf("resident_pages = %d\n", info.resident_pages);
  exit(0);
}