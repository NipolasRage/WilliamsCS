#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // sleep

#define NUM_ITEMS 10000
#define MAX 100
#define USE_LOCKS 1

int num_threads;

// Here is the data for my bounded buffer
int buffer[MAX];
int front;
int tail;
int num;
#ifdef USE_LOCKS
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
#endif


int get()
{
  int result = -1;
#ifdef USE_LOCKS
  pthread_mutex_lock(&lock);  
#endif
  while(num == 0){ //  while(front == tail){
#ifdef USE_LOCKS
    pthread_mutex_unlock(&lock);      
    pthread_mutex_lock(&lock);  
#endif
  }
  result = buffer[front % MAX];
  front++;
  num--;
#ifdef USE_LOCKS
  pthread_mutex_unlock(&lock);
#endif
  return result;
}

void *consumer(void *rank)
{
  int count = 0;
  while(count < NUM_ITEMS){
    get();
    //    printf("***CONS thread %ld read %d \n", (long)rank, get());
    count++;
  }
  fflush(0);
  printf("***CONS thread done\n");
  pthread_mutex_lock(&lock);  
  printf("front %d tail %d num %d\n", front, tail, num);
  pthread_mutex_unlock(&lock);  
  fflush(0);  

  return NULL;
}

void put(int item)
{
#ifdef USE_LOCKS
  pthread_mutex_lock(&lock);  
#endif
  while(num == MAX){  //  while((tail-front) == MAX){
#ifdef USE_LOCKS
    pthread_mutex_unlock(&lock);      
    pthread_mutex_lock(&lock);  
#endif
  }
  buffer[tail % MAX] = item;
  tail++;
  num++;
#ifdef USE_LOCKS
  pthread_mutex_unlock(&lock);
#endif

}

void *producer(void *rank)
{
  int count = 0;
  while(count < NUM_ITEMS){
    count++;
    put(count);
    //    printf("P thread %ld put %d \n", (long)rank, count);
  }
  fflush(0);
  pthread_mutex_lock(&lock);  
  printf("P front %d tail %d num %d\n", front, tail, num);
  pthread_mutex_unlock(&lock);  
  printf("P thread done\n");
  fflush(0);  

  return NULL;
}

int main(int argc, char *argv[])
{
  long pthread;
  num_threads = 2;

  pthread_t ids[num_threads];
  pthread_mutex_init(&lock, NULL);

  front=tail=num = 0; // initialize variables
  
  pthread_create(&ids[0], NULL, producer, (void*)0);
  pthread_create(&ids[1], NULL, consumer, (void*)1);  

  for(int i = 0; i < num_threads; i++){
    pthread_join(ids[i], NULL);
  }

  printf("main finishes\n");
  return 0;
}
