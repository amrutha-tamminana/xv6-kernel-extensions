#include "kernel/types.h"
#include "user/user.h"
int main(){
  printf("TEST: CPU-bound workload\n");
  int pid = getpid();
  for(long i = 0; i < 1500000000; i++);
  struct mlfqinfo info;
  if(getmlfqinfo(pid, &info) < 0){
    printf("Error retrieving scheduling info\n");
    exit(1);
  }
  printf("\nObserved behaviour:\n");
  printf("Process final level: %d\n", info.level);
  printf("\nExpected behaviour:\n");
  printf("- CPU-bound processes consume their entire time slice\n");
  printf("- They should be demoted across queues\n");
  printf("- Eventually they should reach Level 3 (lowest priority)\n");
  exit(0);
}