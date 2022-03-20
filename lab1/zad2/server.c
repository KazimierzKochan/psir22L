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
#define PORT 19398

int main(){
	struct addrinfo h, *r, c;
	int s, new_s, c_len=sizeof(c);
	unsigned char message[MAX_BUF];
	unsigned char t[MAX_BUF];
	unsigned char mip_str[INET_ADDRSTRLEN];

	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family=PF_INET;
	h.ai_socktype=SOCK_DGRAM;
	h.ai_flags=AI_PASSIVE;

	getaddrinfo(NULL, PORT, &h, &r);

	printf("PSIR 22L Lab1, exercise 2: Simple server UDP\n");

	//create socket
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
	for(;;){
		// receive data from client
		int result=recvfrom(s, message, MAX_BUF, 0, (struct sockaddr*)&c, &c_len );
		if(result==0){
			printf("Client has disconneted\n");
			exit(EXIT_FAILURE);
		}else if(result<0){
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(message[result-1] != '\0')
			message[result]='\0';

		printf("Client sent data: %s", message);
		snprintf(t, MAX_BUF, "Re: %s\n", message);
		if((result=sendto(s, t, strlen(t), 0, (const struct sockaddr *)&c, c_len ))<0){
		printf("ERROR: %s\n", strerror(errno));
		}
	}

	//clean up
	close(s);
	freeaddrinfo(r);

	return 0;
}
