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
#include <time.h>

#define MAX_BUF 128
#define SERVER_IP "192.168.56.110"
#define SERVER_PORT "19398"

int main(){
	//time
	unsigned char send_m[MAX_BUF];
	unsigned char recv_m[MAX_BUF];
	int s; //socket
	int pos;
	struct addrinfo h, *r, c;
	int c_len = sizeof(c);

	printf("PSIR 2022L Lab1, exercise 2: UDP client\n");

	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family = PF_INET;
	h.ai_socktype = SOCK_DGRAM;

	if(getaddrinfo(SERVER_IP, SERVER_PORT, &h, &r) != 0){
		fprintf(stderr, "ERROR: Couldn't  get server info (%s:%d)\n", __FILE__, __LINE__-1);
		exit(EXIT_FAILURE);
	}

	//create socket
	s=socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if(s==-1){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-2);
		exit(EXIT_FAILURE);
	}

	//send hello message, flag=69 (HELLO)
	snprintf(send_m, MAX_BUF, "hHELLO!");
	printf("Sending HELLO message...\n");
	pos = sendto(s, send_m, strlen(send_m), 0, r->ai_addr, r->ai_addrlen);
	if(pos < 0){
		fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-2);
		exit(EXIT_FAILURE);
	}

	//wait for server messages, reply
	printf("Waiting for server messages...\n");
	for(;;){
		pos = recvfrom(s, recv_m, MAX_BUF, 0, (struct sockaddr*)&c, &c_len);
		if(pos < 0){
			fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-2);
			exit(EXIT_FAILURE);
		}

		recv_m[pos]='\0';
		snprintf("%s", MAX_BUF, recv_m+1);
		printf("Received message: \"%s\"\n", recv_m);
		snprintf(send_m, MAX_BUF, "mRe:%s", recv_m);

		printf("Sending message: \"%s\"\n", send_m);
		pos = sendto(s, send_m, strlen(send_m), 0, r->ai_addr, r->ai_addrlen);
		if(pos < 0){
			fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-2);
			exit(EXIT_FAILURE);
		}
	}

	//clean up
	freeaddrinfo(r);
	close(s);

	return 0;
}
