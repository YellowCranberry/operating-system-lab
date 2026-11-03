# Assignment 10 — xv6 Memory Management and Paging

## Implementation

This submission uses xv6-public at commit `eeb7b41`. The kernel changes are in [`xv6/`](xv6/); all four required user programs are included in the xv6 filesystem image.

- **Q1 — Physical memory statistics:** [`kalloc.c`](xv6/kalloc.c) tracks free, allocated, and freed pages under the allocator lock. Boot-time free-list initialization is excluded from lifetime allocation/free counters. `memstats()` prints total, available, allocated, freed, and currently allocated pages. [`memtest.c`](xv6/memtest.c) allocates and touches three pages with `sbrk()`, then releases them.
- **Q2 — Virtual-to-physical translation:** [`vm.c`](xv6/vm.c) exports `walkpgdir()` and implements `vtop()`, checking for a present mapping below `KERNBASE` and combining the physical frame with the virtual page offset. [`vttest.c`](xv6/vttest.c) tests an allocated page, its next byte, and an unmapped address.
- **Q3 — Page inspection/protection:** [`pageinfo()` and `setreadonly()`](xv6/vm.c) report the frame and P/W/U bits, clear `PTE_W`, and invalidate the TLB entry. [`pagetest.c`](xv6/pagetest.c) forks before attempting a write so the parent can report the expected child page fault.
- **Q4 — Page replacement:** [`pagereplace.c`](xv6/pagereplace.c) traces FIFO, LRU, and Optimal for every reference with three frames, then compares faults, hits, hit ratio, and fault ratio for three, four, and five frames.

The syscall declarations, dispatch, and user stubs are wired through [`sysproc.c`](xv6/sysproc.c), [`syscall.c`](xv6/syscall.c), [`syscall.h`](xv6/syscall.h), [`defs.h`](xv6/defs.h), [`user.h`](xv6/user.h), and [`usys.S`](xv6/usys.S). The xv6 [`Makefile`](xv6/Makefile) adds the test programs to the filesystem image. [`README.md`](README.md) contains build and run instructions.

## Results

All tests below were run in QEMU from the built xv6 image. Full console transcripts and QEMU VGA screenshots are linked in each section.

### Q1 — Memory statistics

The runtime allocator pool contained **57,000 pages**. At the start of `memtest`, 56,789 pages were available, with 425 runtime allocations and 214 releases (211 currently allocated). Allocating and touching three pages reduced available pages to 56,786 and raised current allocations to 214. Releasing them restored availability to 56,789 and current allocations to 211; the freed counter increased to 217.

- [Q1 console transcript](evidence/q1-memtest.txt)
- [Q1 screenshot](evidence/q1-memtest.png)

### Q2 — Virtual-to-physical translation

In the recorded run, virtual address `0x3000` translated to `0xDFBC000`, and `0x3001` translated to `0xDFBC001`, demonstrating preservation of the page offset. Unmapped address `0x7FFFF000` returned `-1`.

- [Q2 console transcript](evidence/q2-vttest.txt)
- [Q2 screenshot](evidence/q2-vttest.png)

### Q3 — Page information and read-only protection

The allocated page mapped to frame 57276 (`0xDFBC000`). Before protection its flags were P=1, W=1, U=1; after `setreadonly()`, W became 0. The forked child’s write caused trap 14 (page fault, error 7) and the parent continued.

- [Q3 console transcript](evidence/q3-pagetest.txt)
- [Q3 screenshot](evidence/q3-pagetest.png)

### Q4 — Replacement algorithm results

Reference string: `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1` (20 references).

| Frames | Algorithm | Faults | Hits | Hit ratio | Fault ratio |
|---:|---|---:|---:|---:|---:|
| 3 | FIFO | 15 | 5 | 25.00% | 75.00% |
| 3 | LRU | 12 | 8 | 40.00% | 60.00% |
| 3 | Optimal | 9 | 11 | 55.00% | 45.00% |
| 4 | FIFO | 10 | 10 | 50.00% | 50.00% |
| 4 | LRU | 8 | 12 | 60.00% | 40.00% |
| 4 | Optimal | 8 | 12 | 60.00% | 40.00% |
| 5 | FIFO | 9 | 11 | 55.00% | 45.00% |
| 5 | LRU | 7 | 13 | 65.00% | 35.00% |
| 5 | Optimal | 7 | 13 | 65.00% | 35.00% |

Optimal has the fewest faults for three frames. With four and five frames, Optimal and LRU tie on this reference string.

- [Q4 full per-reference transcript and comparison table](evidence/q4-pagereplace.txt)
- [Q4 screenshot](evidence/q4-pagereplace.png)

## Validation

A clean build of the kernel, all user programs, `xv6.img`, and `fs.img` completed successfully. The stock xv6 source triggers legacy GCC warnings in `mp.c` and `sh.c` on the installed compiler; the build kept `-Werror` enabled while exempting those two pre-existing warning classes. All four programs were run from fresh QEMU boots.
