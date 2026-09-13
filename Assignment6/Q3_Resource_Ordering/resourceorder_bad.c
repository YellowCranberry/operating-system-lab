#include "kernel/types.h"
#include "user/user.h"

#define WATCHDOG_TICKS 300

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
process_a(void)
{
  printf("Process A starting: will acquire Lock1 then Lock2\n");
  lock_acquire(lock1, "Lock1", "Process A");

  for (volatile int i = 0; i < 50000000; i++)
    ;

  printf("Process A waiting for Lock2\n");
  lock_acquire(lock2, "Lock2", "Process A");

  printf("Process A completed work (should not be reached in bad version)\n");
  lock_release(lock2, "Lock2", "Process A");
  lock_release(lock1, "Lock1", "Process A");
  exit(0);
}

void
process_b(void)
{
  printf("Process B starting: will acquire Lock2 then Lock1\n");
  lock_acquire(lock2, "Lock2", "Process B");

  for (volatile int i = 0; i < 50000000; i++)
    ;

  printf("Process B waiting for Lock1\n");
  lock_acquire(lock1, "Lock1", "Process B");

  printf("Process B completed work (should not be reached in bad version)\n");
  lock_release(lock1, "Lock1", "Process B");
  lock_release(lock2, "Lock2", "Process B");
  exit(0);
}

int
main(int argc, char *argv[])
{
  printf("===== Q3: BAD version (inconsistent lock order) =====\n");
  printf("Expected outcome: Process A and Process B deadlock; the\n");
  printf("watchdog forcibly terminates them after a bounded timeout.\n\n");

  lock_init(lock1);
  lock_init(lock2);

  int pid_a = fork();
  if (pid_a == 0) {
    process_a();
  }

  int pid_b = fork();
  if (pid_b == 0) {
    process_b();
  }

  int watchdog = fork();
  if (watchdog == 0) {
    sleep(WATCHDOG_TICKS);
    printf("\nWatchdog: timeout reached -- A and B are still deadlocked.\n");
    printf("Watchdog: killing Process A (pid %d) and Process B (pid %d)\n",
           pid_a, pid_b);
    kill(pid_a);
    kill(pid_b);
    exit(0);
  }

  wait(0);
  wait(0);
  wait(0);

  printf("\n===== Q3 BAD version finished (deadlock reproduced and safely terminated by watchdog) =====\n");
  exit(0);
}
