# xv6 Kernel Extensions

This repository contains my extensions to the **xv6 (RISC-V)** kernel, developed as part of a series of Operating Systems course assignments. The implementation builds on the original xv6 source by extending core kernel components for process management, scheduling, virtual memory, disk-backed swap, and RAID-based storage.

---

## Features Implemented

### Custom System Calls
- Added custom system calls for process management, scheduling, and virtual memory.
- Implemented process accounting and runtime statistics.

---

### SC-MLFQ Scheduler
Implemented a **System-Call-Aware Multi-Level Feedback Queue (SC-MLFQ)** scheduler with:
- Four priority queues and round-robin scheduling
- Queue-specific time quanta
- System-call-aware scheduling for interactive processes
- CPU-bound process demotion and priority boosting
- Per-process scheduling statistics

---

### Virtual Memory Management
Extended xv6's virtual memory subsystem by implementing:
- Global frame table management
- Clock page replacement algorithm
- Swap-in and swap-out handling
- Priority-aware page replacement using SC-MLFQ levels
- Per-process virtual memory statistics

---

### Disk-backed Swap and Storage
Implemented:
- Disk-backed swap management
- Disk request queue
- FCFS and SSTF disk scheduling
- RAID 0, RAID 1, and RAID 5 storage
- Disk I/O statistics

---

## Major Kernel Components Modified

### Kernel
```
kernel/
├── defs.h
├── kalloc.c
├── proc.c
├── proc.h
├── syscall.c
├── syscall.h
├── sysproc.c
├── trap.c
├── virtio_disk.c
├── vm.c
├── raid.c        (Added)
└── raid.h        (Added)
```
### User-space Interface
```
user/
├── user.h
└── usys.pl
```

The user-space modifications were made primarily to expose the newly implemented kernel system calls.

---

## Acknowledgement

This work is based on the original **xv6 (RISC-V)** operating system developed by MIT. The repository contains my implementations and kernel extensions completed as part of a series of Operating Systems course assignments.
