
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "3490" // Port to connect to
#define MAXDATASIZE 100


int main (int argc, char *argv[]){
	printf("Client started\n");

  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;
	
  memset(&hints, 0, sizeof hints); // make sure it is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	if((status = getaddrinfo(NULL,
                  PORT,
                  *hints,
                  *servinfo) != 0){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
  }

  servsfd = socket(servinfo->ai_family, 
                  servinfo->ai_socktype, 
                  servinfo->ai_protocol);

  connect(servsfd, servinfo->ai_addr, servinfo->ai_addrlen);
  


}