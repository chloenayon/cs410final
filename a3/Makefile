create:
	gcc -c -o my-histogram.o my-histogram.c
	ar rs libserver.a my-histogram.o
	gcc -o tcp_server tcp_server.c libserver.a 
	gcc -o tcp_client tcp_client.c libserver.a 

webserv: webserv.c
	gcc -pthread -o webserv  webserv.c

debug: webserv.c
	gcc -g -pthread -o debugWebserv  webserv.c

clean:
	rm my-histogram.o tcp_server tcp_client libserver.a debugWebserv webserv hist_info.dat
