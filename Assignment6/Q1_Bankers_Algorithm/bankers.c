#include "kernel/types.h"
#include "user/user.h"

#define N 5
#define M 3

int Allocation[N][M];
int Max[N][M];
int Need[N][M];
int Available[M];

void
print_matrix(char *name, int mat[N][M])
{
  printf("%s\n", name);
  printf("      R0  R1  R2\n");
  for (int i = 0; i < N; i++) {
    printf("  P%d   %d   %d   %d\n", i, mat[i][0], mat[i][1], mat[i][2]);
  }
}

void
print_available(void)
{
  printf("Available   %d   %d   %d\n",
         Available[0], Available[1], Available[2]);
}

void
compute_need(void)
{
  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++)
      Need[i][j] = Max[i][j] - Allocation[i][j];
}

int
is_safe(int safe_seq[N])
{
  int work[M];
  int finish[N];

  for (int j = 0; j < M; j++)
    work[j] = Available[j];
  for (int i = 0; i < N; i++)
    finish[i] = 0;

  int count = 0;
  while (count < N) {
    int found = 0;
    for (int i = 0; i < N; i++) {
      if (finish[i])
        continue;

      int can_run = 1;
      for (int j = 0; j < M; j++) {
        if (Need[i][j] > work[j]) {
          can_run = 0;
          break;
        }
      }

      if (can_run) {
        for (int j = 0; j < M; j++)
          work[j] += Allocation[i][j];
        finish[i] = 1;
        safe_seq[count] = i;
        count++;
        found = 1;
      }
    }
    if (!found)
      break;
  }

  return (count == N);
}

void
print_safe_check(char *label)
{
  int safe_seq[N];
  printf("\n-- %s --\n", label);
  if (is_safe(safe_seq)) {
    printf("System is in a SAFE state. Safe sequence: ");
    for (int i = 0; i < N; i++) {
      printf("P%d", safe_seq[i]);
      if (i != N - 1)
        printf(" -> ");
    }
    printf("\n");
  } else {
    printf("System is in an UNSAFE state.\n");
  }
}

int
request_resources(int pid, int request[M])
{
  printf("\n-- Request from P%d: (%d, %d, %d) --\n",
         pid, request[0], request[1], request[2]);

  for (int j = 0; j < M; j++) {
    if (request[j] > Need[pid][j]) {
      printf("Request denied -- exceeds process's declared maximum need\n");
      return 0;
    }
  }

  for (int j = 0; j < M; j++) {
    if (request[j] > Available[j]) {
      printf("Request denied -- insufficient available resources\n");
      return 0;
    }
  }

  for (int j = 0; j < M; j++) {
    Available[j]  -= request[j];
    Allocation[pid][j] += request[j];
    Need[pid][j]  -= request[j];
  }

  int safe_seq[N];
  if (is_safe(safe_seq)) {
    printf("Request granted. New state is safe. Safe sequence: ");
    for (int i = 0; i < N; i++) {
      printf("P%d", safe_seq[i]);
      if (i != N - 1)
        printf(" -> ");
    }
    printf("\n");
    return 1;
  } else {

    for (int j = 0; j < M; j++) {
      Available[j]  += request[j];
      Allocation[pid][j] -= request[j];
      Need[pid][j]  += request[j];
    }
    printf("Request denied -- would lead to unsafe state\n");
    return 0;
  }
}

int
main(int argc, char *argv[])
{

  int alloc_init[N][M] = {
      {0, 1, 0},
      {2, 0, 0},
      {3, 0, 2},
      {2, 1, 1},
      {0, 0, 2},
  };
  int max_init[N][M] = {
      {7, 5, 3},
      {3, 2, 2},
      {9, 0, 2},
      {2, 2, 2},
      {4, 3, 3},
  };
  int avail_init[M] = {3, 3, 2};

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++) {
      Allocation[i][j] = alloc_init[i][j];
      Max[i][j] = max_init[i][j];
    }
  for (int j = 0; j < M; j++)
    Available[j] = avail_init[j];

  compute_need();

  printf("===== Banker's Algorithm =====\n\n");
  print_matrix("Allocation", Allocation);
  printf("\n");
  print_matrix("Max", Max);
  printf("\n");
  print_matrix("Need", Need);
  printf("\n");
  print_available();

  print_safe_check("Initial safety check");

  int req1[M] = {1, 0, 2};
  request_resources(1, req1);
  print_matrix("Allocation (after P1 request)", Allocation);
  printf("\n");
  print_available();

  int req2[M] = {2, 3, 0};
  request_resources(0, req2);

  int req3[M] = {8, 0, 0};
  request_resources(0, req3);

  printf("\n===== End of demonstration =====\n");
  exit(0);
}
