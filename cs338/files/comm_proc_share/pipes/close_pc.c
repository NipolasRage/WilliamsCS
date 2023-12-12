#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

// Need to close pipes you are not actively using.  If any process has
// a fd open still for writing, it will be assumed that the pipe
// should remain open and the EOF marker will not be recognized by reading
// side


int main(int argc, char **argv)
{
  int parent_to_child[2];
  if(pipe(parent_to_child) == -1){
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
    int val;
    close(parent_to_child[1]);
    while((val = read(parent_to_child[0], buffer, 100)) != 0){
    
      printf("CHILD received: %s\n", buffer);
      printf("val %d errno %d\n", val, errno);
    }
    printf("CHILD: after read returns 0\n");
    
    return 0;
  } else {
    char buffer[100];
    close(parent_to_child[0]);
    for(int i = 0; i < 5; i++){
      sprintf(buffer, "Message %d", i);
      if(write(parent_to_child[1], buffer, 100) != 0){  // are writes blocking?
	printf("Parent sent: %s\n", buffer);
      }
    }
    printf("after last send\n");
    close(parent_to_child[1]);

    printf("after close\n");
    
    wait(NULL);
    printf("I am parent %d of process %d\n", getpid(), child_pid);
    return 0;
  }
  

}
