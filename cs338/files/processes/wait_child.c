#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
  int child_pid = fork();
  if(child_pid == -1){
    printf("Error creating process \n");
    exit(1);
  }
  else if (child_pid == 0) {
    sleep(5);
    printf("I am process %d\n", getpid());
    return 0;
  } else {
    wait(NULL);
    printf("I am parent %d of process %d\n", getpid(), child_pid);
    return 0;
  }
  

}
