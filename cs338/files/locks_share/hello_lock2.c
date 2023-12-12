#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // sleep

int num_threads;
int val;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv;

void *Hello(void *rank)
{

  pthread_mutex_lock(&mutex);
  int tmp = val+1;
  if((long)rank % 2 == 0) {
    pthread_cond_wait(&cv, &mutex);
    sleep(1);
    tmp = val+1;
  }
  val = tmp;
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main(int argc, char *argv[])
{
  long pthread;
  num_threads = 4;
  val = 0;
  pthread_t ids[num_threads];
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cv, NULL);
  
  for(long i = 0; i < num_threads; i++){
    pthread_create(&ids[i], NULL, Hello, (void*)i);
  }
  sleep(5);
  pthread_cond_signal(&cv);
  pthread_cond_signal(&cv);
  for(int i = 0; i < num_threads; i++){
    pthread_join(ids[i], NULL);
  }

  printf("Value of val %d\n", val);
  return 0;
}
