
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "defs.h"

#define NSEMS 8

struct semaphore {
  int value;
  int in_use;
};

static struct {
  struct spinlock lock;
  struct semaphore sems[NSEMS];
} semaphores;

void
seminit(void)
{
  initlock(&semaphores.lock, "semaphores");
  for (int i = 0; i < NSEMS; i++) {
    semaphores.sems[i].value = 0;
    semaphores.sems[i].in_use = 0;
  }
}

int
ksem_init(int id, int value)
{
  if (id < 0 || id >= NSEMS || value < 0)
    return -1;

  acquire(&semaphores.lock);
  semaphores.sems[id].value = value;
  semaphores.sems[id].in_use = 1;
  wakeup(&semaphores.sems[id]);
  release(&semaphores.lock);
  return 0;
}

int
ksem_wait(int id)
{
  struct semaphore *sem;

  if (id < 0 || id >= NSEMS)
    return -1;

  sem = &semaphores.sems[id];
  acquire(&semaphores.lock);
  if (!sem->in_use) {
    release(&semaphores.lock);
    return -1;
  }
  while (sem->value == 0) {
    if (killed(myproc())) {
      release(&semaphores.lock);
      return -1;
    }
    sleep_prepare(sem);
    release(&semaphores.lock);
    sleep();
    acquire(&semaphores.lock);
  }
  sem->value--;
  release(&semaphores.lock);
  return 0;
}

int
ksem_post(int id)
{
  struct semaphore *sem;

  if (id < 0 || id >= NSEMS)
    return -1;

  sem = &semaphores.sems[id];
  acquire(&semaphores.lock);
  if (!sem->in_use) {
    release(&semaphores.lock);
    return -1;
  }
  sem->value++;
  wakeup(sem);
  release(&semaphores.lock);
  return 0;
}
