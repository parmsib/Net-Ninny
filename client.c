#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "client.h"

#include "util.h"

void client_handle_request(int client_fd){
    //entry point from server

    //some dummy code 
    char buf[MAXDATASIZE];
    int numbytes;

    if((numbytes = recv(client_fd, buf, MAXDATASIZE-1,0)) == -1) 
    {

        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("server: recieved '%s'\n",buf);
    

    if (send(client_fd, "Hello, world!", 13, 0) == -1)
        perror("send");

    //end of dummy code

    //TODO: make "main" below into this function
}


int main2 (int argc, char *argv[])
{
	printf("Client started\n");

  int servsfd, hostfd, numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int status; // Return value
  char s[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints); // make sure it is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

  if((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }


  for(p = servinfo; p != NULL; p = p->ai_next){
    if ((servsfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("client: socket");
      continue;
    }

    if (connect(servsfd, p->ai_addr, p->ai_addrlen) == -1){
      close(servsfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if(p == NULL){
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  printf("client: connecting to %s\n", s);
  
  char HTTP[] = "I'm a client!";
  if (send(servsfd, HTTP, sizeof HTTP, 0) == -1)
    perror("send");

  freeaddrinfo(servinfo); // all done with this structure

  if((numbytes = recv(servsfd, buf, MAXDATASIZE-1,0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';

  printf("client: recieved '%s'\n",buf);
  close(servsfd);
  return 0;
}
