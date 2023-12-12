#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
  int children[5];
  for(int i = 0; i < 5; i++){
    int child_pid = fork();
    if(child_pid == -1){
      printf("Error creating process \n");
      exit(1);
    }
    else if (child_pid == 0) {
      printf("I am process %d\n", getpid());
      return 0;
    } else {
      children[i] = child_pid;
      //      wait(NULL);
      printf("I am parent %d of process %d\n", getpid(), child_pid);
    }
  }

  for(int i = 0; i < 5; i++){
    waitpid(children[i], NULL, 0);
  }
  
  return 0;

}
