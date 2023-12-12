#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // sleep

#define NUM_ITEMS 1000
#define MAX 100

int num_threads;

// Here is the data for my bounded buffer
int buffer[MAX];
int front;
int tail;
int num;

int get()
{
  int result = -1;

  while(num == 0){ //  while(front == tail){

  }
  result = buffer[front % MAX];
  front++;
  num--;

  return result;
}

void *consumer(void *rank)
{
  int count = 0;
  while(count < NUM_ITEMS){
    printf("***CONS thread %ld read %d \n", (long)rank, get());
    get();
    count++;
    //    if(count % 17 == 0) sleep(1);
  }
  fflush(0);
  printf("***CONS thread done\n");
  printf("front %d tail %d num %d\n", front, tail, num);
  fflush(0);
  return NULL;
}

void put(int item)
{

  while(num == MAX){  //  while((tail-front) == MAX){

  }
  buffer[tail % MAX] = item;
  tail++;
  num++;

}

void *producer(void *rank)
{
  int count = 0;
  while(count < NUM_ITEMS){
    count++;
    put(count);
    printf("P thread %ld put %d \n", (long)rank, count);
    //    if(count % 7 == 0) sleep(1);
  }
  fflush(0);
  printf("P front %d tail %d num %d\n", front, tail, num);
  printf("P thread done\n");
  fflush(0);
  
  return NULL;
}

int main(int argc, char *argv[])
{
  long pthread;
  num_threads = 2;

  pthread_t ids[num_threads];

  front=tail=num = 0; // initialize variables
  
  pthread_create(&ids[0], NULL, producer, (void*)0);
  pthread_create(&ids[1], NULL, consumer, (void*)1);  

  for(int i = 0; i < num_threads; i++){
    pthread_join(ids[i], NULL);
  }

  printf("main finishes\n");
  return 0;
}
