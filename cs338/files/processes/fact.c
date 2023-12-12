#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  //  printf("in fact");
  if(argc > 1){
    int val = atoi(argv[1]);
    
    int result = 1;
    for(int i = 1; i <= val ; i++){
      result *= i;
    }
    printf("The factorial of %d is %d!\n", val, result);
  }

}
