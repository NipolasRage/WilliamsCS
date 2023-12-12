#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int findSum(int n, int strlen2) {
  if (n < 1) {
    printf("n must be greater than 0\n");
    exit(1);
  }
  int p_child[2];
  int child_p[2];
  if (pipe(p_child) == -1 ||
        pipe(child_p) == -11) {
      fprintf(stderr, "Unable to open pipe\n");
      exit(1);
    }
  int childpid = fork();
  if (childpid == -1) {
    fprintf(stderr, "Error creating process \n");
	  exit(1);
  } else if (childpid == 0) {
    char input[strlen2];
    read(p_child[0], input, strlen2);
    int n = atoi(input);
    if (n == 1) {
      write(child_p[1], input, strlen2);
      exit(0);
    } else {
      n--;
      int x = findSum(n, strlen2);
      char output[strlen2];
      sprintf(output, "%d", n + x + 1);
      write(child_p[1], output, strlen2);
      //close(child_p[1]);
      exit(0);
    }
  } else {
    char output[strlen2];
    sprintf(output, "%d", n);
    write(p_child[1], output, strlen2);
    int status;
    waitpid(childpid, &status, 0);
    char response[strlen2];
    read(child_p[0], response, strlen2);
    close(child_p[0]);
    close(child_p[1]);
    close(p_child[1]);
    close(p_child[0]);
    return atoi(response);
  }
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "Usage: ./recurse n\n");
    return 1;
  }
  int n = atoi(argv[1]);
  int sum = findSum(n, strlen(argv[1]) + 1);
  printf("%d\n", sum);
}
