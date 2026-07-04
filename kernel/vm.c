#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "buf.h"
#include "raid.h"
//PA4 CHANGES
//pa3 changes
// #define SWAP_SIZE 256
#define MAX_FRAMES 128
#define SWAP_PAGES (SWAP_BLOCKS / BLOCKS_PER_PAGE)
// char swap_mem[SWAP_SIZE][PGSIZE];
int used_swap[SWAP_PAGES];// 0->free 1->already in use
struct spinlock swap_lock;


void swap_initialise() {
  initlock(&swap_lock, "swap");
  int i=0;
  while(i<SWAP_PAGES){
    used_swap[i] = 0;
    i++;
  }
}

struct frame {
  int used;
  struct proc *p;
  uint64 va;
  int ref;
};

struct frame frame_table[MAX_FRAMES];
struct spinlock frame_lock;
int clk_ptr = 0;

void frame_initialise() {
  initlock(&frame_lock, "frame");
  for(int i = 0; i < MAX_FRAMES; i++){
    frame_table[i].used = 0;
    frame_table[i].p = 0;
    frame_table[i].va = 0;
    frame_table[i].ref = 0;
  }
  clk_ptr = 0;
}

void mark_frame_used(struct proc *p, uint64 va) {
  acquire(&frame_lock);
  for(int i = 0; i < MAX_FRAMES; i++){
    if(frame_table[i].used &&
       frame_table[i].p == p &&
       frame_table[i].va == va){
      frame_table[i].ref = 1;
      break;
    }
  }
  release(&frame_lock);
}

int find_victim_frame()
{
  int best_idx = -1;
  int maxqueue = -1;
  int i = 0;
  while(i < MAX_FRAMES){
    int idx = (clk_ptr + i) % MAX_FRAMES;
    struct frame *f = &frame_table[idx];

    if(f->used == 0){
      continue;
    }

    if(f->ref == 1)
      f->ref = 0;
    else{
      if(f->p && (best_idx == -1 || f->p->queue_level > maxqueue)){
        best_idx = idx;
        maxqueue = f->p->queue_level;
      }
    }
    i++;
  }
  // if none had reference bit 0
  if(best_idx == -1){
    int i = 0;
    while(i < MAX_FRAMES){
      int idx = (clk_ptr + i) % MAX_FRAMES;
      struct frame *f = &frame_table[idx];

      if(f->used && f->ref == 0 && f->p){
        if(best_idx == -1 || f->p->queue_level > maxqueue){
          best_idx = idx;
          maxqueue = f->p->queue_level;
        }
      }

      i++;
    }
  }
  return best_idx;
}
uint64
evict_page()
{
  acquire(&frame_lock);

  int best_idx = find_victim_frame();

  if(best_idx == -1){
    release(&frame_lock);
    return 0;
  }

  struct frame *victim = &frame_table[best_idx];
  struct proc *victim_proc = victim->p;
  uint64 va = victim->va;

  pte_t *pte = walk(victim_proc->pagetable, va, 0);
  if(pte == 0 || !(*pte & PTE_V)){
    clk_ptr = (best_idx + 1) % MAX_FRAMES;
    release(&frame_lock);
    return evict_page();
  }

  uint64 pa = PTE2PA(*pte);
  release(&frame_lock);
  acquire(&swap_lock);
  int swap_idx = -1;

  for(int i = 0; i < SWAP_PAGES; i++){
    if(used_swap[i] == 0){
      used_swap[i] = 1;
      swap_idx = i;
      break;
    }
  }
  release(&swap_lock);
  if(swap_idx == -1){
    return 0;
  }
//PA4 CHANGES
  // memmove(swap_mem[swap_idx], (void*)pa, PGSIZE);
  //write page to disk blocks
  int start_block = SWAP_START_BLOCK + swap_idx * BLOCKS_PER_PAGE;
  char *src = (char*)pa;
  for(int b = 0; b < BLOCKS_PER_PAGE; b++){
    uint blockno = start_block + b;
    raid_write(blockno, src + b * BSIZE);
    
  }
  acquire(&frame_lock);
//CHANGE DONE

  victim_proc->swap_slot[va / PGSIZE] = swap_idx;


  // unmap and update afterwards
  *pte = 0;
  sfence_vma();
  victim_proc->pages_swapped_out++;
  victim_proc->resident_pages--;
  victim_proc->pages_evicted++;

  victim->used = 0;
  victim->p = 0;
  victim->va = 0;
  victim->ref = 0;

  clk_ptr = (best_idx + 1) % MAX_FRAMES;

  release(&frame_lock);

  return pa;
}

void
cleanvm(struct proc *p)
{
  // free the frames
  acquire(&frame_lock);
  for(int i = 0; i < MAX_FRAMES; i++){
    if(frame_table[i].used && frame_table[i].p == p){
      frame_table[i].used = 0;
      frame_table[i].p = 0;
      frame_table[i].va = 0;
      frame_table[i].ref = 0;
    }
  }
  release(&frame_lock);

   acquire(&swap_lock);
   for(int i = 0; i < MAX_PAGES; i++){
     if(p->swap_slot[i] != -1){
       used_swap[p->swap_slot[i]] = 0;
       p->swap_slot[i] = -1;
     }
   }
   release(&swap_lock);
}
//change done




/*
 * the kernel's page table.
 */
pagetable_t kernel_pagetable;

extern char etext[];  // kernel.ld sets this to end of kernel code.

extern char trampoline[]; // trampoline.S

// Make a direct-map page table for the kernel.
pagetable_t
kvmmake(void)
{
  pagetable_t kpgtbl;

  kpgtbl = (pagetable_t) kalloc();
  memset(kpgtbl, 0, PGSIZE);

  // uart registers
  kvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);

  // virtio mmio disk interface
  kvmmap(kpgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

  // PLIC
  kvmmap(kpgtbl, PLIC, PLIC, 0x4000000, PTE_R | PTE_W);

  // map kernel text executable and read-only.
  kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);

  // map kernel data and the physical RAM we'll make use of.
  kvmmap(kpgtbl, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);

  // map the trampoline for trap entry/exit to
  // the highest virtual address in the kernel.
  kvmmap(kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

  // allocate and map a kernel stack for each process.
  proc_mapstacks(kpgtbl);
  
  return kpgtbl;
}

// add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
void
kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
  if(mappages(kpgtbl, va, sz, pa, perm) != 0)
    panic("kvmmap");
}

// Initialize the kernel_pagetable, shared by all CPUs.
void
kvminit(void)
{
  kernel_pagetable = kvmmake();
}

// Switch the current CPU's h/w page table register to
// the kernel's page table, and enable paging.
void
kvminithart()
{
  // wait for any previous writes to the page table memory to finish.
  sfence_vma();

  w_satp(MAKE_SATP(kernel_pagetable));

  // flush stale entries from the TLB.
  sfence_vma();
}

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
//
// The risc-v Sv39 scheme has three levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// A 64-bit virtual address is split into five fields:
//   39..63 -- must be zero.
//   30..38 -- 9 bits of level-2 index.
//   21..29 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
//    0..11 -- 12 bits of byte offset within the page.
pte_t *
walk(pagetable_t pagetable, uint64 va, int alloc)
{
  if(va >= MAXVA)
    panic("walk");

  for(int level = 2; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if(*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      if(!alloc || (pagetable = (pde_t*)kalloc()) == 0)
        return 0;
      memset(pagetable, 0, PGSIZE);
      *pte = PA2PTE(pagetable) | PTE_V;
    }
  }
  return &pagetable[PX(0, va)];
}

// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64
walkaddr(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  uint64 pa;

  if(va >= MAXVA)
    return 0;

  pte = walk(pagetable, va, 0);
  if(pte == 0)
    return 0;
  if((*pte & PTE_V) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;
  pa = PTE2PA(*pte);
  return pa;
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa.
// va and size MUST be page-aligned.
// Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int
mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
  uint64 a, last;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("mappages: va not aligned");

  if((size % PGSIZE) != 0)
    panic("mappages: size not aligned");

  if(size == 0)
    panic("mappages: size");
  
  a = va;
  last = va + size - PGSIZE;
  for(;;){
    if((pte = walk(pagetable, a, 1)) == 0)
      return -1;
    if(*pte & PTE_V)
      panic("mappages: remap");
    *pte = PA2PTE(pa) | perm | PTE_V;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// create an empty user page table.
// returns 0 if out of memory.
pagetable_t
uvmcreate()
{
  pagetable_t pagetable;
  pagetable = (pagetable_t) kalloc();
  if(pagetable == 0)
    return 0;
  memset(pagetable, 0, PGSIZE);
  return pagetable;
}

// Remove npages of mappings starting from va. va must be
// page-aligned. It's OK if the mappings don't exist.
// Optionally free the physical memory.
void
uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
  uint64 a;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("uvmunmap: not aligned");

  for(a = va; a < va + npages*PGSIZE; a += PGSIZE){
    if((pte = walk(pagetable, a, 0)) == 0) // leaf page table entry allocated?
      continue;   
    if((*pte & PTE_V) == 0)  // has physical page been allocated?
      continue;
    if(do_free){
      if(*pte & PTE_V){
      uint64 pa = PTE2PA(*pte);
      kfree((void*)pa);
      struct proc* p=myproc();
      if(p&&p->resident_pages>0)p->resident_pages--;
    }}
    *pte = 0;
  }
}

// Allocate PTEs and physical memory to grow a process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
uint64
uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
{
  char *mem;
  uint64 a;

  if(newsz < oldsz)
    return oldsz;

  oldsz = PGROUNDUP(oldsz);
  for(a = oldsz; a < newsz; a += PGSIZE){
    mem = kalloc();
    if(mem == 0){
      uvmdealloc(pagetable, a, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if(mappages(pagetable, a, PGSIZE, (uint64)mem, PTE_R|PTE_U|xperm) != 0){
      kfree(mem);
      uvmdealloc(pagetable, a, oldsz);
      return 0;
    }
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
uint64
uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
  if(newsz >= oldsz)
    return oldsz;

  if(PGROUNDUP(newsz) < PGROUNDUP(oldsz)){
    int npages = (PGROUNDUP(oldsz) - PGROUNDUP(newsz)) / PGSIZE;
    uvmunmap(pagetable, PGROUNDUP(newsz), npages, 1);
  }

  return newsz;
}

// Recursively free page-table pages.
// All leaf mappings must already have been removed.
void
freewalk(pagetable_t pagetable)
{
  // there are 2^9 = 512 PTEs in a page table.
  for(int i = 0; i < 512; i++){
    pte_t pte = pagetable[i];
    if((pte & PTE_V) && (pte & (PTE_R|PTE_W|PTE_X)) == 0){
      // this PTE points to a lower-level page table.
      uint64 child = PTE2PA(pte);
      freewalk((pagetable_t)child);
      pagetable[i] = 0;
    } else if(pte & PTE_V){
      panic("freewalk: leaf");
    }
  }
  kfree((void*)pagetable);
}

// Free user memory pages,
// then free page-table pages.
void
uvmfree(pagetable_t pagetable, uint64 sz)
{
  if(sz > 0)
    uvmunmap(pagetable, 0, PGROUNDUP(sz)/PGSIZE, 1);
  freewalk(pagetable);
}

// Given a parent process's page table, copy
// its memory into a child's page table.
// Copies both the page table and the
// physical memory.
// returns 0 on success, -1 on failure.
// frees any allocated pages on failure.
int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  pte_t *pte;
  uint64 pa, i;
  uint flags;
  char *mem;

  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walk(old, i, 0)) == 0)
      continue;   // page table entry hasn't been allocated
    if((*pte & PTE_V) == 0)
      continue;   // physical page hasn't been allocated
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    if((mem = kalloc()) == 0)
      goto err;
    memmove(mem, (char*)pa, PGSIZE);
    if(mappages(new, i, PGSIZE, (uint64)mem, flags) != 0){
      kfree(mem);
      goto err;
    }
  }
  return 0;

 err:
  uvmunmap(new, 0, i / PGSIZE, 1);
  return -1;
}

// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void
uvmclear(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  
  pte = walk(pagetable, va, 0);
  if(pte == 0)
    panic("uvmclear");
  *pte &= ~PTE_U;
}

// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int
copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
  uint64 n, va0, pa0;
  pte_t *pte;

  while(len > 0){
    va0 = PGROUNDDOWN(dstva);
    //pa 3 change
    mark_frame_used(myproc(), va0);

    if(va0 >= MAXVA)
      return -1;
  
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0) {
      if((pa0 = vmfault(pagetable, va0, 0)) == 0) {
        return -1;
      }
    }

    pte = walk(pagetable, va0, 0);
    // forbid copyout over read-only user text pages.
    if((*pte & PTE_W) == 0)
      return -1;
      
    n = PGSIZE - (dstva - va0);
    if(n > len)
      n = len;
    memmove((void *)(pa0 + (dstva - va0)), src, n);

    len -= n;
    src += n;
    dstva = va0 + PGSIZE;
  }
  return 0;
}

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
int
copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
  uint64 n, va0, pa0;

  while(len > 0){
    va0 = PGROUNDDOWN(srcva);
    //pa 3 change
    mark_frame_used(myproc(), va0);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0) {
      if((pa0 = vmfault(pagetable, va0, 0)) == 0) {
        return -1;
      }
    }
    n = PGSIZE - (srcva - va0);
    if(n > len)
      n = len;
    memmove(dst, (void *)(pa0 + (srcva - va0)), n);

    len -= n;
    dst += n;
    srcva = va0 + PGSIZE;
  }
  return 0;
}

// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcva in a given page table,
// until a '\0', or max.
// Return 0 on success, -1 on error.
int
copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
  uint64 n, va0, pa0;
  int got_null = 0;

  while(got_null == 0 && max > 0){
    va0 = PGROUNDDOWN(srcva);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (srcva - va0);
    if(n > max)
      n = max;

    char *p = (char *) (pa0 + (srcva - va0));
    while(n > 0){
      if(*p == '\0'){
        *dst = '\0';
        got_null = 1;
        break;
      } else {
        *dst = *p;
      }
      --n;
      --max;
      p++;
      dst++;
    }

    srcva = va0 + PGSIZE;
  }
  if(got_null){
    return 0;
  } else {
    return -1;
  }
}

// allocate and map user memory if process is referencing a page
// that was lazily allocated in sys_sbrk().
// returns 0 if va is invalid or already mapped, or if
// out of physical memory, and physical address if successful.
uint64
vmfault(pagetable_t pagetable, uint64 va, int read)
{
  uint64 mem;
  struct proc *p = myproc();

  if (va >= p->sz)
    return 0;

  va = PGROUNDDOWN(va);
  //pa3 changes
  // if already mapped set reference to 1
  if(ismapped(pagetable, va)) {
    mark_frame_used(p, va);
    return walkaddr(pagetable,va);
  }
  int vpn = va / PGSIZE;
  int was_swapped = (p->swap_slot[vpn] != -1);
  // ensure a free frame exists
  while(1){
    acquire(&frame_lock);
    int has_free = 0;
    for(int i = 0; i < MAX_FRAMES; i++){
      if(frame_table[i].used == 0){
        has_free = 1;
        break;
      }
    }
    release(&frame_lock);
    if(has_free)
      break;
    if(evict_page() == 0)
      return 0;
  }

  // allocate memory
  mem = (uint64)kalloc();
  if(mem == 0){
    while((mem = evict_page()) == 0);
  }

  // load data
  if(was_swapped){
    acquire(&swap_lock);

    int idx = p->swap_slot[vpn];
    //PA4 CHANGES
    release(&swap_lock);
    // memmove((void*)mem, swap_mem[idx], PGSIZE);
    //read page from disk blocks

    int start_block = SWAP_START_BLOCK + idx * BLOCKS_PER_PAGE;
    char *dst = (char*)mem;
    for(int b = 0; b < BLOCKS_PER_PAGE; b++){
      uint blockno = start_block + b;
      raid_read(blockno, dst + b * BSIZE);
    }
//CHANGES DONE
    acquire(&swap_lock);
    used_swap[idx] = 0;
    p->swap_slot[vpn] = -1;
    release(&swap_lock);
    p->pages_swapped_in++;
  } else {
    memset((void*)mem, 0, PGSIZE);
  }
  // insert into frame table
  int slot = -1;
  acquire(&frame_lock);
  for(int i = 0; i < MAX_FRAMES; i++){
    if(!frame_table[i].used){
      frame_table[i].used = 1;
      frame_table[i].p = p;
      frame_table[i].va = va;
      frame_table[i].ref = 1;
      slot = i;
      break;
    }
  }
  release(&frame_lock);

  if(slot == -1){
    kfree((void*)mem);
    return 0;
  }

  // map page
  if(mappages(pagetable, va, PGSIZE, mem, PTE_W|PTE_U|PTE_R) != 0){
    acquire(&frame_lock);
    frame_table[slot].used = 0;
    frame_table[slot].p = 0;
    frame_table[slot].va = 0;
    frame_table[slot].ref = 0;
    release(&frame_lock);
    kfree((void*)mem);
    return 0;
  }

  p->resident_pages++;

  return mem;
}

int
ismapped(pagetable_t pagetable, uint64 va)
{
  pte_t *pte = walk(pagetable, va, 0);
  if (pte == 0) {
    return 0;
  }
  if ((*pte & PTE_V)&&(*pte&(PTE_R|PTE_W|PTE_X))){
    return 1;
  }
  return 0;
}