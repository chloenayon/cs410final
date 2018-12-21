#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

int main(){

  char *entry = malloc(32);
  FILE *fd;
  
  fd = fopen("hist_info.dat", "rw+");

  if (fd == NULL){
    printf("ERROR: Creating hist_info.dat failed/n");
    return(1);
  }

  fprintf(fd, "0  Happy       50\n");
  fprintf(fd, "1  Regular     20\n");
  fprintf(fd, "2  Symbolic    30\n");
  fprintf(fd, "3  Block       2\n");
  fprintf(fd, "4  Special     0\n");
  fprintf(fd, "5  FIFO        67\n");
  fprintf(fd, "6  Socket      11\n");
  
  FILE *gnupipe = popen("gnuplot -persist","w");

  fprintf(gnupipe, "set terminal png\n");
  fprintf(gnupipe, "set output 'graph.png'\n");
  fprintf(gnupipe, "set title 'Filetype Histogram'\n");
  fprintf(gnupipe, "set boxwidth 0.5\n");
  fprintf(gnupipe, "set yrange[0:100]\n");
  fprintf(gnupipe, "set style fill solid\n");
  fprintf(gnupipe, "plot 'hist_info.dat' using 1:3:xtic(2) with boxes");

  close(pipe);
  return 0;
}
