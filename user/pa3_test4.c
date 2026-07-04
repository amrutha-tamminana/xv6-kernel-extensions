#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define PAGES 150
#define PGSIZE 4096
int main(){
  printf("\nSWAP IN TEST\n");
  printf("Expected constraints:\n");
  printf("pages_swapped_out > 0\n");
  printf("pages_swapped_in  > 0\n\n");
  char *buf = sbrk(PAGES * PGSIZE);
  for(int i = 0; i < PAGES; i++)
    buf[i * PGSIZE] = i;
  for(int i = 0; i < PAGES; i++)
    buf[i * PGSIZE]++;
  struct vmstats info;
  getvmstats(getpid(), &info);
  printf("\nActual values:\n");
  printf("pages_swapped_in  = %d\n", info.pages_swapped_in);
  printf("pages_swapped_out = %d\n", info.pages_swapped_out);
  exit(0);
}