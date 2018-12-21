create:
	gcc -c -o my-histogram.o my-histogram.c
	ar rs libserver.a my-histogram.o
	gcc -pthread -o tcp_server tcp_server.c libserver.a 
	gcc -o tcp_client tcp_client.c libserver.a 

webserv: webserv.c
	gcc -c -o my-histogram.o my-histogram.c
	ar rs libserver.a my-histogram.o
	gcc -pthread -o webserv  webserv.c libserver.a

debug: webserv.c
	gcc -g -c -o my-histogram.o my-histogram.c
	ar rs libserver.a my-histogram.o
	gcc -g -pthread -o debugWebserv  webserv.c libserver.a

clean:
	rm my-histogram.o tcp_server tcp_client libserver.a debugWebserv webserv hist_info.dat
