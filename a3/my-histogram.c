/** Authors: Chloe Kaubisch, Gaspard Etienne, Adi Mikulic **/

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
/**#include "apue.h" **/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>


/** 0: regular, 1: directory, 2: symbolic link, 3: special character, 4: block special, 5: fifo, 6: socket **/
// traverse a file hierarchy with root p and search for string s

int traverse(char *p,  int *count){
  struct stat buf;
  char *pathname = p;

  int res = lstat(pathname, &buf);
  if (res < 0) {    // error, deal with this later
    return -1;
  }

  DIR *dirFile;                      // pointer to directory
  struct dirent *dirEnt;             // directory entry
  char *fname;
  char *new_path;

  if (S_ISREG(buf.st_mode)){         // is a regular file
    
    count[0] += 1;
    return 0;

  } else if (S_ISDIR(buf.st_mode)){  // is a directory

    count[1] += 1;

    if ((dirFile = opendir(pathname)) == NULL){
      return 1;
    }

    while ((dirEnt=readdir(dirFile)) != NULL){

      fname = dirEnt->d_name;
      new_path = malloc(strlen(fname) + strlen(pathname) + 2);

      if (!strcmp(fname, ".") || !strcmp(fname, "..")) {             // if this file is not "." or ".."
      } else {

        new_path = malloc(strlen(fname) + strlen(pathname) + 2);
        strcpy(new_path, pathname);
        strcat(new_path, "/");
        strcat(new_path, fname);

        traverse(new_path, count);

      }
    }
    return 1;
  } else if (S_ISLNK(buf.st_mode)){  // is a symbolic link & we are searching by symbolic link

    count[2] += 1;
    return 0;

  } else if (S_ISCHR(buf.st_mode)) {
    
    count[3] += 1;
    return 0;

  } else if (S_ISBLK(buf.st_mode)) {
    
    count[4] += 1;
    return 0;

  } else if (S_ISFIFO(buf.st_mode)) {

    count[5] += 1;
    return 0;

  } else {

    count[6] += 1;
    return 0;
  }
}


int count_files(char *pname){
  
  char *pathname = malloc(255);
  pathname = pname;
  int count[7] = {0};
  int *fd;
  
  int x;
  for (x = 0; x < 7; x++){
    //printf("Index %d is: %d\n", x, count[x]);
  }

  traverse(pathname, count);

  int i;
  for (i = 0; i < 7; i++){
    //printf("Index %d is: %d\n", i, count[i]);
  }

  fd = fopen("hist_info.dat", "wb+");

  if (fd == NULL){
    printf("ERROR: Creating hist_info.dat failed/n");
    return(1);
  }

  fprintf(fd, "0  Regular       %d\n", count[0]);
  fprintf(fd, "1  Directory     %d\n", count[1]);
  fprintf(fd, "2  Symlink    %d\n", count[2]);
  fprintf(fd, "3  Special       %d\n", count[3]);
  fprintf(fd, "4  Block     %d\n", count[4]);
  fprintf(fd, "5  FIFO        %d\n", count[5]);
  fprintf(fd, "6  Socket      %d\n", count[6]);

  FILE *gnupipe = popen("gnuplot -persist","w");

  fprintf(gnupipe, "set terminal png\n");
  fprintf(gnupipe, "set output 'graph.png'\n");
  fprintf(gnupipe, "set title 'Filetype Histogram'\n");
  fprintf(gnupipe, "set boxwidth 0.5\n");
  fprintf(gnupipe, "set yrange[0:100]\n");
  fprintf(gnupipe, "set style fill solid\n");
  fprintf(gnupipe, "plot 'hist_info.dat' using 1:3:xtic(2) with boxes");

  close(pipe);

  free(pathname);
  
  return 0;
}

