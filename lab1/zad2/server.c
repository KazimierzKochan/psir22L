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
#include <stdbool.h>

#define MAX_BUF 128
#define SERVER_PORT "19398"
#define MAX_CLIENTS 100
#define MAX_LINE 1024
#define MESSAGE_LENGTH 18
#define HELLO_MESSAGE 'h'
#define NORMAL_MESSAGE 'm'
#define GOODBYE_MESSAGE 'g'
#define MIN_SLEEP 100
#define MAX_SLEEP 2310

int generate_random_string_with_timestamp(char *where, int len);
void get_time_string(char *where);
void *random_sleep_function(void *ptr);

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

	srand(time(NULL));

	struct addrinfo h, *r=NULL; //hints, res
	struct sockaddr_in c;
	int s; //socket
	int c_len=sizeof(c);
	struct sockaddr_in clients[MAX_CLIENTS];
	unsigned short clients_num = 0;

	//messages
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

	//uptime
	time_t start_time;
	double time_elapsed;
	time(&start_time);

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

	while(!done){
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		//read end of pipe
		FD_SET(self_pipe[0], &readfds);

		//find greater descriptor
		int max = (s >= self_pipe[0] ? s : self_pipe[0]);
		res_select = select(max+1, &readfds, NULL, NULL, &tv);
		if(res_select < 0){
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
				recv_m[pos]='\0';

				//hello message
				if(recv_m[0] == HELLO_MESSAGE){
					get_time_string(time_now);
					printf("%s: Received HELLO message ", time_now);
					printf("from (%s:%d): %s\n", inet_ntoa(c.sin_addr), ntohs(c.sin_port), recv_m+1);

					//if there is space
					if(clients_num < MAX_CLIENTS){
						//add client, increase index
						clients[clients_num] = c;
						clients_num++;
						printf("Added a new client\n", clients_num);
					}else{
						printf("Too many clients, can't add another one\n");
					}
				}
				//normal message
				else if(recv_m[0] == NORMAL_MESSAGE){
					bool contains = false;
					for(int index=0; index < clients_num; index++){
						if((clients[index].sin_addr.s_addr == c.sin_addr.s_addr) && (clients[index].sin_port == c.sin_port)){
							contains = true;
							break;
						}
					}
					if(contains){
						get_time_string(time_now);
						printf("%s: Received message", time_now);
						printf(" from registered client(%s:%d): %s\n", inet_ntoa(c.sin_addr), ntohs(c.sin_port), recv_m+1);
					}
				}
				//goodbye message
				else if(recv_m[0] == GOODBYE_MESSAGE){
					get_time_string(time_now);
					printf("%s: Received GOODBYE message", time_now);
					printf(" from registered client(%s:%d): %s\n", inet_ntoa(c.sin_addr), ntohs(c.sin_port), recv_m+1);

					//delete client
					for(int index=0; index < clients_num; index++){
					//if was registered
						if((clients[index].sin_addr.s_addr == c.sin_addr.s_addr) && (clients[index].sin_port == c.sin_port)){
							//if last, decrease num
							if(index = clients_num-1){
								clients_num--;
							}else{ //decrease num, put last in its place
								clients[index] = clients[clients_num-1];
								clients_num--;
							}
							printf("%s: Client (%s:%d) has quit\n", time_now, inet_ntoa(c.sin_addr), ntohs(c.sin_port));
							break;
						}
					}
				}
			}
			//if related to self-pipe
			else if(FD_ISSET(self_pipe[0], &readfds)) {
				read(self_pipe[0], ignore_pipe, MAX_BUF);

				//if 1+ clients are registered
				if(clients_num > 0){
					bool sent = false;
					int random_index;
					//try to send
					unsigned char random_string[MESSAGE_LENGTH];

					do{
						//generate random index
						random_index = rand() % clients_num;
						struct sockaddr_in random_client = clients[random_index];

						//try to send
						generate_random_string_with_timestamp(random_string, MESSAGE_LENGTH);

						snprintf(send_m, MAX_BUF, "%c%s", NORMAL_MESSAGE, random_string);

						pos = sendto(s, send_m, strlen(send_m), 0, (const struct sockaddr *)&random_client, c_len);
						get_time_string(time_now);
						//if cant send, delete that client
						if(pos < 0){
							fprintf(stderr, "ERROR: %s (%s,%d)\n", strerror(errno), __FILE__, __LINE__-4);

							for(int index=0; index < clients_num; index++){
							//was registered
								if((clients[index].sin_addr.s_addr == random_client.sin_addr.s_addr) && (clients[index].sin_port == random_client.sin_port)){
									//if last, decrease num
									if(index = clients_num-1){
										clients_num--;
									}else{ //delete and decrease num, put last in its place
										clients[index] = clients[clients_num-1];
										clients_num--;
									}
								}
								printf("%s: Client (%s:%d) has quit\n", time_now, inet_ntoa(random_client.sin_addr), ntohs(random_client.sin_port));
								break;
							}

						}else{
							get_time_string(time_now);
							printf("%s: Sent dgram: %s to registered client (%s:%d)\n", time_now, send_m+1, inet_ntoa(random_client.sin_addr), ntohs(random_client.sin_port));
							sent = true;
						}
					}while(!sent);
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
	printf("Server has worked for %.1f seconds\n", time_elapsed);
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

	strftime(where, MESSAGE_LENGTH, "%02H%02M%02S", timeinfo);

	if(len < strlen(where)){
		return 1;
	}

	for(i = strlen(where); len > i; i++){
		where[i] = lookUp[(rand() % range)];
	}
	where[len] = '\0';
	return 0;
}

void *random_sleep_function(void *ptr){
	int *thread_args = (int*)ptr;
	int min_t = thread_args[1];
	int max_t = thread_args[2];
	int random_time;

	for(;;){
		random_time = (rand() % (max_t+1-min_t) + min_t );
		printf("Sleeping for: %d ms\n", random_time);
		usleep(random_time*1000);

		char pipe_says[] = "Wake up, time to send dgram to random client!";
		write(thread_args[0], pipe_says, strlen(pipe_says)+1);
	}
}
