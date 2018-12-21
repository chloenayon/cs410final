#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include "server.h"
#include <unistd.h>
#include <time.h>


void servConn (int port) {

  int sd, new_sd;
  struct sockaddr_in name, cli_name;
  int sock_opt_val = 1;
  int cli_len;
  char data[80];		/* Our receive data buffer. */
  const char f_slash[2] = "/";
  char *parsed_data;
  
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("(servConn): socket() error");
    exit (-1);
  }

  if (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, (char *) &sock_opt_val,
		  sizeof(sock_opt_val)) < 0) {
    perror ("(servConn): Failed to set SO_REUSEADDR on INET socket");
    exit (-1);
  }

  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (bind (sd, (struct sockaddr *)&name, sizeof(name)) < 0) {
    perror ("(servConn): bind() error");
    exit (-1);
  }

  listen (sd, 5);

  for (;;) {
      cli_len = sizeof (cli_name);
      new_sd = accept (sd, (struct sockaddr *) &cli_name, &cli_len);
      printf ("Assigning new socket descriptor:  %d\n", new_sd);
      
      if (new_sd < 0) {
	perror ("(servConn): accept() error");
	exit (-1);
      }

      if (fork () == 0) {	/* Child process. */
	close (sd);
	read (new_sd, &data, 25); /* Read our string: "Hello, World!" */
	printf ("Received string = %s\n", data);

	handle_info(&data);
	
	parsed_data = strtok(data, f_slash);
	parsed_data = strtok(NULL, f_slash);
	
	printf("data received is: %s\n", parsed_data);
	
	exit(0);
      }
  }
}

int err_404(){
  char *resp = "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: 44\n\n<!DOCTYPE html> <html> <body> <h1> ERROR 4O4: NOT FOUND </h1></body> </html>\n";
}

int err_501(){
  char *resp = "HTTP/1.1 501 Not Implemented\nContent-Type: text/html\nContent-Length: 44\n\n<!DOCTYPE html> <html> <body> <h1> ERROR 501: NOT IMPLEMENTED </h1></body> </html>\n";
}

int get_file(char *fname, char *listing){

  struct stat buf;
  char *pathname = malloc(128);
  pathname = fname;
  char *info = malloc(256);
  char *time = malloc(256);
  int fp;
  
  strcat(info, "   ");
  printf("pathname is: %s\n", pathname);

  int res = lstat(pathname, &buf);

  if (res != 0){
    printf("Error: stat failed\n");
  }

  switch (buf.st_mode & S_IFMT) {
  case S_IFBLK:
    strcat(info, "Block Device");
    break;
  case S_IFCHR:
    strcat(info, "Character Device");
    break;
  case S_IFDIR:
    strcat(info, "Directory");
    break;
  case S_IFIFO:
    strcat(info, "FIFO");
    break;
  case S_IFLNK:
    strcat(info, "Symlink");
    break;
  case S_IFREG:
    strcat(info, "Regular File");
    break;
  case S_IFSOCK:
    strcat(info, "Socket");
    break;
  default:
    strcat(info, "Unknown");
    break;
  }

  strcat(info, "  ");

  strftime(time, 50, "%B %d %Y , %I:%M:%S", localtime(&(buf.st_ctime)));

  printf("date is: %s\n", time);
  strcat(info, time);

  strcat(info, "   ");

  char* num = malloc(64);

  sprintf(num, "%d", buf.st_size);
  printf("size is: %d\n", buf.st_size);

  strcat(info, num);
  
  strcat(listing, info);
  strcat(listing, "\n");

  //free(pathname);
  //free(info);
}

int dir_req(char *fname){

  DIR *thisDir;
  struct dirent *entry;

  printf("THIS IS DIRECTORY!\n");

  char *listing = malloc(1024);
  thisDir = opendir(fname);

  if (thisDir == NULL){
    printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
    err_404();
    return 1;
  }

  entry = readdir(thisDir);
  char *e;
  char *thisdir = malloc(256);
  
  while (entry != NULL){
    e = entry->d_name;
    if ((strcmp(e, ".git") != 0) && (strcmp(e, ".") != 0) && (strcmp(e, "..") != 0)){
      strcpy(thisdir, "");
      printf("current entry is: %s\n", e);

      strcat(thisdir, fname);
      strcat(thisdir, "/");
      strcat(thisdir, e);

      printf("Adding e to listing which is: %s\n", e);
      
      strcat(listing, e);

      get_file(thisdir, listing);
    }
    entry = readdir(thisDir);
  }

  printf("full listing is: \n %s\n", listing);

}

int html_req(char *fname){

  printf("This is the file: %s!\n", fname);

  int fd = open(fname, O_RDONLY, 0777);
  char *html = malloc(2048);

  if (fd == -1){
    printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
    err_404();
    return 1;
  }

  read(fd, html, 2048);

  printf("this is the content: %s\n", html);
  
  close(fd);
  free(html);

}

int jpg_req(char *fname){

  printf("This is the file: %s!\n", fname);

  FILE *fd;
  fd = fopen(fname, "r");
  long size;
  char *buf = 0;
  
  if (fd == NULL){
    printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
    err_404();
    return 1;
  }

  fseek(fd, 0, SEEK_END);
  size = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  
  buf = malloc(size);
}


int cgi_req(char *fname){

  printf("in cgi-req, fname is %s\n", fname);
  
  char *buf = malloc(2048);
  FILE *fp;
  char *cmd = malloc(64);//"./";
  strcat(cmd, "./");
  strcat(cmd, fname);

  fp = popen(cmd, "r");
  
  if (fp == NULL){
    printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
    err_404();
    return 1;
  }

  char *fin = malloc(256);

  while (fgets(buf, 31, fp) != NULL){
    //printf("%s\n", buf);
    strcat(fin, buf);
  }

  printf("RESULT: %s\n", fin);

  pclose(fp);
  free(buf);
}


int handle_gnuplot(char *pname){
  
  count_files(pname);
  
}

int handle_info(char *data){

  char *fullfile = malloc(sizeof(data));
  strcpy(fullfile, data);
  char *filename[2];
  char *token;
  char pd = '.';

  token = strtok(data, &pd);
  filename[0] = token;

  if (token != NULL){
    printf("TOKEN IS: %s\n", token);
    token = strtok(NULL, &pd);
    filename[1] = token;
  }

  if (filename[1] == NULL) {                        // DIRECTORY

    //handle_gnuplot(fullfile);
    
    dir_req(filename[0]);

  } else if (!strcmp(filename[1], "html")){         // HTML FILE 

    html_req(fullfile);

  } else if (!strcmp(filename[1], "jpg") || !strcmp(filename[1], "jpeg") || !strcmp(filename[1], "gif")) {                                    // STATIC IMAGE

    jpg_req(fullfile);
    
  } else if (!strcmp(filename[1], "py")) {                                    // CGI SCRIPT
    
    cgi_req(fullfile);

    //} else if () {                                    // PROGRAM & HTML FORMATTER
  } else {                                          // GNUPLOT IMAGE
    
    handle_gnuplot(fullfile);

  }


  
}


int main () {
  
  servConn (5050);		/* Server port. */

  return 0;
}
