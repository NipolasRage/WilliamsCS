#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "Usage: ./client name\n");
    return 1;
  }
  int fd;
  while (((fd = open("requestFifo", O_WRONLY)) == -1));
  printf("opened\n");
  pid_t processID = getpid();
  char request[300];
  char* pronoun = argv[1];
  sprintf(request, "%d %s", processID, pronoun);
  write(fd, request, strlen(request) + 1);

  int responseFD;
  char responseFifo[300];
  sprintf(responseFifo, "fifo%d", processID);
  while (((responseFD = open(responseFifo, O_RDONLY)) == -1));
  char response[300];
  read(responseFD, response, 300);
  printf("%s\n", response);
  return 0;
}
