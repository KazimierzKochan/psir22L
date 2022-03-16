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

int main(){
	int s, new_s;
	struct addrinfo h, *r;
	unsigned char mip_str[INET_ADDRSTRLEN];
	char message[MAX_BUF];
	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family=PF_INET;
	h.ai_socktype=SOCK_STREAM;
	h.ai_flags=AI_PASSIVE;
	getaddrinfo(NULL, "3792", &h, &r );

	printf("Witamy w programiku na lab1 z psiru\n");

	s=socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if(s==-1){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}

	if(bind(s, r->ai_addr, r->ai_addrlen)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}

	if(listen(s, 1)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-6);
	}
	struct sockaddr_in their_addr;
	socklen_t addr_size=sizeof(their_addr);
	if((new_s=accept(s, (struct sockaddr *)&their_addr, &addr_size))==-1)
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
	if(inet_ntop(AF_INET, &(their_addr.sin_addr), mip_str, INET_ADDRSTRLEN)!=NULL)
		printf("IP: %s, new sock desc.: %d\n", mip_str, new_s);

	for(;;){
		int result=recv(new_s, message, MAX_BUF, 0);
		if(result==0){
			printf("Peer was disconeted\n");
			break;
			exit(-1);
		}else if(result<0){
			printf("ERROR: %s\n", strerror(errno));
			exit(-4);
		}
		message[result]='\0';
		printf("Client said: %s\n", message);

		//obsluga innych sytuacji

	}













	close(s);
	freeaddrinfo(r);


	return 0;
}
