#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
  int val = 4;

  int pid = fork();
  if(pid == 0){
    val += 4;
    int child = fork();
    if(child == 0){
      char *str = "3";
      execl("fact", "fact", str, NULL);
      printf("val is %d\n", val);
    }
    else{
      val += 5;
      waitpid(child, NULL, 0); 
      printf("val2 is %d\n", val);
    }
  }
  else{
    val += 6;
    waitpid(pid, NULL, 0);
    printf("val3 is %d\n", val);
  }
}
