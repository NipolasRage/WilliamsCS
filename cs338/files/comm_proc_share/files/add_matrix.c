#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(stderr, "Error creating nums\n");
    exit(1);
  }
  
  int num = atoi(argv[2]);
  int vector_size = num / 2;
  int *a_vector = (int*)malloc(sizeof(int)*vector_size);
  int *b_vector = (int*)malloc(sizeof(int)*vector_size); 
  int *c_vector = (int*)malloc(sizeof(int)*vector_size); 
  
  int fd = 0;
  while(fd= open(argv[1], O_RDONLY) == -1)
    ;
  close(fd);
  
  FILE *file = fopen(argv[1], "r");
  if(file == NULL){
    fprintf(stderr, "add_matrix Error opening file to read\n");
    exit(2);
  }
  
  for(int i = 0; i < vector_size; i++){
    fscanf(file, "%d", &a_vector[i]);
  }
  for(int i = 0; i < vector_size; i++){
    fscanf(file, "%d", &b_vector[i]);
  }
  fclose(file);
  for(int i = 0; i < vector_size; i++){
    c_vector[i] = a_vector[i] + b_vector[i];
  }

  file = fopen(argv[1], "w");
  for(int i = 0; i < vector_size; i++){
    fprintf(file, "%d ", a_vector[i]);
  }
  fprintf(file, "\n");
  for(int i = 0; i < vector_size; i++){
    fprintf(file, "%d ", b_vector[i]);
  }
  fprintf(file, "\n");
  for(int i = 0; i < vector_size; i++){
    fprintf(file, "%d ", c_vector[i]);
  }
  fprintf(file, "\n");

  fclose(file);

  return 0;
}
 
