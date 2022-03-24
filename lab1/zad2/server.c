//gcc -o server server.c -pthread
/*
PSIR 2022L
Monika Lewandowska, Kazimierz Kochan
Warsaw Univeristy of Technology
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX_BUF 128
#define SERVER_PORT "19398"
#define MAX_CLIENTS 100
#define MAX_LINE 1024
#define MESSAGE_LENGTH 18
#define HELLO_MESSAGE 'h'
#define NORMAL_MESSAGE 'm'
#define MIN_SLEEP 100
#define MAX_SLEEP 2310

int generate_random_string_with_timestamp(char *where, int len);
void get_time_string(char *where);
void *random_sleep_function(void *ptr);

int main(){
	srand(time(NULL));

	struct addrinfo h, *r=NULL;
	struct sockaddr_in c;
	int s, c_len=sizeof(c);

	unsigned char send_m[MAX_BUF];
	unsigned char recv_m[MAX_BUF];
	char message_t;
	int pos;

	unsigned char time_now[MAX_LINE];

	//self-pipe trick
	int self_pipe[2]; //two ends of pipe
	char ignore_pipe[MAX_BUF];

	//select
	fd_set readfds;
	struct timeval tv;
	int res_select;
	tv.tv_sec=0;
	tv.tv_usec=5000;

	//randomly sleeping thread
	pthread_t sleep_thread;

	//addrinfo
	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family=PF_INET;
	h.ai_socktype=SOCK_DGRAM;
	h.ai_flags=AI_PASSIVE;

	if(getaddrinfo(NULL, SERVER_PORT, &h, &r) != 0){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-1);
		exit(EXIT_FAILURE);
	}

	printf("PSIR 22L Lab1, exercise 2: Simple UDP server\n");

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

	//"Self-pipe trick"
	int pipe_res = pipe(self_pipe);
	if(pipe_res == -1){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-2);
		exit(EXIT_FAILURE);
	}
	//set non blocking
	if(fcntl(pipe_res, F_SETFL, O_NONBLOCK) == -1){
		fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-1);
		exit(EXIT_FAILURE);
	}

	//start thread that periodically sleeps
	get_time_string(time_now);
	printf("%s: Starting thread\n", time_now);
	//write end of pipe
	int thread_args[] = {self_pipe[1], MIN_SLEEP, MAX_SLEEP};
	int t_ret = pthread_create(&sleep_thread, NULL, random_sleep_function, (void*) thread_args);

	for(;;){
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		//read end of pipe
		FD_SET(self_pipe[0], &readfds);

		int max = (s >= self_pipe[0] ? s : self_pipe[0]);
		res_select = select(max+1, &readfds, NULL, NULL, &tv);
		if(r < 0){
			fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-2);
			exit(EXIT_FAILURE);
		}else if(res_select > 0){
			//if related to s then read
			if(FD_ISSET(s, &readfds)){
				//process received dgram
				pos = recvfrom(s, recv_m, MAX_BUF, 0, (struct sockaddr*)&c, &c_len);
				if(pos < 0){
					fprintf(stderr, "ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__-2);
					exit(EXIT_FAILURE);
				}

				if(recv_m[0] == HELLO_MESSAGE){

					get_time_string(time_now);
					printf("%s: Received Hello message ", time_now);
					recv_m[pos]='\0';
					printf("from (%s:%d): %s\n", inet_ntoa(c.sin_addr), ntohs(c.sin_port), recv_m+1);

					//add to local "database"
				}else if(recv_m[0] = NORMAL_MESSAGE){
					//w swojej konsoli prezentuje takie wiadomości dodając na jej początku 
					//datę i czas otrzymania pakietu UDP od zgłoszonych klientów
					get_time_string(time_now);
					recv_m[pos]='\0';
					printf("%s: Received message", time_now);
					printf(" from (%s:%d): %s\n", inet_ntoa(c.sin_addr), ntohs(c.sin_port), recv_m+1);
				}
			}
			else if(FD_ISSET(self_pipe[0], &readfds)) {
				read(self_pipe[0], ignore_pipe, MAX_BUF);

				get_time_string(time_now);
				printf("%s: Received pipe signal\n", time_now);
				printf("Pipe said: %s\n", ignore_pipe);
				//TODO: send dgram to random client
			}
		}else{
			//background
		}
	}

	//clean up
	close(s);
	freeaddrinfo(r);
	pthread_cancel(sleep_thread);

	return 0;
}

void get_time_string(char *where){
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(where, MAX_LINE, "%02H:%02M:%02S", timeinfo);
}

int generate_random_string_with_timestamp(char *where, int len){
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

void *random_sleep_function(void *ptr){
	int *thread_args = (int*)ptr;
	int min_t = thread_args[1];
	int max_t = thread_args[2];
	int random_time;

	unsigned char t_n[MAX_LINE];
	for(;;){
		random_time = (rand() % (max_t+1-min_t) + min_t );

		get_time_string(t_n);
		printf("%s: Start sleeping for %d ms\n", t_n, random_time);

		usleep(random_time*1000);

		get_time_string(t_n);
		printf("%s: Wake up\n", t_n);

		char pipe_says[] = "How you doin?";
		write(thread_args[0], pipe_says, strlen(pipe_says)+1);
	}
}
