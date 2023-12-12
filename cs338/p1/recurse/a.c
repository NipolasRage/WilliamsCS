#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#define FIFO_PERM (S_IRUSR | S_IWUSR)

// download book's header file
ssize_t r_write(int fd, void *buf, size_t size) {
  char *bufp;
  size_t bytestowrite;
  ssize_t byteswritten;
  size_t totalbytes;

  for (bufp = buf, bytestowrite = size, totalbytes = 0;
       bytestowrite > 0;
       bufp += byteswritten, bytestowrite -= byteswritten) {
    byteswritten = write(fd, bufp, bytestowrite);
    if ((byteswritten == -1) && (errno != EINTR))
      return -1;
    if (byteswritten == -1)
      byteswritten = 0;
    totalbytes += byteswritten;
  }
  return totalbytes;
}

int main(int argc, char **argv)
{
  if(argc != 2) {
    fprintf(stderr, "Usage: ./recurse n\n");
    return 1;
  }

  int n = atoi(argv[1]);
  int numberRead;
  pid_t childpid = 0;
  char pipeName[256];
  int childNum = 0;
  char buffer[strlen(argv[1])];
  
  for (int i = 1; i < n+1; i++) {
    if (childpid = fork()) break;
    else {
      childNum = i;
      printf("just made a kid\n");
      sprintf(pipeName, "fifo%d", i);
      if (mkfifo(pipeName, FIFO_PERM) == -1) {
	if (errno != EEXIST) {
	  fprintf(stderr, "[%ld]:failed to create named pipe %s\n",
		  (long)getpid(), pipeName);
	  return 1;
	}
      }
    }
  }

  // if main process
  if (childNum == 0) {
    int fd;
    while (((fd = open("fifo1", O_RDWR)) == -1) && (errno == EINTR)) ;
    write(fd, argv[1], strlen(argv[1]));
    //printf("wrote %s\n", argv[1]);
    char result[strlen(argv[1])];
    printf("waiting\n");
    wait(NULL);
    printf("reading\n");
    read(fd, result, strlen(argv[1]));
    printf("%s!!!!!!!!!!!!\n", result);
    close(fd);
    for (int i = 1; i < n+1; i++) {
      sprintf(pipeName, "fifo%d", i);
      unlink(pipeName);
    }
    return 0;
  }

  if (childNum > 0) {
    int fd;
    while (((fd = open(pipeName, O_RDWR)) == -1) && (errno == EINTR)) ;
    while (read(fd, buffer, strlen(argv[1])) == 0) {
      printf("reading from %s\n", pipeName);
    }
    
    numberRead = atoi(buffer);
    printf("child %d just read the number %d\n", childNum, numberRead);
    if (numberRead == 1) {
      sprintf(buffer, "%d", 1);
      printf("Base case: writing %s up to %s\n", buffer, pipeName);
      write(fd, buffer, strlen(buffer));
    }
    else {
      char number[strlen(argv[1])];
      char destination[strlen(argv[1])];
      sprintf(destination, "fifo%d", childNum + 1);
      sprintf(number, "%d", numberRead - 1);
      int strsize = strlen(number) + 1;
      int fd2;
      printf("Writing %s to %s\n", number, destination);
      while (((fd2 = open(destination, O_RDWR)) == -1) && (errno == EINTR)) ;

      write(fd2, number, strsize);
      wait(NULL);
      char result[strlen(argv[1])];
      while (read(fd2, result, strlen(argv[1])) == 0 || (strcmp(number, result) == 0 && atoi(result) != 1)) ;
      printf("read %d, child %d from %s\n", atoi(result), childNum, destination);
      sprintf(result, "%d", numberRead + atoi(result));
      printf("child %d is writing number %d up\n", childNum, atoi(result));
      write(fd, result, strlen(result));
      close(fd2);
    }
    close(fd);
    return 0;
  } 
}
