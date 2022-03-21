/*
PSIR 2022L
Monika Lewandowska, Kazimierz Kochan
Warsaw Univeristy of Technology
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BUF 128
#define SERVER_PORT "3792"

int main(){
	int s, new_s; //sockets
	struct addrinfo h, *r;
	unsigned char mip_str[INET_ADDRSTRLEN];
	char message[MAX_BUF];

	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family=PF_INET;
	h.ai_socktype=SOCK_STREAM;
	h.ai_flags=AI_PASSIVE;

	getaddrinfo(NULL, SERVER_PORT, &h, &r);

	printf("PSIR 22L Lab1, exercise 1: Simple TCP server\n");

	//create a socket
	s=socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if(s==-1){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-2);
		exit(EXIT_FAILURE);
	}
	//bind
	if(bind(s, r->ai_addr, r->ai_addrlen)!=0){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-1);
		exit(EXIT_FAILURE);
	}
	//listen for the client
	printf("Listening for the client\n");
	if(listen(s, 1)!=0){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-1);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in their_addr;
	socklen_t addr_size=sizeof(their_addr);

	//accept client
	if((new_s=accept(s, (struct sockaddr *)&their_addr, &addr_size))==-1)
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-1);
	if(inet_ntop(AF_INET, &(their_addr.sin_addr), mip_str, INET_ADDRSTRLEN)!=NULL)
		printf("Client with IP: %s has connected. New sock desc.: %d\n", mip_str, new_s);
	//main loop
	for(;;){
		//receive data from client
		int result=recv(new_s, message, MAX_BUF, 0);
		if(result==0){
			printf("Client has disconneted\n");
			break;
			exit(EXIT_FAILURE);
		}else if(result<0){
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		//end message with a null char
		message[result]='\0';
		printf("Client sent data: %s", message);
	}

	//clean up
	close(s);
	freeaddrinfo(r);

	return 0;
}
