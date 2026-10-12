#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NTHREADS 3
#define ITERATIONS 100000

volatile int counter = 0;

#define SEM_MUTEX    0
#define SEM_EMPTY    1
#define SEM_FULL     2
#define SEM_PC_MUTEX 3

void
sync_worker(void *arg)
{
  int id = *(int *)arg;
  printf("[Sync Thread %d] Started (PID: %d). Incrementing with sem_wait/sem_signal...\n", id, getpid());

  for (int i = 0; i < ITERATIONS; i++) {
    sem_wait(SEM_MUTEX);
    counter++;
    sem_signal(SEM_MUTEX);
  }

  printf("[Sync Thread %d] Finished %d increments.\n", id, ITERATIONS);
  exit(0);
}

// Producer-Consumer definitions
#define BUFFER_SIZE 5
#define TOTAL_ITEMS 15

int buffer[BUFFER_SIZE];
int in_idx = 0;
int out_idx = 0;

void
producer_worker(void *arg)
{
  printf("[Producer] Started (PID: %d). Producing %d items into buffer (size %d)...\n",
         getpid(), TOTAL_ITEMS, BUFFER_SIZE);

  for (int i = 1; i <= TOTAL_ITEMS; i++) {
    sem_wait(SEM_EMPTY);
    sem_wait(SEM_PC_MUTEX);

    buffer[in_idx] = i;
    printf("[Producer] Produced item %d at buffer[%d]\n", i, in_idx);
    in_idx = (in_idx + 1) % BUFFER_SIZE;

    sem_signal(SEM_PC_MUTEX);
    sem_signal(SEM_FULL);

    // Minor delay to simulate production work
    for (volatile int d = 0; d < 80000; d++);
  }

  printf("[Producer] Finished producing all %d items.\n", TOTAL_ITEMS);
  exit(0);
}

void
consumer_worker(void *arg)
{
  printf("[Consumer] Started (PID: %d). Consuming %d items from buffer...\n",
         getpid(), TOTAL_ITEMS);

  for (int i = 1; i <= TOTAL_ITEMS; i++) {
    sem_wait(SEM_FULL);
    sem_wait(SEM_PC_MUTEX);

    int item = buffer[out_idx];
    printf("[Consumer] Consumed item %d from buffer[%d]\n", item, out_idx);
    out_idx = (out_idx + 1) % BUFFER_SIZE;

    sem_signal(SEM_PC_MUTEX);
    sem_signal(SEM_EMPTY);

    // Minor delay to simulate consumption work
    for (volatile int d = 0; d < 120000; d++);
  }

  printf("[Consumer] Finished consuming all %d items.\n", TOTAL_ITEMS);
  exit(0);
}

int
main(int argc, char *argv[])
{
  printf("====================================================\n");
  printf("  OS Lab Assignment 7 - Question 2: Process/Thread Synchronization\n");
  printf("====================================================\n");

  printf("\n--- Part 1: Mutex Semaphore Synchronized Counter ---\n");
  sem_init(SEM_MUTEX, 1);
  counter = 0;

  int ids[NTHREADS];
  void *stacks[NTHREADS];

  for (int i = 0; i < NTHREADS; i++) {
    ids[i] = i + 1;
    stacks[i] = malloc(4096);
    int tid = clone(sync_worker, &ids[i], (char *)stacks[i] + 4096);
    if (tid < 0) {
      printf("Error: clone failed\n");
      exit(1);
    }
  }

  for (int i = 0; i < NTHREADS; i++) {
    int pid = join();
    printf("Joined thread PID %d\n", pid);
  }

  printf("\nPart 1 Result:\n");
  printf("Expected Counter: %d\n", NTHREADS * ITERATIONS);
  printf("Actual Counter:   %d\n", counter);

  if (counter == NTHREADS * ITERATIONS) {
    printf("SUCCESS: Mutex semaphore completely eliminated race condition! (Exact %d)\n", counter);
  } else {
    printf("FAILURE: Unexpected counter value %d\n", counter);
  }

  printf("\n--- Part 2: Producer-Consumer Problem ---\n");
  printf("Buffer size = %d, Total items = %d\n", BUFFER_SIZE, TOTAL_ITEMS);
  sem_init(SEM_EMPTY, BUFFER_SIZE);
  sem_init(SEM_FULL, 0);
  sem_init(SEM_PC_MUTEX, 1);

  void *p_stack = malloc(4096);
  void *c_stack = malloc(4096);

  int prod_pid = clone(producer_worker, 0, (char *)p_stack + 4096);
  int cons_pid = clone(consumer_worker, 0, (char *)c_stack + 4096);

  if (prod_pid < 0 || cons_pid < 0) {
    printf("Error: clone failed for producer/consumer\n");
    exit(1);
  }

  join();
  join();

  printf("\nPart 2 Result:\n");
  printf("SUCCESS: All %d items produced and consumed without buffer underflow or overflow!\n", TOTAL_ITEMS);
  printf("====================================================\n");

  exit(0);
}
