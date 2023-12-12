#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // sleep
#include <semaphore.h>

int num_threads;
int val;
sem_t semaphore;

void *Hello(void *rank)
{
  sem_wait(&semaphore);
  val++;
  sem_post(&semaphore);
  return NULL;
}

int main(int argc, char *argv[])
{
  long pthread;
  num_threads = 4;
  val = 0;
  pthread_t ids[num_threads];

  sem_init(&semaphore, 0, 1);
  
  for(long i = 0; i < num_threads; i++){
    pthread_create(&ids[i], NULL, Hello, (void*)i);
  }

  for(int i = 0; i < num_threads; i++){
    pthread_join(ids[i], NULL);
  }

  printf("Value of val %d\n", val);
  return 0;
}
