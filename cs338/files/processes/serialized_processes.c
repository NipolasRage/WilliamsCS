#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
  int val = 0;
  int child_pid = -1;
  for(int i = 0; i < 3; i++){
    child_pid = fork();
    if(child_pid == -1){
      printf("Error creating process \n");
      exit(1);
    }
    else if (child_pid == 0) {
      val = i;
      if(i == 2){
	printf("I am process %d\n", getpid());
	return 0;
      }
    }
    else{
      wait(NULL);
      printf("I am process %d\n", getpid());
      break;
    }
  }
  return 0;

}
