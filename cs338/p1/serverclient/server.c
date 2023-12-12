#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define FIFO_PERM (S_IRUSR | S_IWUSR)

int main(int argc, char **argv)
{
  if (argc != 1) {
    fprintf(stderr, "Usage: ./server\n");
    return 1;
  }

  char* requestFifoName = "requestFifo";
  if (mkfifo(requestFifoName, FIFO_PERM) == -1) {
    if (errno != EEXIST) {
      fprintf(stderr, "[%ld]:failed to create named pipe %s\n",
                      (long)getpid(), requestFifoName);
      return 1;
    }
  }

  int requestFD;
  char request[300];
  while ((requestFD = open(requestFifoName, O_RDWR | O_NONBLOCK)) == -1);
  printf("opened\n");
  while (read(requestFD, request, 300) < 1);
  printf("%s\n", request);

  // while ()
  //
  // char* clientName = argv[1];
  // int fd;
  // while (((fd = open("requestFifo", O_WRONLY)) == -1)) ;
  // pid_t processID = getpid();
  // write(fd, &processID, sizeof(pid_t));
}
