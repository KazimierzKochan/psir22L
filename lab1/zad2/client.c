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

#define MAX_BUF 128
#define SERVER "192.168.56.110"
#define SERVER_PORT "19398"
#define HELLO_MESSAGE 'h'
#define NORMAL_MESSAGE 'm'
#define GOODBYE_MESSAGE 'g'

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

	unsigned char send_m[MAX_BUF];
	unsigned char recv_m[MAX_BUF];
	int s; //socket
	int pos;
	struct addrinfo h, *r; //hints, res
	struct sockaddr_in c;
	int c_len = sizeof(c);
	char message_t;

	//uptime
	time_t start_time;
	double time_elapsed;
	time(&start_time);

	//select
	fd_set readfds;
	struct timeval tv;
	int res_select;
	tv.tv_sec=0;
	tv.tv_usec=5000;

	//set addr info
	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family = PF_INET;
	h.ai_socktype = SOCK_DGRAM;

	if(getaddrinfo(SERVER, SERVER_PORT, &h, &r) != 0){
		fprintf(stderr, "ERROR: Couldn't  get server info (%s:%d)\n", __FILE__, __LINE__-1);
		exit(EXIT_FAILURE);
	}

	printf("PSIR 2022L Lab1, exercise 2: Simple UDP client\n");
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

	//send hello message
	message_t = HELLO_MESSAGE;
	snprintf(send_m, MAX_BUF, "%cHELLO!", message_t);
	printf("Sending HELLO message\n");
	pos = sendto(s, send_m, strlen(send_m), 0, r->ai_addr, r->ai_addrlen);
	if(pos < 0){
		fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-2);
		exit(EXIT_FAILURE);
	}

	//wait for server messages, reply
	printf("Waiting for server messages...\n");
	while(!done){
		//select so that recv doesnt block program
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);

		res_select = select(s+1, &readfds, NULL, NULL, &tv);
		if(r < 0){
			fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-2);
			exit(EXIT_FAILURE);
		}else if(res_select > 0){
			if(FD_ISSET(s, &readfds)){
				//receive message from server
				pos = recvfrom(s, recv_m, MAX_BUF, 0, (struct sockaddr*)&c, &c_len);
				if(pos < 0){
					fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-2);
					exit(EXIT_FAILURE);
				}

				recv_m[pos]='\0';
				message_t = recv_m[0];
				snprintf(recv_m, MAX_BUF, "%s", recv_m+1);

				if(message_t == NORMAL_MESSAGE){
					//print received message
					printf("Received message: \"%s\"\n", recv_m);

					//prepare end send response
					snprintf(send_m, MAX_BUF, "%cRe:%s", message_t, recv_m);
					printf("Sending message: \"%s\"\n", send_m);

					pos = sendto(s, send_m, strlen(send_m), 0, r->ai_addr, r->ai_addrlen);
					if(pos < 0){
						fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-2);
						exit(EXIT_FAILURE);
					}
				}
			}
		}else{
			//background
			//check if client should end its job
			time_elapsed = difftime(time(NULL), start_time);
			if(time_elapsed >= uptime && uptime != 0){
				done = true;
			}
		}
	}

	printf("Quitting...\n");
	printf("Sending GOODBYE message\n");
	snprintf(send_m, MAX_BUF, "%cGOODBYE!", GOODBYE_MESSAGE);

	pos = sendto(s, send_m, strlen(send_m), 0, r->ai_addr, r->ai_addrlen);
	if(pos < 0){
		fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-2);
		exit(EXIT_FAILURE);
	}
	printf("Client has worked for %.1f seconds\n", time_elapsed);
	//clean up
	freeaddrinfo(r);
	close(s);

	return 0;
}

