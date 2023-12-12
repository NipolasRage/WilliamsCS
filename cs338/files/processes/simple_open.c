#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  int filedes = open("foo.txt", O_RDWR);
  if(filedes == -1){
    printf("Error opening file\n");
  }
  else{
    printf("Opened file correctly %d\n", filedes);
    close(filedes);
  }
}
