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

#define MAX_LINE 1024

int main(){
	//time
	time_t rawtime;
	struct tm * timeinfo;
	unsigned char stringtime[MAX_LINE];

	int s; //socket
	struct addrinfo h;
	struct addrinfo *r;

	memset(&h, 0, sizeof(struct addrinfo));
	getaddrinfo("192.168.56.110", "3792", &h, &r);

	printf("PSIR 22L Lab1, exercise 1: Simple client\n");

	//create socket
	s=socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if(s==-1){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-2);
		exit(EXIT_FAILURE);
	}

	//connect to server
	if(connect(s, r->ai_addr, r->ai_addrlen)!=0){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-1);
		exit(EXIT_FAILURE);
	}
	for(;;){
		//create current time string
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		snprintf(stringtime, MAX_LINE, asctime(timeinfo));

		//send packet
		printf("Sending current time: %s", stringtime);
		if(send(s, stringtime, strlen(stringtime), 0) < 0){
			fprintf(stderr, "ERROR while trying to send packet (%s:%d)\n", __FILE__, __LINE__-1);
			exit(EXIT_FAILURE);
		}
		//sleep for 1030 ms
		usleep(1030000);

	}

	//clean up
	freeaddrinfo(r);
	close(s);

	return 0;
}
