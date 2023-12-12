#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // sleep

int num_threads;
int val;

void *Hello(void *rank)
{
  int tmp = val+1;
  if((long)rank % 2 ==  0)
    sleep(1);
  val = tmp;
  return NULL;
}

int main(int argc, char *argv[])
{
  long pthread;
  num_threads = 4;
  val = 0;
  pthread_t ids[num_threads];
  
  for(long i = 0; i < num_threads; i++){
    pthread_create(&ids[i], NULL, Hello, (void*)i);
  }

  for(int i = 0; i < num_threads; i++){
    pthread_join(ids[i], NULL);
  }

  printf("Value of val %d\n", val);
  return 0;
}
