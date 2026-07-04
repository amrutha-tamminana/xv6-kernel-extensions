#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096
#define NPAGES 200

struct vmstats before, after;

void get_stats(int pid, struct vmstats *s) {
  if(getvmstats(pid, s) < 0){
    printf("ERROR: getvmstats failed for pid %d\n", pid);
    exit(1);
  }
}

void print_mlfq(int pid, char *name){
  struct mlfqinfo info;

  if(getmlfqinfo(pid, &info) < 0){
    printf("ERROR: getmlfqinfo failed for pid %d\n", pid);
    return;
  }

  printf("%s (PID %d): level=%d, times_scheduled=%d\n",
         name, pid, info.level, info.times_scheduled);

  printf("  ticks: [0]=%d [1]=%d [2]=%d [3]=%d\n",
         info.ticks[0], info.ticks[1],
         info.ticks[2], info.ticks[3]);
}

int main() {
  printf("\n===== STRICT VM TEST START =====\n");

  int pid = getpid();

  char *base = sbrk(NPAGES * PGSIZE);
  if(base == (char*)-1){
    printf("sbrk failed\n");
    exit(1);
  }

  /* touch memory once so pages exist before fork */
  for(int i = 0; i < NPAGES; i++)
    base[i * PGSIZE] = 0;

  printf("\n[TEST] MLFQ behavior\n");

  int child = fork();

  if(child == 0){
    int cpid = getpid();

    /* CPU-bound workload */
    for(volatile int i = 0; i < 1000000000; i++);

    print_mlfq(cpid, "Child BEFORE access");

    get_stats(cpid, &before);

    for(int i = 0; i < NPAGES; i++){
      base[i * PGSIZE] += 1;
    }

    get_stats(cpid, &after);


    print_mlfq(cpid, "Child AFTER access");
    exit(0);

  } else {

    pause(10);

    print_mlfq(pid, "Parent BEFORE access");

    get_stats(pid, &before);

    for(int i = 0; i < NPAGES; i++){
      base[i * PGSIZE] += 1;
    }

    get_stats(pid, &after);

    print_mlfq(pid, "Parent AFTER access");

    wait(0);

    printf("\n[INFO] EXPECTED:\n");
    printf("  Child level > Parent level\n");
  }
  exit(0);
}