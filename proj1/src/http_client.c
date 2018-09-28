
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

//for error messaging
void error_out(char *errorMessage)
{
    printf("%s\n", errorMessage);
    exit(1);
}

//sends the GET request
//returns the number of bytes sent
int send_data(int socketfd, char* site_addr, char* site_fp, int buff_size){
	char* msg1 = "GET /";
	char* msg2 = " HTTP/1.1\r\nHost: ";
	char* msg3 = "\r\nConnection: close\r\n\r\n";
	if(site_fp == NULL){
		site_fp = "index.html";
	}

	char full_msg[buff_size];
	snprintf(full_msg, sizeof(full_msg), "%s%s%s%s%s", msg1, site_fp,
			msg2, site_addr, msg3);

	printf("\nMessage:\n %s\n", full_msg);
	int len = strlen(full_msg);
	return send(socketfd, full_msg, len, 0);
}

//Recieves the http message
//returns the bytes sent
unsigned int recv_data(int socketfd, int buff_size){
	unsigned int byte_sum;
	int i;
	char buf[buff_size];
	memset(&buf, '\0', sizeof buf);
	i = 1;
	printf("Returned:\n");
	while(i){
		i = recv(socketfd, buf, sizeof buf, 0);
		byte_sum+=i;
		buf[i] = '\0';
		//printf("bytes recv %d\n", i);
		printf("%s", buf);

	}
	printf("\n");
	return byte_sum;
}

//Timeout modification
void tout(int socketfd){
	//Timeout
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO,
			(const char*)&tv, sizeof tv);
}

int main(int argc, char *argv[]) {

	float rtt;
	int status;
	int poption = 0;
	int BUFF_SIZE = 20560;
	struct addrinfo hints;
	struct addrinfo *servinfo, *p;
	char ipstr[INET6_ADDRSTRLEN];
	char *full_addr, *port_num, *base_addr, *addr_fp;
	struct timeval start, end;

	if(argc < 3){
		error_out("Need arguments as follows: [options] address port#");
	}
	if(argc == 3){
		full_addr = strdup(argv[1]);
		port_num = strdup(argv[2]);
	}
	if(argc > 3){
		if(strcmp(argv[1], "-p") == 0){
		full_addr = strdup(argv[2]);
		port_num = strdup(argv[3]);
		poption = 1;
		}
		else{
			error_out("Option not known: [Use '-p' for printing]");
		}
	}

	printf("addr: %s pn: %s\n", full_addr, port_num);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	base_addr = strtok(full_addr, "/");
	addr_fp = strtok(NULL, "/");

	//printf("get addr with args %s and %s\n", argv[1], argv[2]);
	if ((status = getaddrinfo(full_addr, port_num, &hints, &servinfo)) != 0) {
	 fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
	 return 2;
	}
	printf("IP addresses for %s:\n\n", base_addr);

	for(p = servinfo ;p != NULL; p = p->ai_next) {
	void *addr;
	char *ipver;
	// get the pointer to the address itself,
	// different fields in IPv4 and IPv6:
	if (p->ai_family == AF_INET) { // IPv4
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
	addr = &(ipv4->sin_addr);
	ipver = "IPv4";
	} else { // IPv6
	struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
	addr = &(ipv6->sin6_addr);
	ipver = "IPv6";
	}
	// convert the IP to a string and print it:
	inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
	printf(" %s: %s\n", ipver, ipstr);
	}

	printf("status: %d\n", status);
	//socket connection
	int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	//tout(sockfd);

	gettimeofday(&start, NULL);
	//connect
	int connfd = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);

	if(sockfd < 0){
		error_out("Socketing Fault");
	}
	if(connfd < 0){
		error_out("Connection Fault");
	}
	gettimeofday(&end, NULL);

	int bytes_sent = send_data(sockfd, full_addr, addr_fp, BUFF_SIZE);
	unsigned int bytes_recv = recv_data(sockfd, BUFF_SIZE);
	printf("Bytes| Sent: %d | Recieved: %d\n", bytes_sent, bytes_recv);
	if(poption){
		rtt = (((end.tv_sec - start.tv_sec)*1000) + ((end.tv_usec - start.tv_usec)/1000));
		        printf("RTT in milliseconds: %f\n", rtt);
	}
	freeaddrinfo(servinfo);
	close(sockfd);
	return EXIT_SUCCESS;
}
