#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  for(int i = 0; i < 5; i++){

    printf("Process: %d \t Parent process: \t %d \t", getpid(), getppid());
    printf("Uid %5d \t Gid %5d\n", getuid(), getgid());
    sleep(5);
  }

}
