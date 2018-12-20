#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define REQLEN 2048
#define RESLEN 2048
#define MAXCON 10
#define NOTFOUND  404
#define Ok 200
#define NOTIMPL  500
int err_404(){
  char *resp = "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: 44\n\n<!DOCTYPE html> <html> <body> <h1> ERROR 4O4: NOT FOUND </h1></body> </html>\n";
}

int err_501(){
  char *resp = "HTTP/1.1 501 Not Implemented\nContent-Type: text/html\nContent-Length: 44\n\n<!DOCTYPE html> <html> <body> <h1> ERROR 501: NOT IMPLEMENTED </h1></body> </html>\n";
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

  while (entry != NULL){
    e = entry->d_name;
    //printf("current entry is: %s\n", e);
    strcat(listing, e);
    strcat(listing, "\n");
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

int  buildResponse(int type,char *s1, char *s2, int len){
    	switch (type) {
	case OK:
		(void)sprintf(buffer,"HTTP/1.1 200 OK\nServer: nweb/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", VERSION, len, fstr);
		break;
	case NOTFOUND:
		(void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
		break;
	case NOTIMPL:
        (void)write(socket_fd, "HTTP/1.1 500 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
		break;
	}
}
void *connection_handler(void *);
void servConn(int port)
{

    int sd, new_sd;
    struct sockaddr_in name, cli_name;
    int sock_opt_val = 1;
    int cli_len;
    char method[BUFF];  /* request method */
    char url[BUFF];     /* request uri */
    char version[BUFF]; /* request method */
    char data[BUFF];    /* Our receive data buffer. */
    FILE *fp;
    char buf[BUFF];
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("(servConn): socket() error");
        exit(-1);
    }

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_opt_val,
                   sizeof(sock_opt_val)) < 0)
    {
        perror("(servConn): Failed to set SO_REUSEADDR on INET socket");
        exit(-1);
    }

    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sd, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        perror("(servConn): bind() error");
        exit(-1);
    }

    listen(sd, 5);
    pthread_t thread_id;

    for (;;)
    {
        cli_len = sizeof(cli_name);
        new_sd = accept(sd, (struct sockaddr *)&cli_name, &cli_len);
        //printf("Assigning new socket descriptor:  %d\n", new_sd);
        close(sd);
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&new_sd) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        pthread_join(thread_id, NULL);
        puts("Handler assigned");

        if (new_sd < 0)
        {
            perror("accept failed");
            return 1;
        }
    }
    return 0;
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int *)socket_desc;
    int read_size;
    char *message, request[2000];
    FILE *fp;
    char *responseHead;
    char *responseBody;
    char *method, *url, *version;
    const char *buildResponse;

    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock, message, strlen(message));

    message = "Now type something and i shall repeat what you type \n";
    write(sock, message, strlen(message));

    //Receive a message from client
    while ((read_size = recv(sock, request, 2000, 0)) > 0)
    {
        //end of string marker
        request[read_size] = '\0';

        //Send the message back to client
       // printf(request);
        respond_to_client(request, &message);
          
    //printf("*************\n%s**************\n",request);
    char *firstline = strtok(request, "\n");
    printf("*************\n%s**************\n",firstline);
    sscanf(firstline, "%s %s %s\n", method, url, version);

    //printf("-------------------\nReceived string = %s\n-----------------------\n", request);


    if (strcmp(method, "GET"))
    {
        //fp = fopen(url, "r+");
        buildResponse(int type,char *s1, char *s2, int len)

}

        write(sock, request, strlen(request));

        //clear the message buffer
        memset(request, 0, 2000);
    }

    if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}

int main(int argc, char const **argv)
{
    if (argc <= 1)
    {
        printf("Port number is required! Please provide a port number as an argument!");
        return 1;
    }
    else
    {
        int port = atoi(argv[1]);
        printf("Server listening on port %d\n You can connect to it at  http://localhost:%d/\n", port,port);
        servConn(port);
        return 0;
    }
}
