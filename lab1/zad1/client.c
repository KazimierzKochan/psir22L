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
	time_t rawtime;
	struct tm * timeinfo;
	int s; //identyfikator gniazda
	unsigned char stringtime[MAX_LINE];
	struct addrinfo h;
	struct addrinfo *r;
	memset(&h, 0, sizeof(struct addrinfo));//lepiej wyczyscic

	getaddrinfo("192.168.56.110", "3792", &h, &r);

	s=socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if(s==-1){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}

	//korzystanie z gniazda 's'
	if(connect(s, r->ai_addr, r->ai_addrlen)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	for(;;){
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		snprintf(stringtime, MAX_LINE, asctime(timeinfo));
		printf("Co chcemy wyslac: %s", stringtime);

		if(send(s, stringtime, strlen(stringtime), 0) < 0){
			printf("error nie wyslal sie\n");
			exit(-5);
		}
		//sleep for 1030 ms
		usleep(1030000);

	}

	freeaddrinfo(r);
	close(s);

	return 0;
}
