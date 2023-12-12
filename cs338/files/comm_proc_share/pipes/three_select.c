#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
  int child1_to_parent[2];
  if(pipe(child1_to_parent) == -1){
    fprintf(stderr, "Unable to open pipe\n");
    exit(1);
  }

  int child2_to_parent[2];
  if(pipe(child2_to_parent) == -1){
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
    close(child1_to_parent[0]);
    close(child2_to_parent[1]);
    close(child2_to_parent[0]);
    for(int i = 0; i < 5; i++){
      sprintf(buffer, "Message %d", i);
      if(write(child1_to_parent[1], buffer, 100) != 0){ 
	//	printf("CHILD1 sent %s\n", buffer);
      }
    }

    close(child1_to_parent[1]);
    printf("CHILD1 done\n");
    
    return 0;
  } else {

    int pid = fork();
    if(pid == 0){
    
      char buffer[100];
      close(child1_to_parent[1]);
      close(child1_to_parent[0]);
      close(child2_to_parent[0]);
      for(int i = 0; i < 5; i++){
	sprintf(buffer, "Speak %d", i);
	if(write(child2_to_parent[1], buffer, 100) != 0){ 
	  //printf("CHILD2 sent %s\n", buffer);
	}
      }
      close(child2_to_parent[1]);
    }
    else{
      close(child1_to_parent[1]);
      close(child2_to_parent[1]);

      fd_set readset;

      int done1=0, done2=0;
      while(done1 == 0 || done2==0){
	FD_ZERO(&readset);
	FD_SET(child1_to_parent[0], &readset);
	FD_SET(child2_to_parent[0], &readset);

	// read fd still ready on EOF
	int num = select(child2_to_parent[0] + 1, &readset, NULL, NULL, NULL);

	if(num == -1)
	  break;
	char buffer[100];
	if(FD_ISSET(child1_to_parent[0], &readset)){
	  num = read(child1_to_parent[0], buffer, 100);
	  if(num == 0) done1++;
	  else printf("from child1: %d %s\n", num, buffer);
	}
	if(FD_ISSET(child2_to_parent[0], &readset)){
	  num = read(child2_to_parent[0], buffer, 100);
	  if(num == 0) done2++;
	  else printf("from child2: %d %s\n", num, buffer);
	}

      }

      
      waitpid(pid, NULL, 0);
      printf("I am parent %d of process %d\n", getpid(), child_pid);
      waitpid(child_pid, NULL, 0);
      printf("parent finishing\n");
      return 0;
    }
    
  } 
}
