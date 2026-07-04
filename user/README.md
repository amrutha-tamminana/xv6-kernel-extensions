# PA4 README – Disk Scheduling and RAID-backed Swap

## Implemented Features

- Disk-backed swap system replacing in-memory swap from PA3  
- Page eviction writes pages to disk using multiple blocks per page  
- Page faults trigger swap-in by reading data from disk  
- Disk scheduling policies implemented:
  - FCFS (First Come First Serve)
  - SSTF (Shortest Seek Time First)
- Disk request queue maintained in kernel  
- Disk latency model implemented using:
  - |current_block - requested_block| + constant delay  
- RAID support for swap blocks:
  - RAID 0 (striping)
  - RAID 1 (mirroring)
  - RAID 5 (striping with parity)
- RAID mapping applied only to swap region blocks  
- Global disk statistics maintained:
  - disk_reads
  - disk_writes
  - total_disk_latency
  - number of disk requests  
- Integration with virtual memory system for swap-in and swap-out  
- Integration with scheduler (MLFQ) for frame eviction decisions :contentReference[oaicite:0]{index=0}  

---

## Design Decisions and Assumptions

- Swap space is placed at the end of disk using:
  - SWAP_START_BLOCK and SWAP_BLOCKS  

- Each page is divided into multiple disk blocks using BLOCKS_PER_PAGE  

- RAID is applied only for swap blocks, normal filesystem blocks directly use bread/bwrite  

- RAID 0 uses striping across disks based on logical block mapping  

- RAID 1 writes data to both primary and mirror blocks, reads are always served from primary  

- RAID 5 parity is computed using XOR and updated on every write  

- RAID 5 parity blocks are distributed across disks using rotating parity (stripe % NDISKS)  

- Disk scheduling is implemented using a linked list queue of disk requests  

- FCFS selects the first request in the queue  

- SSTF selects the request with minimum seek distance from current disk head  

- Disk head position is tracked globally using curr_block  

- Disk latency is computed as:
  - |current_block - requested_block| + constant delay  

- Disk statistics are maintained globally (not per process)  

- Swap slots are tracked using an array (used_swap[]) and freed after swap-in  

- Frame replacement uses a clock algorithm with second chance (reference bit)  

- Frame eviction additionally prefers processes with lower priority (higher queue_level value)  
---

## Experimental Results

### Swap and Eviction
- Large memory allocation triggers page faults and eviction  
- Observed:
  - pages_evicted > 0  
  - pages_swapped_out > 0  

### Swap-in Correctness
- Accessing evicted pages triggers swap-in  
- Data verified after reload  
- Result:
  - errors = 0  

### Disk Scheduling
- FCFS:
  - Requests served in order of arrival  
- SSTF:
  - Requests reordered based on nearest block  
- Observed difference in block access patterns  

### RAID Verification
- RAID 0:
  - Blocks distributed across disks  
- RAID 1:
  - Data correctly mirrored  
  - No data loss observed  
- RAID 5:
  - Parity computed and updated correctly  
  - Data reconstructed correctly  
  - errors = 0  

### Scheduling Comparison
- FCFS vs SSTF:
  - SSTF reduces seek distance  
  - Lower total disk latency observed  

---

## Summary

- Successfully implemented disk-backed swap system  
- Disk scheduling and RAID integrated correctly  
- All test cases pass with correct outputs  
- System maintains correctness and data integrity under all configurations  