#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFFER_SIZE 5

#define EMPTY 0
#define FULL 1
#define MUTEX 2

struct buffer_state {
  int buffer[BUFFER_SIZE];
  int limit;
  int write_index;
  int read_index;
  int item_count;
};

void
delay(void)
{
  for(volatile int i = 0; i < 500000; i++) ;
}

void
producer(struct buffer_state *buffer_state)
{
  for(int item = 1; item <= 20; item++){
    if(buffer_state->item_count == buffer_state->limit)
      printf("Producer: buffer full, waiting...\n");

    sem_wait(EMPTY);

    sem_wait(MUTEX);

        buffer_state->buffer[buffer_state->write_index] = item;
        buffer_state->write_index =
          (buffer_state->write_index + 1) % buffer_state->limit;
        buffer_state->item_count++;

    printf("Producer: produced %d (buffer count = %d)\n",
          item, buffer_state->item_count);

    sem_signal(MUTEX);
    sem_signal(FULL);

    delay();
  }
}

void
consumer(struct buffer_state *buffer_state)
{
  for(int i = 1; i <= 20; i++){
    if(buffer_state->item_count == 0)
      printf("Consumer: buffer empty, waiting...\n");

    sem_wait(FULL);

    sem_wait(MUTEX);

        int item = buffer_state->buffer[buffer_state->read_index];

        buffer_state->read_index =
          (buffer_state->read_index + 1) % buffer_state->limit;
        buffer_state->item_count--;

    printf("Consumer: consumed %d (buffer count = %d)\n",
          item, buffer_state->item_count);

    sem_signal(MUTEX);
    sem_signal(EMPTY);

    delay();
  }
}

int
main(int argc, char *argv[])
{
  struct buffer_state *buffer_state;
  int buffer_limit = BUFFER_SIZE;
  int pid;

  if(argc > 1){
    buffer_limit = atoi(argv[1]);

    if(buffer_limit < 1 || buffer_limit > BUFFER_SIZE){
      printf("Buffer size must be between 1 and %d\n",
             BUFFER_SIZE);
      exit(1);
    }
  }

  buffer_state = (struct buffer_state *)shm_get();

  if(buffer_state == 0){
    printf("shm_get failed\n");
    exit(1);
  }

  buffer_state->limit = buffer_limit;
  buffer_state->write_index = 0;
  buffer_state->read_index = 0;
  buffer_state->item_count = 0;

  sem_init(EMPTY, buffer_limit);
  sem_init(FULL, 0);
  sem_init(MUTEX, 1);

  pid = fork();

  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){
    buffer_state = (struct buffer_state *)shm_get();

    if(buffer_state == 0){
      printf("child shm_get failed\n");
      exit(1);
    }

    consumer(buffer_state);
    exit(0);
  }

  producer(buffer_state);

  wait(0);

  printf("Producer-Consumer completed\n");

  exit(0);
}
