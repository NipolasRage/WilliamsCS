#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv)
{
  int fd[2];
  if(pipe(fd) == -1){
    fprintf(stderr, "Unable to open pipe\n");
    exit(1);
  }
  
  int child_pid = fork();
  if(child_pid == -1){
    printf("Error creating process \n");
    exit(1);
  }
  else if (child_pid == 0) {
    char buffer[100];
    if(read(fd[0], buffer, 100) != 0){
      printf("Received %s\n", buffer);
    }
    printf("I am process %d\n", getpid());
    return 0;
  } else {
    char buffer[100];
    strcat(buffer, "Message");
    if(write(fd[1], buffer, 10) != 0){
      printf("Sent %s\n", buffer);
    }
    
    wait(NULL);
    printf("I am parent %d of process %d\n", getpid(), child_pid);
    return 0;
  }
  

}
