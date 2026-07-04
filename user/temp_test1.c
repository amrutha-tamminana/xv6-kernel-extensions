#include "kernel/types.h"
#include "user/user.h"

#define PAGES 128  // > MAX_FRAMES (128)

int
main()
{
  printf("PA4 disk swap test start\n");

  char *arr[PAGES];

  // Step 1: Allocate pages
  for(int i = 0; i < PAGES; i++){
    arr[i] = sbrk(4096);
    if(arr[i] == (char*)-1){
      printf("sbrk failed\n");
      exit(1);
    }
  }

  // Step 2: Write full pages (force allocation + swap-out later)
  for(int i = 0; i < PAGES; i++){
    for(int j = 0; j < 4096; j++){
      arr[i][j] = i;
    }
  }

  printf("After allocation and writes\n");

  // Step 3: Access first pages (forces swap-in)
  for(int i = 0; i < 32; i++){
    for(int j = 0; j < 4096; j++){
      if(arr[i][j] != i){
        printf("ERROR at page %d offset %d: got %d expected %d\n",
               i, j, arr[i][j], i);
        exit(1);
      }
    }
    printf("PAGE %d OK\n", i);
  }

  printf("After re-reading first pages\n");
  printf("TEST PASSED\n");

  exit(0);
}