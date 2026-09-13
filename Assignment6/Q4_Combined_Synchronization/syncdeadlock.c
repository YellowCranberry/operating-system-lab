#include "kernel/types.h"
#include "user/user.h"

#define NPROC   5
#define NRES    3
#define CYCLES  2

char *resname[NRES] = {"Printer", "Scanner", "Disk"};
int   rescount[NRES] = {2, 1, 2};

int sem[NRES][2];

int needs[NPROC][2] = {
    {0, 1},
    {0, 2},
    {1, 2},
    {0, 2},
    {0, 1},
};

void
sem_init(int r)
{
  pipe(sem[r]);
  for (int i = 0; i < rescount[r]; i++)
    write(sem[r][1], "T", 1);
}

void
sem_acquire(int r, int who)
{
  char buf[1];
  printf("P%d requesting %s\n", who, resname[r]);
  read(sem[r][0], buf, 1);
  printf("P%d granted %s\n", who, resname[r]);
}

void
sem_release(int r, int who)
{
  write(sem[r][1], "T", 1);
  printf("P%d releasing %s\n", who, resname[r]);
}

void
worker(int who)
{
  int first = needs[who][0];
  int second = needs[who][1];

  for (int c = 0; c < CYCLES; c++) {
    printf("P%d starting cycle %d (needs %s + %s)\n",
           who, c, resname[first], resname[second]);

    sem_acquire(first, who);
    sem_acquire(second, who);

    printf("P%d starting work (cycle %d)\n", who, c);
    for (volatile int i = 0; i < 5000000; i++)
      ;
    printf("P%d work completed (cycle %d)\n", who, c);

    sem_release(second, who);
    sem_release(first, who);
  }

  exit(0);
}

int
main(int argc, char *argv[])
{
  printf("===== Q4: Combined Synchronization & Deadlock Avoidance =====\n");
  printf("Strategy: Resource ordering (Printer < Scanner < Disk)\n");
  printf("Pool sizes -> Printer=%d Scanner=%d Disk=%d\n\n",
         rescount[0], rescount[1], rescount[2]);

  for (int r = 0; r < NRES; r++)
    sem_init(r);

  int pids[NPROC];
  for (int i = 0; i < NPROC; i++) {
    int pid = fork();
    if (pid == 0) {
      worker(i);
    }
    pids[i] = pid;
  }

  for (int i = 0; i < NPROC; i++)
    wait(0);

  printf("\n===== All %d processes completed %d cycle(s) each with no deadlock =====\n",
         NPROC, CYCLES);
  exit(0);
}
