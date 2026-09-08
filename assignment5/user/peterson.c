#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define ITERATIONS 10

struct shared_data {
  volatile int flag[2];
  volatile int turn;
  volatile int counter_value;
};

static inline void
memory_barrier(void)
{
  __sync_synchronize();
}

void
enter_cs(struct shared_data *shared_state, int process_id)
{
  int other_process = 1 - process_id;

  shared_state->flag[process_id] = 1;
  memory_barrier();

  shared_state->turn = other_process;
  memory_barrier();

  while(shared_state->flag[other_process] == 1 &&
        shared_state->turn == other_process) ;
}

void
leave_cs(struct shared_data *shared_state, int process_id)
{
  memory_barrier();
  shared_state->flag[process_id] = 0;
  memory_barrier();
}

void
delay(void)
{
  for(volatile int i = 0; i < 500000; i++) ;
}

int
main(void)
{
  struct shared_data *shared_state;
  int child_pid;
  int process_id;

  child_pid = fork();

  if(child_pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  shared_state = (struct shared_data *)shm_get();

  if(shared_state == 0){
    printf("shm_get failed\n");
    exit(1);
  }

  if(child_pid == 0)
    process_id = 1;
  else
    process_id = 0;

  if(process_id == 0){
    shared_state->flag[0] = 0;
    shared_state->flag[1] = 0;
    shared_state->turn = 0;
    shared_state->counter_value = 0;

    memory_barrier();
  }

  if(process_id == 0){
    for(volatile int i = 0; i < 100000; i++) ;
  }

  for(int i = 0; i < ITERATIONS; i++){
        enter_cs(shared_state, process_id);

        shared_state->counter_value++;

    printf("Process %d ENTER CS, counter = %d\n",
          process_id, shared_state->counter_value);

    for(volatile int j = 0; j < 100000; j++) ;

    printf("Process %d EXIT CS\n", process_id);

    leave_cs(shared_state, process_id);

    delay();
  }

  if(process_id == 0){
    wait(0);

    printf("Final counter = %d\n", shared_state->counter_value);
  }

  exit(0);
}