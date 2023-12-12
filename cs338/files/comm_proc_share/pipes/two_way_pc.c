#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv)
{
  int parent_to_child[2];
  if(pipe(parent_to_child) == -1){
    fprintf(stderr, "Unable to open pipe\n");
    exit(1);
  }

  int child_to_parent[2];
  if(pipe(child_to_parent) == -1){
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
    close(parent_to_child[1]);
    while(read(parent_to_child[0], buffer, 100) != 0){
      printf("CHILD read: %s\n", buffer);
      strncat(buffer, "--return", 8);
      write(child_to_parent[1], buffer, 100);
      printf("CHILD sent: %s\n", buffer);
    }
    printf("CHILD: after read returns 0\n");
    close(child_to_parent[1]);
    
    return 0;
  } else {
    char buffer[100];
    close(parent_to_child[0]);
    close(child_to_parent[1]);
    for(int i = 0; i < 5; i++){
      sprintf(buffer, "Message %d", i);
      if(write(parent_to_child[1], buffer, 100) != 0){  // are writes blocking?
	printf("PARENT sent: %s\n", buffer);
      }
    }
    close(parent_to_child[1]);
    while(read(child_to_parent[0], buffer, 100)){
      printf("PARENT read: %s\n", buffer);
    }
    printf("Child closed\n");
    
    wait(NULL);
    printf("I am parent %d of process %d\n", getpid(), child_pid);
    return 0;
  }
  

}
