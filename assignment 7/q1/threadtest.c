#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_THREADS 3
#define ITERS 100000
#define PGSIZE 4096

volatile int counter = 0;
volatile int start_gate = 0; // Ensures all threads start at the exact same moment

void
worker(void *arg)
{
  // Spin until the main process has created all threads
  while(start_gate == 0)
    ;

  for(int i = 0; i < ITERS; i++){
    int val = counter;
    // Tiny delay to widen the race condition window across CPU cores
    for(volatile int d = 0; d < 10; d++)
      ;
    counter = val + 1;
  }
  exit(0);
}

int
main(int argc, char *argv[])
{
  void *stacks[NUM_THREADS];

  printf("Starting threadtest with %d threads, %d iterations each...\n", NUM_THREADS, ITERS);

  for(int i = 0; i < NUM_THREADS; i++){
    stacks[i] = malloc(PGSIZE);
    if(stacks[i] == 0){
      printf("malloc failed\n");
      exit(1);
    }

    uint64 stack_top = ((uint64)stacks[i] + PGSIZE) & ~0xF;

    int tid = clone(worker, 0, (void*)stack_top);
    if(tid < 0){
      printf("clone failed\n");
      exit(1);
    }
  }

  // Release all 3 threads simultaneously so they execute concurrently
  start_gate = 1;

  for(int i = 0; i < NUM_THREADS; i++){
    join();
    free(stacks[i]);
  }

  printf("Expected counter: %d\n", NUM_THREADS * ITERS);
  printf("Actual counter:   %d\n", counter);

  exit(0);
}