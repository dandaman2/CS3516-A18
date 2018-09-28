#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

//global for intercepting ^c
int serv_sock = -1;

//signal handler function
void signalHandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     printf("Terminate Server? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y')
          exit(0);
     else
          signal(SIGINT, signalHandler);
}

//error messaging function
void error_out(char *errorMessage)
{
    printf("%s", errorMessage);
    exit(1);
}

//function for sending data to the client
int send_dat(int socketfd, long sz, FILE *fp, time_t t){
	char status[128];
	char date[128];
	char server[128];
	char length[128];
	char type[128];

	sprintf(status, "HTTP/1.1 200 OK\r\n");
	sprintf(date, "Date: %s", ctime(&t));
	sprintf(server, "Server: (Unix)\r\n");
	sprintf(length, "Content-Length: %ld\r\n", sz);
	sprintf(type, "Content-Type: text/html; charset=utf-8\r\n\r\n");

	char *fbuf = malloc(sz+1);
	fread(fbuf,1,sz,fp);
	int sendbufsize = strlen(fbuf) + strlen(status) + strlen(date) +
	 		strlen(server) + strlen(length) + strlen(type);
	char all_buf[sendbufsize];
	memset(&all_buf, '\0', sizeof all_buf);


	snprintf(all_buf, sizeof( all_buf ), "%s%s%s%s%s%s", status, date, server, length, type, fbuf);
	int len = strlen(all_buf);
	send(socketfd, all_buf, len, 0);
	free(fbuf);
	fclose(fp);
	return 0;
}

int recv_and_reply(int socketfd, int buff_size){
	time_t t;
	time(&t);
	char in_buf[buff_size];
	memset(&in_buf, '\0', sizeof in_buf);
	int bytes_count = recv(socketfd, in_buf, sizeof in_buf, 0);
	in_buf[bytes_count] = '\0';

	char* furl = strchr(in_buf, ' ') + 1;
	*(strchr(furl, ' ')) = 0;
	char* url = malloc(strlen(furl) + 1);
	strcpy(url, furl);


	char * c_url;
	c_url = malloc(1+strlen(url)+1);
	c_url[0] = '\0';
	strcat(c_url, ".");
	strcat(c_url, url);

	if(strcmp(url,"/") == 0 || strcmp(url,"/index.html")==0){
		FILE *fp = fopen("./TMDG.html", "r");
	    fseek(fp, 0L, SEEK_END);
	    long sz = ftell(fp);
	    rewind(fp);
	    send_dat(socketfd, sz, fp, t);

	}
	else{
		FILE *fp = fopen(c_url, "r");
		if(fp == NULL){
			char *resp404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
			char target[buff_size];
			memset(&target, '\0', sizeof target);
			snprintf(target, sizeof( target ), "%s", resp404);
			int len = strlen(target);
			send(socketfd, target, len, 0);

		}
		else{
			FILE *fp = fopen("./TMDG.html", "r");
			fseek(fp, 0L, SEEK_END);
			long sz = ftell(fp);
			rewind(fp);
			send_dat(socketfd, sz, fp, t);
		}

	}
	free(c_url);
	free(url);
	return 0;
}

void onExit(void);
int main(int argc, char* argv[]){
	signal(SIGINT, signalHandler);
	atexit(onExit);
	int backlog = 20;
	int BUFF_SIZE =20560;
	//The socket ids for server and client,
	//as well as the port number
	int clnt_sock;
	char* use_port;
	struct addrinfo *res;
	struct addrinfo hints;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;

	if (argc != 2){
	error_out("Incorrect arguments\n");
	}
	use_port = strdup(argv[1]); //the port to use

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, argv[1], &hints, &res);

	//make socket
	serv_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(serv_sock<0){
		error_out("Bad Socket Creation\n");
	}

	//bind to the port from getaddrinfo
	int bc = bind(serv_sock, res->ai_addr, res->ai_addrlen);
	if(bc<0){
		error_out("Bind unsuccessful\n");
	}

	//listen for incoming connections
	if(listen(serv_sock, backlog) < 0){
		error_out("Listening unsuccessful");
	}
	printf("Listening Now... port %s up and running\n", use_port);

	while(1){
		//accept incoming connection
		addr_size = sizeof their_addr;
		clnt_sock = accept(serv_sock, (struct sockaddr *)&their_addr, &addr_size);
		if(clnt_sock<0){
			error_out("Acceptance Error\n");
		}

		if(fork() == 0){
			recv_and_reply(clnt_sock, BUFF_SIZE);
			close(clnt_sock);
			exit(0);
		}
		close(clnt_sock);
	}
	return 0;
}
void onExit(void){
	if(serv_sock <-1){
		close(serv_sock);
	}
}

