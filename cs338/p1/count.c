#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

char* lowerNoPunct(char* src, int punct) {
  char* dst = src;
  int j = 0;
  for (int i = 0; src[i]; i++) {
    if (punct && ispunct(src[i])) {
      continue;
    } else {
      dst[j] = tolower(src[i]);
      j++;
    }
  }
  dst[j] = 0;
  return dst;
}

char* lowercase(char* src) {
  char* dst = src;
  for (int i = 0; src[i]; i++) {
    dst[i] = tolower(src[i]);
  }
  return dst;
}

int main(int argc, char **argv)
{
  if(argc < 3) {
    printf("Usage: ./count string file\n");
  } else {
    char* str = lowerNoPunct(argv[1], 1);
    const int wordSize = 300;
    int main_to_child1[2];
    int child1_to_child2[2];
    int child2_to_child3[2];
    int child3_to_main[2];
    char buffer[wordSize];

    if (pipe(main_to_child1) == -1 ||
	pipe(child1_to_child2) == -1 ||
	pipe(child2_to_child3) == -1 ||
	pipe(child3_to_main) == -1) {
      fprintf(stderr, "Unable to open pipe\n");
      exit(1);
    }

    int child_pid = 1;
    int childNumber = 0;
    for (int i = 1; i < 4; i++) {
      if (child_pid > 0) {
	child_pid = fork();
	if (child_pid == -1) {
	  printf("Error creating process \n");
	  exit(1);
	} else if (child_pid == 0) {
	  childNumber = i;
	  break;
	}
      }
    }
    
    if (childNumber == 1) {
      close(main_to_child1[1]);
      close(child1_to_child2[0]);
      close(child2_to_child3[0]);
      close(child2_to_child3[1]);
      close(child3_to_main[0]);
      close(child3_to_main[1]);

      while(read(main_to_child1[0], buffer, wordSize)){
	if (write(child1_to_child2[1], lowerNoPunct(buffer, 0), wordSize) < 0) {
	  fprintf(stderr, "Error writing to child2\n");
	  return 1;
	}
      }

      close(main_to_child1[0]);
      close(child1_to_child2[1]);
      
      return 0;
    } else if (childNumber == 2) {
      close(child1_to_child2[1]);
      close(child2_to_child3[0]);
      close(main_to_child1[0]);
      close(main_to_child1[1]);
      close(child3_to_main[0]);
      close(child3_to_main[1]);
      
      while(read(child1_to_child2[0], buffer, wordSize)){
	if (write(child2_to_child3[1], lowerNoPunct(buffer, 1), wordSize) < 0) {
	  fprintf(stderr, "Error writing to child2\n");
	  return 1;
	} 
      }
      close(child1_to_child2[0]);
      close(child2_to_child3[1]);
      return 0;
    } else if (childNumber == 3) {
      close(child2_to_child3[1]);
      close(child3_to_main[0]);
      close(child1_to_child2[0]);
      close(child1_to_child2[1]);
      close(main_to_child1[0]);
      close(main_to_child1[1]);
      
      int count = 0;
      while(read(child2_to_child3[0], buffer, wordSize)){
	if (strcmp(str, buffer) == 0) count++;
      }
      char result[wordSize];
      sprintf(result, "The word %s appeared in %s %d times", str, argv[2], count);
      if (write(child3_to_main[1], result, wordSize) < 0) {
	  fprintf(stderr, "Error writing to child2\n");
	  return 1;
      }
      close(child2_to_child3[0]);
      close(child3_to_main[1]);
      return 0;
    } else {

    
      char* fileName = argv[2];
      FILE* file = fopen(fileName, "r");
      if(file == NULL){
	fprintf(stderr, "Error opening %s\n", fileName);
	return 1;
      } else {
	close(main_to_child1[0]);
	close(child3_to_main[1]);
	close(child1_to_child2[0]);
	close(child1_to_child2[1]);
	close(child2_to_child3[1]);
	close(child2_to_child3[0]);
	
	while (EOF != fscanf(file, "%s", buffer)) {
	  if (write(main_to_child1[1], buffer, wordSize) != 0) {
	  }
	}

	close(main_to_child1[1]);
	if (read(child3_to_main[0], buffer, wordSize) > 1)
	  printf("%s\n", buffer);
	
	close(child3_to_main[0]);
	
	int status = 0;
	int wait_pid = 0;
	while ( (wait_pid = wait(&status)) > 0);
      }
    }
  }
}
