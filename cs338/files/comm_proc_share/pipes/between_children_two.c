#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv)
{
  int child1_to_child2[2];
  if(pipe(child1_to_child2) == -1){
    fprintf(stderr, "Unable to open pipe\n");
    exit(1);
  }

  int child2_to_child1[2];
  if(pipe(child2_to_child1) == -1){
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
    close(child1_to_child2[0]);
    close(child2_to_child1[1]);
    for(int i = 0; i < 5; i++){
      sprintf(buffer, "Message %d", i);
      if(write(child1_to_child2[1], buffer, 100) != 0){  // are writes blocking?
	printf("CHILD1 sent %s\n", buffer);
      }
    }

    close(child1_to_child2[1]);

    while(read(child2_to_child1[0], buffer, 100) != 0){
      printf("CHILD1 received: %s\n", buffer);
    }
    
    return 0;
  } else {

    int pid = fork();
    if(pid == 0){
    
      char buffer[100];
      close(child1_to_child2[1]);
      close(child2_to_child1[0]);
      while(read(child1_to_child2[0], buffer, 100)){
	printf("CHILD2 received: %s\n", buffer);
	strncat(buffer, "--return", 8);
	write(child2_to_child1[1], buffer, 100);
      }
      printf("Child2 closed\n");
    }
    else{
      close(child1_to_child2[0]);
      close(child1_to_child2[1]);
      close(child2_to_child1[0]);
      close(child2_to_child1[1]);
      
      waitpid(pid, NULL, 0);
      printf("I am parent %d of process %d\n", getpid(), child_pid);
      waitpid(child_pid, NULL, 0);
      printf("parent finishing\n");
      return 0;
    }
    
  } 
}
