create:
	gcc -c -o my-histogram.o my-histogram.c
	ar rs libserver.a my-histogram.o
	gcc -o tcp_server tcp_server.c libserver.a 
	gcc -o tcp_client tcp_client.c libserver.a 

clean:
	rm my-histogram tcp_server tcp_client libserver.a
