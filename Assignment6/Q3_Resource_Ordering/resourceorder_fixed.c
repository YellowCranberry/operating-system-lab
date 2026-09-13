#include "kernel/types.h"
#include "user/user.h"

int lock1[2];
int lock2[2];

void
lock_init(int fd[2])
{
  pipe(fd);
  write(fd[1], "T", 1);
}

void
lock_acquire(int fd[2], char *name, char *who)
{
  char buf[1];
  read(fd[0], buf, 1);
  printf("%s acquired %s\n", who, name);
}

void
lock_release(int fd[2], char *name, char *who)
{
  write(fd[1], "T", 1);
  printf("%s released %s\n", who, name);
}

void
worker(char *who)
{
  printf("%s starting: will acquire Lock1 then Lock2 (global order)\n", who);

  lock_acquire(lock1, "Lock1", who);

  for (volatile int i = 0; i < 50000000; i++)
    ;

  lock_acquire(lock2, "Lock2", who);

  printf("%s completed work\n", who);

  lock_release(lock2, "Lock2", who);
  lock_release(lock1, "Lock1", who);
  exit(0);
}

int
main(int argc, char *argv[])
{
  printf("===== Q3: FIXED version (consistent lock order: Lock1 -> Lock2) =====\n\n");

  lock_init(lock1);
  lock_init(lock2);

  int pid_a = fork();
  if (pid_a == 0) {
    worker("Process A");
  }

  int pid_b = fork();
  if (pid_b == 0) {
    worker("Process B");
  }

  wait(0);
  wait(0);

  printf("\n===== Q3 FIXED version finished: both processes completed, no deadlock =====\n");
  exit(0);
}
