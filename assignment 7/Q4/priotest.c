#include "kernel/types.h"
#include "user/user.h"

#define CHILDREN 4
#define WORK 10000000

static int priorities[CHILDREN] = {1, 5, 10, 20};

static void
run_child(int index)
{
  int pid = getpid();
  int last_priority;
  volatile int value = 0;

  last_priority = get_priority(pid);
  printf("P%d started with priority %d\n", pid, last_priority);
  for (int i = 0; i < WORK; i++) {
    value = value * 1664525 + i;
    for (volatile int delay = 0; delay < 64; delay++)
      ;
    if ((i % 10000) == 0) {
      int current_priority = get_priority(pid);
      if (current_priority != last_priority) {
        printf("P%d aging: priority %d -> %d\n", pid, last_priority,
               current_priority);
        last_priority = current_priority;
      }
    }
  }
  printf("P%d completed with priority %d\n", pid, get_priority(pid));
  exit(0);
}

int
main(void)
{
  int pids[CHILDREN];
  int completed = 0;

  printf("priority scheduling test\n");
  for (int i = 0; i < CHILDREN; i++) {
    pids[i] = fork();
    if (pids[i] < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if (pids[i] == 0)
      run_child(i);
    if (set_priority(pids[i], priorities[i]) < 0) {
      printf("set_priority failed for P%d\n", pids[i]);
      exit(1);
    }
  }

  while (completed < CHILDREN) {
    int pid = wait(0);
    if (pid < 0)
      break;
    completed++;
    printf("completion %d: P%d\n", completed, pid);
  }
  printf("all priority test processes completed\n");
  exit(0);
}
