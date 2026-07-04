#include "kernel/types.h"
#include "user/user.h"
int main(){
  int pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }
  if(pid == 0){
    for(int i = 0; i < 50; i++){
      pause(1);
    }
    exit(0);
  }
  else{
    int parent_pid = getpid();
    for(long i = 0; i < 1000000000; i++);
    struct mlfqinfo parent_info;
    struct mlfqinfo child_info;
    if(getmlfqinfo(parent_pid, &parent_info) < 0){
      printf("TEST FAILED: parent info error\n");
      exit(1);
    }
    if(getmlfqinfo(pid, &child_info) < 0){
      printf("TEST FAILED: child info error\n");
      exit(1);
    }
    wait(0);
    printf("\nMixed workload snapshot\n");
    printf("\nParent (CPU-bound)\n");
    printf("PID: %d\n", parent_pid);
    printf("Level: %d\n", parent_info.level);
    printf("Times scheduled: %d\n", parent_info.times_scheduled);
    printf("\nChild (IO-bound)\n");
    printf("PID: %d\n", pid);
    printf("Level: %d\n", child_info.level);
    printf("Times scheduled: %d\n", child_info.times_scheduled);
    printf("\nExpected:\n");
    printf("- CPU-bound process eventually at Level 3\n");
    printf("- IO-bound process remains at Level 0 or 1\n");
    exit(0);
  }
}