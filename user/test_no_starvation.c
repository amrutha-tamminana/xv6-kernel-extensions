#include "kernel/types.h"
#include "user/user.h"
#define NPROC 3

int main(){
  printf("TEST: Starvation prevention\n");
  int pids[NPROC];
  for(int i = 0; i < NPROC; i++){
    int pid = fork();
    if(pid == 0){
      for(long j = 0; j < 1500000000; j++);
      exit(0);
    }
    pids[i] = pid;
  }
  pause(100);
  printf("\nObserved behaviour:\n");
  for(int i = 0; i < NPROC; i++){
    struct mlfqinfo info;
    if(getmlfqinfo(pids[i], &info) == 0){
      printf("PID %d scheduled %d times\n",
             pids[i],
             info.times_scheduled);
    }
  }
  for(int i = 0; i < NPROC; i++)
    wait(0);
  printf("\nExpected behaviour:\n");
  printf("- All processes must be scheduled at least once\n");
  printf("- Scheduling counts should be roughly similar\n");
  printf("- No process should starve\n");
  exit(0);
}