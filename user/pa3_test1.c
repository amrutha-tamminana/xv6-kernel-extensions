#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define PAGES 10
#define PGSIZE 4096
int main(){
  printf("\nPAGE FAULT TEST\n");
  printf("Expected constraints:\n");
  printf("page_faults >= %d\n", PAGES);
  printf("pages_evicted = 0\n");
  printf("pages_swapped_out = 0\n\n");
  char *buf = sbrk(PAGES * PGSIZE);
  for(int i = 0; i < PAGES; i++)
    buf[i * PGSIZE] = i;
  struct vmstats info;
  getvmstats(getpid(), &info);
  printf("\nActual values:\n");
  printf("page_faults      = %d\n", info.page_faults);
  printf("pages_evicted    = %d\n", info.pages_evicted);
  printf("pages_swapped_in = %d\n", info.pages_swapped_in);
  printf("pages_swapped_out= %d\n", info.pages_swapped_out);
  printf("resident_pages   = %d\n", info.resident_pages);
  exit(0);
}