client:	http_client.c
	gcc -Wall -o http_client http_client.c
server: http_server.c
	gcc -Wall -pthread -o http_server http_server.c
all:	http_client.c http_server.c
	gcc -Wall -o http_client http_client.c
	gcc -Wall -pthread -o http_server http_server.c
clean:
	rm http_client
	rm http_server