#include "kernel/types.h"
#include "user/user.h"

#define MAX_PROCESSES 5
#define MAX_RESOURCES 3

static int process_count;
static int resource_count;
static int allocation[MAX_PROCESSES][MAX_RESOURCES];
static int maximum[MAX_PROCESSES][MAX_RESOURCES];
static int need[MAX_PROCESSES][MAX_RESOURCES];
static int available[MAX_RESOURCES];
static int deadlocked[MAX_PROCESSES];
static int deadlocked_count;

static void
print_vector(char *name, int vector[])
{
  int resource;

  printf("%s[", name);
  for (resource = 0; resource < resource_count; resource++) {
    printf("%d", vector[resource]);
    if (resource + 1 < resource_count)
      printf(" ");
  }
  printf("]\n");
}

static void
print_matrix(char *name, int matrix[][MAX_RESOURCES])
{
  int process;

  printf("%s\n", name);
  for (process = 0; process < process_count; process++) {
    printf("  P%d ", process);
    print_vector("", matrix[process]);
  }
}

static void
print_state(char *label)
{
  printf("\n%s\n", label);
  print_vector("Available ", available);
  print_matrix("Allocation", allocation);
  print_matrix("Need", need);
}

static int
can_finish(int process, int work[])
{
  int resource;

  for (resource = 0; resource < resource_count; resource++)
    if (need[process][resource] > work[resource])
      return 0;
  return 1;
}

static void
load_state(int processes, int resources,
           int input_allocation[][MAX_RESOURCES],
           int input_maximum[][MAX_RESOURCES], int input_available[])
{
  int process;
  int resource;

  process_count = processes;
  resource_count = resources;
  for (resource = 0; resource < resource_count; resource++)
    available[resource] = input_available[resource];
  for (process = 0; process < process_count; process++) {
    for (resource = 0; resource < resource_count; resource++) {
      allocation[process][resource] = input_allocation[process][resource];
      maximum[process][resource] = input_maximum[process][resource];
      need[process][resource] =
          maximum[process][resource] - allocation[process][resource];
    }
  }
}

/* Returns the number of processes left unfinished and records their indices. */
int
detect_deadlock(void)
{
  int work[MAX_RESOURCES];
  int finish[MAX_PROCESSES];
  int process;
  int resource;
  int changed;

  deadlocked_count = 0;
  for (resource = 0; resource < resource_count; resource++)
    work[resource] = available[resource];
  for (process = 0; process < process_count; process++)
    finish[process] = 0;

  printf("\nDeadlock detection steps\n");
  print_vector("Work ", work);
  do {
    changed = 0;
    for (process = 0; process < process_count; process++) {
      if (finish[process] || !can_finish(process, work))
        continue;
      finish[process] = 1;
      changed = 1;
      for (resource = 0; resource < resource_count; resource++)
        work[resource] += allocation[process][resource];
      printf("  P%d finishes; ", process);
      print_vector("Work ", work);
    }
  } while (changed);

  for (process = 0; process < process_count; process++)
    if (!finish[process])
      deadlocked[deadlocked_count++] = process;
  if (deadlocked_count == 0)
    printf("  No deadlocked processes\n");
  else {
    printf("  Deadlocked processes: ");
    for (process = 0; process < deadlocked_count; process++)
      printf("P%d%s", deadlocked[process],
             process + 1 == deadlocked_count ? "\n" : ", ");
  }
  return deadlocked_count;
}

/* Checks whether the pending request can be granted while preserving safety. */
int
is_safe_state(void)
{
  int work[MAX_RESOURCES];
  int finish[MAX_PROCESSES];
  int process;
  int resource;
  int changed;

  for (resource = 0; resource < resource_count; resource++)
    work[resource] = available[resource];
  for (process = 0; process < process_count; process++)
    finish[process] = 0;

  do {
    changed = 0;
    for (process = 0; process < process_count; process++) {
      if (finish[process] || !can_finish(process, work))
        continue;
      finish[process] = 1;
      changed = 1;
      for (resource = 0; resource < resource_count; resource++)
        work[resource] += allocation[process][resource];
    }
  } while (changed);

  for (process = 0; process < process_count; process++)
    if (!finish[process])
      return 0;
  return 1;
}

static int
banker_request(int process, int request[])
{
  int resource;
  int old_available[MAX_RESOURCES];
  int old_allocation[MAX_RESOURCES];
  int old_need[MAX_RESOURCES];
  int safe;

  for (resource = 0; resource < resource_count; resource++) {
    if (request[resource] > need[process][resource] ||
        request[resource] > available[resource])
      return 0;
    old_available[resource] = available[resource];
    old_allocation[resource] = allocation[process][resource];
    old_need[resource] = need[process][resource];
    available[resource] -= request[resource];
    allocation[process][resource] += request[resource];
    need[process][resource] -= request[resource];
  }
  safe = is_safe_state();
  for (resource = 0; resource < resource_count; resource++) {
    available[resource] = old_available[resource];
    allocation[process][resource] = old_allocation[resource];
    need[process][resource] = old_need[resource];
  }
  return safe;
}

static void
run_matrix_tests(void)
{
  static int deadlock_allocation[2][MAX_RESOURCES] = {{1, 0}, {0, 1}};
  static int deadlock_maximum[2][MAX_RESOURCES] = {{1, 1}, {1, 1}};
  static int deadlock_available[MAX_RESOURCES] = {0, 0};
  static int safe_allocation[5][MAX_RESOURCES] = {
      {0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}};
  static int safe_maximum[5][MAX_RESOURCES] = {
      {7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}};
  static int safe_available[MAX_RESOURCES] = {3, 3, 2};
  static int unsafe_request[MAX_RESOURCES] = {3, 3, 0};
  int result;

  load_state(2, 2, deadlock_allocation, deadlock_maximum,
             deadlock_available);
  print_state("Test 1: deadlocked state");
  result = detect_deadlock();
  printf("Result: %s\n", result ? "DEADLOCK" : "SAFE");

  load_state(5, 3, safe_allocation, safe_maximum, safe_available);
  print_state("Test 2: safe state");
  result = detect_deadlock();
  printf("Result: %s\n", result ? "DEADLOCK" : "SAFE");

  printf("\nTest 3: Banker request P4 [3 3 0]\n");
  result = banker_request(4, unsafe_request);
  printf("Request result: %s\n", result ?
         "GRANTED (safe)" : "DENIED (would be unsafe)");
}

static void
run_pipe_deadlock(void)
{
  int pipe1[2];
  int pipe2[2];
  int process_a;
  int process_b;
  int status;
  int pipe_allocation[2][MAX_RESOURCES] = {{1, 0}, {0, 1}};
  int pipe_maximum[2][MAX_RESOURCES] = {{1, 1}, {1, 1}};
  int pipe_available[MAX_RESOURCES] = {0, 0};

  printf("\nActual pipe deadlock demonstration\n");
  if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
    printf("pipe creation failed\n");
    return;
  }
  process_a = fork();
  if (process_a == 0) {
    close(pipe1[0]);
    close(pipe2[1]);
    read(pipe2[0], &status, 1);
    exit(0);
  }
  process_b = fork();
  if (process_b == 0) {
    close(pipe1[1]);
    close(pipe2[0]);
    read(pipe1[0], &status, 1);
    exit(0);
  }
  close(pipe1[0]);
  close(pipe1[1]);
  close(pipe2[0]);
  close(pipe2[1]);
  pause(2);

  load_state(2, 2, pipe_allocation, pipe_maximum, pipe_available);
  print_state("Pipe resource graph");
  status = detect_deadlock();
  printf("Pipe demo result: %s\n", status ?
         "circular wait detected" : "no deadlock");
  kill(process_a);
  kill(process_b);
  wait(&status);
  wait(&status);
}

int
main(int argc, char **argv)
{
  run_matrix_tests();
  run_pipe_deadlock();
  exit(0);
}
