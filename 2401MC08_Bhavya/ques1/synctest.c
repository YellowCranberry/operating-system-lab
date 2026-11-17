#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
run_child(int wait_id, int post_id, char letter)
{
  for (int i = 0; i < 5; i++) {
    if (sem_wait(wait_id) < 0) {
      printf("synctest: sem_wait failed\n");
      exit(1);
    }
    printf("%c", letter);
    if (sem_post(post_id) < 0) {
      printf("synctest: sem_post failed\n");
      exit(1);
    }
  }
  exit(0);
}

int
main(int argc, char *argv[])
{
  int pids[3];
  int nchildren = 0;

  if (sem_init(0, 1) < 0 || sem_init(1, 0) < 0 || sem_init(2, 0) < 0) {
    printf("synctest: sem_init failed\n");
    exit(1);
  }

  pids[0] = fork();
  if (pids[0] == 0)
    run_child(0, 1, 'A');
  if (pids[0] < 0)
    goto fork_failed;
  nchildren++;

  pids[1] = fork();
  if (pids[1] == 0)
    run_child(1, 2, 'B');
  if (pids[1] < 0)
    goto fork_failed;
  nchildren++;

  pids[2] = fork();
  if (pids[2] == 0)
    run_child(2, 0, 'C');
  if (pids[2] < 0)
    goto fork_failed;
  nchildren++;
  for (int i = 0; i < 3; i++)
    wait(0);
  printf("Done\n");
  exit(0);

fork_failed:
  printf("synctest: fork failed\n");
  for (int i = 0; i < nchildren; i++)
    kill(pids[i]);
  for (int i = 0; i < nchildren; i++)
    wait(0);
  exit(1);
}
