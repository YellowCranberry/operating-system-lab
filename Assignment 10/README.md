# Assignment 10 — xv6 submission

- [`xv6/`](xv6/) — downloaded xv6-public source with the assignment changes and user programs.
- [`q1_memstats.c`](q1_memstats.c), [`q2_vtop.c`](q2_vtop.c), [`q3_pageprotection.c`](q3_pageprotection.c), and [`q4_pagereplacement.c`](q4_pagereplacement.c) — standalone main/test programs, one for each question. They use xv6's `types.h`, `stat.h`, and `user.h`.
- [`Summary.md`](Assignment_10_Summary.md) — implementation notes, measured results, and conclusions.
- [`evidence/`](evidence/) — serial console transcripts and QEMU VGA screenshots for each question.

The xv6 source is based on `mit-pdos/xv6-public` commit `eeb7b41`.

## Build and run

From `xv6/`, build the boot and filesystem images:

```sh
make TOOLPREFIX= CFLAGS='-fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -Werror -Wno-error=array-bounds -Wno-error=infinite-recursion -MD -ggdb -m32 -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie' xv6.img fs.img
```

With QEMU installed, start the xv6 shell using `make qemu-nox`. Run the user tests at the xv6 prompt:

```text
memtest
vttest
pagetest
pagereplace
```

`pagereplace` prints each reference and frame state for FIFO, LRU, and Optimal with three frames, followed by the 3/4/5-frame comparison table.
