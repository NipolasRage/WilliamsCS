#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
  if(argc > 1){
    for(int i = 1; i < argc; i++){
      int childid = fork();
      if(childid == 0){
	printf("calling fact\n");
	execl("fact", "fact", argv[i], NULL);
      }
      else{
	printf("waiting\n ");
	waitpid(childid, NULL, 0);
      }
    }
  }

}
