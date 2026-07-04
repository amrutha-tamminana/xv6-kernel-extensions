#include "kernel/types.h"
#include "user/user.h"
int main(){
  printf("TEST: Syscall-heavy workload\n");
  int pid = getpid();
  for(int i = 0; i < 100; i++){
    pause(1);
  }
  struct mlfqinfo info;
  getmlfqinfo(pid, &info);
  printf("\nObserved behaviour:\n");
  printf("Process final level: %d\n", info.level);
  printf("Total syscalls: %d\n", info.total_syscalls);
  printf("\nExpected behaviour:\n");
  printf("- Interactive processes make many syscalls\n");
  printf("- If ΔS >= ΔT, they must NOT be demoted\n");
  printf("- Process should remain in Level 0 or Level 1\n");
  exit(0);
}