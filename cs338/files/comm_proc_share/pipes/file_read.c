#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  if(argc > 1){
    int fd = open(argv[1], O_RDONLY);
    if(fd == -1){
      printf("Error opening %s\n", argv[1]);
      exit(1);
    }
    int num=0;
    char buffer[100];
    while( (num = read(fd, buffer, 100)) != 0){
      printf("Read %d-%s-\n", num, buffer);
    }
    printf("errno %d\n", errno);
    
    close(fd);
  }
}
