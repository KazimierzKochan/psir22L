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
#define SERVER_PORT "19398"
#define MAX_CLIENTS 100
#define MAX_LINE 1024

int genRndStrWithTimestamp(char *where, int len);

int main(){
	struct addrinfo h, *r, c;
	int s, new_s, c_len=sizeof(c);
	unsigned char message[MAX_BUF];
	unsigned char t[MAX_BUF];
	unsigned char mip_str[INET_ADDRSTRLEN];
	int clients[MAX_CLIENTS];

	srand(time(NULL));

	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family=PF_INET;
	h.ai_socktype=SOCK_DGRAM;
	h.ai_flags=AI_PASSIVE;

	memset(&clients, 0, MAX_CLIENTS*sizeof(int));

	getaddrinfo(NULL, SERVER_PORT, &h, &r);

	printf("PSIR 22L Lab1, exercise 2: Simple UDP server\n");


	printf("Random string generator test [HMSrand, 18 chars]\n");
	int i, res;
	char test[MAX_LINE];
	memset(&test, 0, MAX_LINE*sizeof(test[0]));
	for(i = 0; i < 5; i++){

		res = genRndStrWithTimestamp(test, 18);
		if(res == 0){
			printf("Test %d: \"%s\"\n", i+1, test);
		}
		sleep(rand() % 5);
	}


// Work in progress
/*
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
*/
	return 0;
}

int genRndStrWithTimestamp(char *where, int len){
	time_t rawtime;
	struct tm *timeinfo;
	int hours, minutes, seconds, day, month, year;
	int i;
	char *lookUp = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int range = strlen(lookUp);

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(where, MAX_LINE, "%02H%02M%02S", timeinfo);
	//%Y, %m, %d, format 02

	if(len < strlen(where)){
		return 1;
	}

	for(i = strlen(where); len > i; i++){
		where[i] = lookUp[(rand() % range)];
	}

	return 0;
}
