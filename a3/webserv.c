#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include "server.h"
#include <sys/sendfile.h>

#define REQLEN 2048
#define RESLEN 2048
#define MAXCON 10
#define NOTFOUND 404
#define Ok 200
#define NOTIMPL 500

char *err_404()
{
    char *resp = "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: 44\n\n<!DOCTYPE html> <html> <body> <h1> ERROR 4O4: NOT FOUND </h1></body> </html>\n";
    return resp;
}

char *err_501()
{
    char *resp = "HTTP/1.1 501 Not Implemented\nContent-Type: text/html\nContent-Length: 44\n\n<!DOCTYPE html> <html> <body> <h1> ERROR 501: NOT IMPLEMENTED </h1></body> </html>\n";
    return resp;
}
int get_file(char *fname, char *listing)
{

    struct stat buf;
    char *pathname = malloc(128);
    pathname = fname;
    char *info = malloc(256);
    char *time = malloc(256);
    int fp;

    strcat(info, "   ");
    printf("pathname is: %s\n", pathname);

    int res = lstat(pathname, &buf);

    if (res != 0)
    {
        printf("Error: stat failed\n");
    }

    switch (buf.st_mode & S_IFMT)
    {
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

    char *num = malloc(64);

    sprintf(num, "%d", buf.st_size);
    printf("size is: %d\n", buf.st_size);

    strcat(info, num);

    strcat(listing, info);
    strcat(listing, "\n");

    //free(pathname);
    //free(info);
}

int dir_req(char *fname, int sd)
{

    DIR *thisDir;
    struct dirent *entry;
    char *response = malloc(RESLEN + RESLEN);
    char *listing = malloc(RESLEN);
    thisDir = opendir(fname);

    if (thisDir == NULL)
    {
        printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
        listing = err_404();
        return 1;
    }

    entry = readdir(thisDir);
    char *e;
    char *thisdir = malloc(256);
    while (entry != NULL)
    {
        e = strcat(entry->d_name," <br>");
        //printf("current entry is: %s\n", e);
        if ((strcmp(e, ".git") != 0) && (strcmp(e, ".") != 0) && (strcmp(e, "..") != 0))
        {
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
    sprintf(response, "HTTP/1.1 200 OK \r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\nTHIS IS DIRECTORY %s :\r\n%s\r\n", strlen(listing), listing, fname);
    printf("full listing is: \n %s\n", listing);

    write(sd, response, strlen(response));
}

int html_req(char *fname, int sd)
{
    char *type = "200 OK";
    printf("This is the file: %s!\n", fname);

    int fd = open(fname, O_RDONLY, 0777);

    char *html = malloc(RESLEN);

    if (fd == -1)
    {
        printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
        html = err_404();
        return 1;
        type = "404 NOT FOUND";
    }

    read(fd, html, RESLEN);
    //write(sd,"check one two",14);
    printf("this is the content: %s\n", html);
    char *response = malloc(RESLEN + RESLEN);
    sprintf(response, "HTTP/1.1 %s\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s", type, RESLEN, html);
    printf("this is the response we are sending:\n%s\n", response);

    write(sd, response, strlen(response));

    free(response);
    free(html);
    close(fd);
    // printf("this is the num bytes written %d\n", bytes);
}

int jpg_req(char *fname, int sd)
{

    printf("This is the file: %s!\n", fname);

    //FILE *fd;
    int fd = open(fname, O_RDONLY, 0777);
    long size;
    char *buf = 0;

    if (fd == -1)
    {
        printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
        buf = err_404();
        return 1;
    }

    lseek(fd, 0, SEEK_END);
    size = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);

    buf = malloc(size);
    char *response = malloc(RESLEN);
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");
    printf("this is the response we are sending:\n%s\n", response);
    int bytes = write(sd, response, strlen(response));
    printf("we sent :%d bytes\n", bytes);
    int sent = sendfile(sd, fd, NULL, 10000);
    printf("sent \n%d\n", sent);

    close(fd);
}

int cgi_req(char *fname, int sd)
{

    printf("in cgi-req, fname is %s\n", fname);

    char *buf = malloc(2048);
    FILE *fp;
    char *cmd = malloc(64); //"./";
    strcat(cmd, "./");
    strcat(cmd, fname);
    char *fin = malloc(256);
    fp = popen(cmd, "r");

    if (fp == NULL)
    {
        printf("SOME ERROR HAS OCCURRED OR NO SUCH FILE FOUND!\n");
        fin = err_404();
        return 1;
    }

    while (fgets(buf, 31, fp) != NULL)
    {
        //printf("%s\n", buf);
        strcat(fin, buf);
    }
    char *response = malloc(RESLEN + RESLEN);
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s", strlen(fin), fin);
    write(sd, response, strlen(response));
    printf("RESULT: %s\n", fin);

    pclose(fp);
    free(buf);
}

int handle_gnuplot(char *pname, int sd)
{

    count_files(pname);
    jpg_req("graph.png", sd);
}

int handle_info(char *data, int sd)
{

  printf("data is: %s\n", data);
  
    char *fullfile = malloc(sizeof(data));
    strcpy(fullfile, data);
    char *filename[2];
    char *token;
    char pd = '.';
    
    int i = 0;
    char *op;
    char *dir;

    char *ext;

    int con = strstr(url, "?");

    if (con == NULL){
      
      op = url;
      char *x = strtok(url, ".");
      char *kv[2];
      i = 0;
      while (x != NULL){
	kv[i] = x;
	x = strtok(NULL, ".");
      }
      
      ext = kv[0];
      //op = url;                                                                                                                                                                                         
      
      printf("OP is: %s\n", op);
      printf("EXT is: %s\n", ext);
      
      return(0);
    
    } else {
    
      char *p = strtok (url, "?");
      char *array[2];
      
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
      
    }

    if (token != NULL)
      {
	printf("TOKEN IS: %s\n", token);
	token = strtok(NULL, ".");
	filename[1] = token;
	}
    
    printf("POINT A\n");
    
    if (!strcmp(op, "car_start"))
      { // DIRECTORY
	
	//handle_gnuplot(op);
	
        //dir_req(filename[0], sd);
      }
    else if (!strcmp(op, "listdir"))
    {
      dir_req(dir);
      
    }
    else if (!strcmp(ext, "html"))
    { // HTML FILE

        html_req(op, sd);
    }
    else if (!strcmp(ext, "jpg") || !strcmp(ext, "jpeg") || !strcmp(ext, "gif"))
    { // STATIC IMAGE

        jpg_req(op, sd);
    }
    else if (!strcmp(ext, "py") || !strcmp(ext, "cgi"))
    { // CGI SCRIPT

      if (filename[0] == "gethist"){

	handle_gnuplot(op);
	
      } else {
      
        cgi_req(op, sd);
      }
        //} else if () {                                    // PROGRAM & HTML FORMATTER
    }
    else
    { // GNUPLOT IMAGE

        handle_gnuplot(fullfile, sd);
    }
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int *)socket_desc;

    int read_size;
    char *message;
    //char *request = args->data;
    char request[REQLEN];
    FILE *stream;
    char *responseHead;
    char *responseBody;
    const char *buildResponse;
    char method[REQLEN];  /* request method */
    char uri[REQLEN];     /* request uri */
    char version[REQLEN]; /* request method */

    //Send some messages to the client
    //message = "Greetings! I am your connection handler\n";
    // write(sock, message, strlen(message));

    //message = "Now type something and i shall repeat what you type \n";
    // write(sock, message, strlen(message));

    //Receive a message from client

    if ((stream = fdopen(sock, "w+")) == NULL)
        perror("ERROR on fdopen\n");

    /* get the HTTP request line */
    fgets(request, REQLEN, stream);
    printf("this is the request : \n%s", request);
    sscanf(request, "%s %s %s\n", method, uri, version);
    /* {
        //end of string marker
        request[read_size] = '\0';

        //Send the message back to client
        // printf(request);

        // respond_to_client(request, &message);

        //printf("*************\n%s**************\n",request);
        char *firstline = strtok(request, "\n");
        printf("*************\n%s**************\n", firstline);
        sscanf(firstline, "%s %s %s\n", method, url, version);

        //printf("-------------------\nReceived string = %s\n-----------------------\n", request);

        if (strcmp(method, "GET"))
        {
            //fp = fopen(url, "r+");
            printf(request); */

    /*  printf("reached here\n");
    if ((strcmp(uri, "/") != 0) || (strcmp(uri, "index.html") != 0))
    {
        char *html = malloc(RESLEN);
        int index = open("index.html", O_RDONLY, 0777);
        read(index, html, RESLEN);
        char *response = malloc(RESLEN);
        char type[] = "200 OK";
        snprintf(response, RESLEN, "HTTP/1.1 %s\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s\r\n", type, RESLEN, html);
        write(sock, response, strlen(response));
    } */
    memmove(uri, uri + 1, strlen(uri));

    handle_info(uri, sock);
    //buildResponse(int type, char *s1, char *s2, int len);
    // }

    // write(sock, request, strlen(request));

    //clear the message buffer
    memset(request, 0, REQLEN);
    //}

    /*   if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    } */

    return 0;
}

void servConn(int port)
{

    int sd, new_sd;
    struct sockaddr_in name, cli_name;
    int sock_opt_val = 1;
    int cli_len;
    char data[REQLEN]; /* Our receive data buffer. */

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("(servConn): socket() error\n");
        exit(-1);
    }

    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_opt_val,
                   sizeof(sock_opt_val)) < 0)
    {
        perror("(servConn): Failed to set SO_REUSEADDR on INET socket\n");
        exit(-1);
    }

    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sd, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        perror("(servConn): bind() error\n");
        exit(-1);
    }

    listen(sd, MAXCON);
    pthread_t thread_id;

    for (;;)
    {
        cli_len = sizeof(cli_name);
        new_sd = accept(sd, (struct sockaddr *)&cli_name, &cli_len);
        printf("Assigning new socket descriptor:  %d\n", new_sd);

        //close(sd);
        // write(new_sd,"connected",9);
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&new_sd) < 0)
        {
            perror("could not create thread\n");
            exit(1);
        }

        //Now join the thread , so that we dont terminate before the thread
        pthread_join(thread_id, NULL);
        puts("Handler assigned\n");

        if (new_sd < 0)
        {
            perror("(servConn): accept() error\n");
            exit(-1);
        }
    }

    exit(0);
}

int main(int argc, char const **argv)
{
    if (argc <= 1)
    {
        printf("Port number is required! Please provide a port number as an argument!\n");
        return 1;
    }
    else
    {
        int port = atoi(argv[1]);
        printf("Server listening on port %d\n You can connect to it at  http://localhost:%d/\n", port, port);
        servConn(port);
        return 0;
    }
}
