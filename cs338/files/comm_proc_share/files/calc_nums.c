#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(stderr, "Error creating nums\n");
    exit(1);
  }
  
  int num = atoi(argv[2]);

  FILE *file = fopen(argv[1], "w");
  if(file == NULL){
    fprintf(stderr, "Error opening file to write\n");
    exit(2);
  }
  srand(time(NULL));
  for(int i = 0; i < num; i++){
    fprintf(file, "%d ", rand() % 10);
  }
  fprintf(file, "\n");

  fclose(file);

  return 0;
}
 
