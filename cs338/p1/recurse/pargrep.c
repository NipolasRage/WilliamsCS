#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void grepFile(char* fileName, char* str) {
  FILE* file = fopen(fileName, "r");
  if(file == NULL){
    fprintf(stderr, "Error opening %s\n", fileName);
    return;
  }
  else{
    char buffer[300];
    char lineOutput[300];
    char result[300] = "";
    int count = 0;
    while (EOF != fscanf(file, "%[^\n]\n", buffer)) {
      count++;
      if (strstr(buffer, str) != NULL) {
	sprintf(lineOutput, "File name: %s Line number:%d Line:%s\n", fileName, count, buffer);
	strcat(result, lineOutput);
      }
    }
    fprintf(stderr, "%s", result);
    fclose(file);
  }
}

int main(int argc, char **argv)
{
  if(argc < 3) {
    printf("Usage: ./pargrep string file1 file2 filen\n");
  } else {
    char* str = argv[1];
    int fileIndex;
    int child_pid = 1;
    for(int i = 2; i < argc; i++) {
      if (child_pid > 0) {
	child_pid = fork();
	if (child_pid == -1) {
	  printf("Error creating process \n");
	  exit(1);
	} else if (child_pid == 0) {
	  fileIndex = i;
	}
      }
    }
    
    if (child_pid == 0) {
      grepFile(argv[fileIndex], str);
    } else {
      int status = 0;
      while ((int wait_pid = wait(&status)) > 0);
      //wait(NULL);
    }
    return 0;
  }
}
