#include <string.h>
#include <stdio.h>

int main(int argn, char *argv[]){

  char url[100] = "listdir?directory='/home/ckaubisch/cs410final/a3'";
  //char url[100] = "quokka.jpg";
  
  int i = 0;
  char *op;

  int con = strstr(url, "?");
  
  if (con == NULL){

    op = url;

    printf("OP is: %s\n", op);
    
    return(0);
    
  }

  char *p = strtok (url, "?");
  char *array[2];
  char *dir;
  
  while (p != NULL){
      array[i++] = p;
      p = strtok (NULL, "?");
  }

  for (i = 0; i < 2; ++i) 
    printf("%s\n", array[i]);


  char *x = strtok(array[1], "=");
  char *kv[2];
  i = 0;
  while (x != NULL){
    kv[i] = x;
    x = strtok(NULL, "=");
  }

  printf("kv[0] is: %s\n", kv[0]);

  dir = kv[0];
  op = array[0];
  
  printf("OPERATION IS: %s\n", op);
  printf("DIRECTORY IS: %s\n", dir);
  
  return 0;
}
