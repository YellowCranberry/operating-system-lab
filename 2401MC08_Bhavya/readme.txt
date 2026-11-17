Name: Bhavya
Roll Number: 2401MC08
Course: MA3105 Operating System Lab Exam
Set: Set 1

q1 Counting Semaphores for Process Synchronization
Files Modified/Added:`sem.c` (new), `defs.h`, `main.c`, `syscall.h`, `syscall.c`, `sysproc.c`, `user.h`, `usys.S`, `Makefile`, `synctest.c` (new).
Description:** Added a kernel table of 8 semaphores protected by a spinlock so user processes can coordinate without busy-waiting. Implemented `sem_init`, `sem_wait`, and `sem_post` using xv6’s sleep-wakeup mechanism to safely block and resume tasks. Wired everything through the syscall layers and created `synctest.c` to force child processes to print A, B, and C in strict order.

q2 Memory Statistics and Address Translation
Files Modified/Added:** `kalloc.c`, `vm.c`, `defs.h`, `syscall.h`, `syscall.c`, `sysproc.c`, `user.h`, `usys.S`, `Makefile`, `memtest.c` (new).
Description:** Tracked free physical memory inside `kalloc.c` and exposed it via a clean `freepages` system call. Added a page table walk helper in `vm.c` to implement `va2pa`, verifying user access permissions before translating virtual addresses to physical frames. Wrote `memtest.c` to verify heap growth, fork memory isolation, and proper cleanup after process exit.
