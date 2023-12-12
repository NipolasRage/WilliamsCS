#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int generateNums()
{
  unlink("nums.txt");
  int child_id = fork();
  if(child_id == -1){
    printf("Error creating process \n");
    exit(1);
  }
  else if (child_id == 0) {
    printf("I am process %d\n", getpid());
    execl("./calc_nums", "calc_nums", "nums.txt", "18", NULL);
    // Should never get here
    return 0;
  } else {
    return child_id;
  }
}

int matrixAdd()
{
  int child_id = fork();
  if(child_id == -1){
    printf("Error creating process \n");
    exit(1);
  }
  else if (child_id == 0) {
    printf("I am process %d\n", getpid());
    execl("./add_matrix", "add_matrix", "nums.txt", "18", NULL);
    // Should never get here
    return 0;
  } else {
    return child_id;
  }
}

int main(int argc, char **argv)
{
  int children[2];
  children[0] = generateNums();
  children[1] = matrixAdd();

  if(children[0] == -1 || children[1] == -1){
    fprintf(stderr, "Error creating processes\n");
    exit(1);
  }

  for(int i = 0; i < 2; i++){
    waitpid(children[i], NULL, 0);
  }
  
  return 0;

}
