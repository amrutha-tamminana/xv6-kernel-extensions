#include "kernel/types.h"
#include "user/user.h"

int main(){
  printf("TEST: Global Priority Boost\n");
  int pid = fork();
  if(pid == 0){
    struct mlfqinfo info;
    getmlfqinfo(getpid(), &info);
    printf("Initial Level: %d\n", info.level);
    volatile long x = 0;
    for(long i = 0; i < 800000000; i++)
      x += i;
    for(int i = 0; i < 30; i++){
      pause(20);
      getmlfqinfo(getpid(), &info);
      printf("Level now: %d\n", info.level);
    }
    exit(0);
  }
  wait(0);
  exit(0);
}