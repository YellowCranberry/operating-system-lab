# Synchronization Primitives for xv6-RISCV

| Field | Details |
| :--- | :--- |
| **Student Name** | Bhavya |
| **Roll Number** | 2401MC08 |
| **Course** | Operating Systems Lab |
| **Assignment** | Assignment 5 (Process Synchronization) |

---

This repository provides kernel-level extensions to the xv6 operating system (RISC-V), introducing inter-process synchronization primitives and shared memory mechanisms.

> **Note:** These patches are design extensions and require an existing xv6 kernel, C user runtime, and associated toolchain environment.

---

## Architecture and Design

The implementation adds two core subsystems to xv6, exposed to user space via four system calls:

### 1. Shared Memory (`kernel/shm.c`)
* **`void* shm_get(void)`**: Allocates a designated physical page frame and establishes page table mappings at virtual address `SHM_VA` for calling processes, enabling direct shared-memory communication across address spaces.

### 2. Counting Semaphores (`kernel/semaphore.c`)
* **`int sem_init(int sem_id, int value)`**: Initializes a kernel semaphore with an initial resource count.
* **`int sem_wait(int sem_id)`**: Decrements the semaphore value; transitions the calling process to a sleeping state if resources are exhausted (`value <= 0`).
* **`int sem_signal(int sem_id)`**: Increments the semaphore value and unblocks sleeping processes queued on the resource.

---

## Demonstration Programs

The userland suite illustrates classical concurrency patterns implemented using these primitives (configured via `UPROGS` in `Makefile`):

| Program | Target Problem | Implementation Details |
| :--- | :--- | :--- |
| `peterson.c` | Mutual Exclusion | Software-based two-process synchronization using shared memory flags. |
| `prodcons.c` | Bounded Buffer | Producer-consumer coordination utilizing counting semaphores. |
| `readwrite.c` | Reader-Writer | Concurrent read-access with mutually exclusive writer access. |

---

## Prerequisites

Ensure the following dependencies are installed and available in your `PATH`:

* **RISC-V Toolchain:** `riscv64-unknown-elf-gcc` and associated binary utilities
* **Emulator:** QEMU (v7.2+) with `qemu-system-riscv64`
* **Build Utility:** GNU Make

---

## Compilation and Execution

1. Build the kernel image and launch the QEMU emulator:

   ```sh
   make clean
   make qemu CPUS=1
