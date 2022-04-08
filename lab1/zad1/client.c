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
#include <stdbool.h>

#define MAX_LINE 1024
#define SERVER "192.168.56.110"
#define SERVER_PORT "3792"

int main(int argc, char *argv[]){

	//assess arguments
	bool done = false;
	double uptime;
	if(argc <= 2){
		//if time given
		if(argc == 2){
			if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0){
				printf("Syntax: %s [uptime (0=infinity)]\n", argv[0]);
				exit(0);
			}else{
				uptime = atof(argv[1]);
			}
		}else{
			uptime = 0;
		}
	}else{
		printf("Syntax: %s [uptime (0=infinity)]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	//time info
	time_t raw_time;
	struct tm * time_info;
	unsigned char string_time[MAX_LINE];

	//uptime
	time_t start_time;
	double time_elapsed;
	time(&start_time);

	int s; //socket
	struct addrinfo h; //hints
	struct addrinfo *r; //res

	//get address info
	memset(&h, 0, sizeof(struct addrinfo));
	getaddrinfo(SERVER, SERVER_PORT, &h, &r);

	printf("PSIR 22L Lab1, exercise 1: Simple TCP client\n");
	if(uptime == 0){
		printf("Infinite loop\n");
	}else{
		printf("Planned uptime: %.1f seconds\n", uptime);
	}

	//create a socket
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

	//main loop
	while(!done){
		//create current time string
		time(&raw_time);
		time_info = localtime(&raw_time);
		snprintf(string_time, MAX_LINE, asctime(time_info));

		//send packet
		printf("Sending current time: %s", string_time);
		if(send(s, string_time, strlen(string_time), MSG_NOSIGNAL) < 0){
			fprintf(stderr, "ERROR while trying to send packet (%s:%d)\n", __FILE__, __LINE__-1);
			exit(EXIT_FAILURE);
		}

		//sleep for 1030 ms
		//no need to use non-blocking methods
		usleep(1030000);

		//check if client should end its job
		time_elapsed = difftime(raw_time, start_time);
		if(time_elapsed >= uptime && uptime != 0){
			done = true;
			printf("Client has worked for %.1f seconds\n", time_elapsed);
		}
	}

	printf("Quitting...\n");
	//clean up
	freeaddrinfo(r);
	close(s);

	return 0;
}

