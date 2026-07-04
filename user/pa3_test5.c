#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define PAGES 180
#define PGSIZE 4096
int main(){
  printf("\nDATA INTEGRITY AFTER SWAP TEST\n");
  printf("Expected constraint:\n");
  printf("All pages must retain original values after swap-in\n\n");
  char *buf = sbrk(PAGES * PGSIZE);
  for(int i = 0; i < PAGES; i++)
    buf[i * PGSIZE] = i;
  for(int i = 0; i < PAGES; i++)
  {
    if(buf[i * PGSIZE] != i)
      printf("ERROR at page %d\n", i);
  }
  printf("Verification complete\n");
  exit(0);
}