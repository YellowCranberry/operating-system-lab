# xv6 Assignments Q1-Q4 Report

## 1. Design Decisions and Data Structures

### Q1-Q2: Threads and race condition

For Q1, I added thread creation through `clone()` and `join()`. A cloned thread gets its own trapframe and user stack, but shares the caller's page table, address space, open files, and working directory. I used `is_thread` in `struct proc` to tell threads apart and `ustack` to remember each thread's stack. `join()` waits for a thread to exit and frees its private trapframe without freeing the shared page table.

The test in `user/threadtest.c` starts three threads that share one `volatile int counter`. Each worker reads the counter, waits briefly, and writes it back. That delay makes it easier to see two threads interfering with each other. Sharing an address space does not make a read-modify-write operation safe.

### Q3: Deadlock detection and avoidance

For Q3, I kept the implementation in user space, as requested. `user/deadlock.c` uses fixed-size arrays, which keeps it simple and avoids dynamic allocation inside xv6:

- `Allocation[n][m]`: resources currently held by each process.
- `maximum[n][m]`: maximum declared demand.
- `need[n][m]`: derived as `maximum - allocation`.
- `available[m]`: currently free resources.
- `deadlocked[]`: process indices left unfinished by detection.

`detect_deadlock()` copies `Available` into a temporary `work` vector. It repeatedly looks for a process whose remaining `Need` can be satisfied, marks that process finished, and adds its allocation back to `work`. Anything left unfinished is reported as deadlocked. For Banker’s Algorithm, `is_safe_state()` checks a tentative request and the program restores the old state afterward.

I also created two real xv6 children and two pipes. Child A keeps Pipe1's write end and waits to read Pipe2. Child B does the opposite. The parent runs the detector and then kills both blocked children, so an intentional deadlock does not leave QEMU stuck.

### Q4: Priority preemptive scheduling

For Q4, I added these fields to `struct proc`:

- `priority`: range 1-20, where 1 is highest.
- `wait_ticks`: runnable ticks since the process was last scheduled.

`allocproc()` sets the default priority to 10. The new `set_priority(pid, priority)` and `get_priority(pid)` system calls access the field while holding the process lock. The scheduler looks through RUNNABLE processes and chooses the lowest priority number. I used a rotating cursor so equal-priority processes do not always lose because of their position in the process table.

On each timer tick from CPU 0, runnable processes increase `wait_ticks`. Once a process waits more than 30 ticks, its priority number is reduced by one and the counter is reset. The kernel prints this event, which made it possible to check that aging was really happening.

`user/priotest.c` forks four CPU-bound children with priorities 1, 5, 10, and 20. The parent assigns the priorities before the children start. I added a bounded delay inside the loop so the timer interrupts and aging show up during the test.

## 2. Q1-Q2 Race-Condition Comparison

| Aspect | Before / unsynchronized baseline | Current implementation / after thread support |
|---|---|---|
| Execution | No safe shared-counter update | Three xv6 threads created with `clone()` and joined with `join()` |
| Counter operation | Read, delay, write; not atomic | Read, delay, write inside the thread test |
| Expected value | 300,000 for 3 threads x 100,000 increments | The test reports a race when actual value is below 300,000 |
| Protection | None | Thread lifecycle and shared-memory support |
| Result | Lost updates are possible | The race remains intentionally demonstrable |

The race happens when two workers read the same old value, calculate the same new value, and then overwrite each other. The synchronized version would look like this:

```text
acquire(counter_lock)
value = counter
counter = value + 1
release(counter_lock)
```

That should consistently produce 300,000.

## 3. Q3 Deadlock Results

### Test 1: Deadlocked matrix

```text
Allocation = [[1, 0], [0, 1]]
Max        = [[1, 1], [1, 1]]
Available  = [0, 0]
```

P0 needs a resource held by P1, while P1 needs a resource held by P0. Since nothing is available, neither process can move forward. The output was:

```text
Deadlocked processes: P0, P1
Result: DEADLOCK
```

### Test 2: Safe matrix

The five-process, three-resource case starts with `Available = [3, 3, 2]`. The detector was able to finish the processes in this order:

```text
P1 -> P3 -> P4 -> P0 -> P2
```

Each process releases its allocation before the next part of the sequence, so the result is:

```text
No deadlocked processes
Result: SAFE
```

### Test 3: Unsafe Banker request

Starting from the safe matrix, P4 asks for `[3, 3, 0]`. The request is within its declared need and is available at first, but granting it would leave the system with no safe completion sequence. Banker rejects it:

```text
Request result: DENIED (would be unsafe)
```

### Actual pipe circular wait

The two children reproduce the same cycle using real xv6 pipes. The detector reports:

```text
Pipe demo result: circular wait detected
```

The parent then kills and waits for both blocked children.

## 4. Q4 Scheduling Behavior

I ran `priotest` with one CPU so the result was easier to read. The PIDs can change between runs, but the observed completion behavior was:

| Completion rank | Process | Requested priority | Effective priority | Result |
|---:|---|---:|---:|---|
| 1 | P4 | 1 | 1 | Completed first |
| 2 | P5 | 5 | 5 | Completed second |
| 3 | P6 | 10 | 9 | Aged once while waiting |
| 4 | P7 | 20 | 19 | Aged once, then completed |

Observed aging output included:

```text
aging: pid=6 wait_ticks=31 priority=9
aging: pid=7 wait_ticks=31 priority=19
```

Qualitative completion graph:

```text
Priority 1   |##########| completion 1
Priority 5   |####################| completion 2
Priority 10  |##############################| completion 3
Priority 20  |########################################| completion 4
```

This graph shows completion rank, not exact milliseconds, because the current test does not record per-process timestamps. It still shows the main point: smaller priority numbers run first, and the priority-20 process eventually gets CPU time after aging.

## 5. Problems Faced and Resolutions

1. **Scheduler locking:** I had to inspect candidates under their individual locks, then lock the selected process again before changing it to RUNNING. This avoids holding several process locks during a context switch.
2. **Tie-breaking:** A rotating scan cursor prevents equal-priority processes from always being selected in fixed process-table order.
3. **Banker safety bug:** My first version treated the requesting process as already finished, so it incorrectly accepted an unsafe request. I fixed the safety simulation so every process starts unfinished after the tentative allocation.
4. **Pipe test hang:** The parent detects the circular wait and then kills and waits for both blocked children.
5. **Invisible aging:** The first workload finished before 30 timer ticks. I added a bounded delay and kernel logging so the wait threshold and priority change could be seen.
