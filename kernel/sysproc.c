#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
extern struct spinlock wait_lock;
extern int disksched_policy;   // global
struct diskstats{
  // PA4 disk stats
  int disk_reads;
  int disk_writes;
  int total_disk_latency;
  int num_disk_requests;
  int avg_disk_latency;
};
uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  int n;
  struct proc *p = myproc();
  argint(0, &n);

  uint64 addr = p->sz;

  if(n < 0){
    if(growproc(n) < 0)
      return -1;
  } else {
    //pa3 changes
    if(addr + n < addr) return -1;
    if(addr + n > TRAPFRAME) return -1;
    //lazy allocation
    p->sz += n;
  }

  return addr;
}


uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//pa1 changed here
uint64
sys_hello(void)
{
  printf("Hello from xv6!\n");
  return 0;
}

uint64
sys_getpid2(void)
{
  return myproc()->pid;
}

uint64
sys_getppid(void)
{
  struct proc* process=myproc();
  if(process->parent){
    return process->parent->pid;
  }
  else return -1;
}

extern struct proc proc[NPROC];
uint64
sys_getnumchild(void)
{
  struct proc *p=myproc();
  struct proc *pp;
  int count=0;
  acquire(&wait_lock);
  for(pp=proc;pp<&proc[NPROC];pp++){
    if(pp->parent==p&&pp->state!=UNUSED&&pp->state!=ZOMBIE){
      count++;
    }
  }
  release(&wait_lock);
  return count;
}

uint64
sys_getsyscount(void)
{
  return myproc()->syscount;
}

uint64
sys_getchildsyscount(void)
{
  int pid;
  argint(0,&pid);
  struct proc *p;
  int total=-1;
  acquire(&wait_lock);
  for(p=proc;p<&proc[NPROC];p++){
    if(p->pid==pid&&p->parent==myproc()&&p->state!=UNUSED){
      total=p->syscount;
      break;
    }
  }
  release(&wait_lock);
  return total;
}
//pa2 changes
uint64
sys_getlevel(void)
{
  struct proc *p=myproc();
  int level;
  acquire(&p->lock);
  level=p->queue_level;
  release(&p->lock);
  return level;
}
uint64
sys_getmlfqinfo(void)
{
  int pid;
  uint64 addr;
  argint(0, &pid);
  argaddr(1, &addr);
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid ){
      struct mlfqinfo info;
      info.level = p->queue_level;
      for(int i=0;i<4;i++)
        info.ticks[i] = p->total_ticks_per_level[i];
      info.times_scheduled = p->total_no_of_times_scheduled;
      info.total_syscalls = p->syscount;
      release(&p->lock);
      if(copyout(myproc()->pagetable, addr, (char*)&info, sizeof(info)) < 0)
        return -1;
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}
//pa3 changes
uint64
sys_getvmstats(void)
{
  int pid;
  uint64 addr;

  argint(0, &pid);
  argaddr(1, &addr);

  struct proc *p;
  struct proc *cur = myproc();

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);

    if(p->pid == pid && p->state != UNUSED){

      struct vmstats info;

      info.page_faults = p->page_faults;
      info.pages_evicted = p->pages_evicted;
      info.pages_swapped_in = p->pages_swapped_in;
      info.pages_swapped_out = p->pages_swapped_out;
      info.resident_pages = p->resident_pages;
      info.disk_reads=dstats.disk_reads;
      info.disk_writes=dstats.disk_writes;
      info.total_disk_latency=dstats.total_disk_latency;
      info.num_disk_requests=dstats.num_disk_requests;
      info.avg_disk_latency=dstats.avg_disk_latency;
      release(&p->lock);

      if(copyout(cur->pagetable, addr, (char*)&info, sizeof(info)) < 0)
        return -1;

      return 0;
    }

    release(&p->lock);
  }

  return -1;
}
//pa4 changes
extern int disksched_policy;

uint64
sys_setdisksched(void)
{
  int policy;

  argint(0, &policy);   // just call, no check

  if(policy != 0 && policy != 1)
    return -1;

  disksched_policy = policy;

  return 0;
}
extern int raid_mode;
uint64
sys_setraidmode(void)
{
  int mode;
  argint(0, &mode);

  if(mode != 0 && mode != 1 && mode != 5)
    return -1;

  raid_mode = mode;
  return 0;
}
