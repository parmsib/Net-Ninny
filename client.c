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

void client_init_hints(struct addrinfo* hints){
  memset(hints, 0, sizeof hints); // make sure it is empty
  hints->ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints->ai_socktype = SOCK_STREAM; // TCP stream sockets
}

void client_handle_request(int browser_fd){
    //entry point from server

    //some dummy code
  char buf[MAXDATASIZE];
  int numbytes;

  if((numbytes = recv(browser_fd, buf, MAXDATASIZE-1,0)) == -1)
  {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';

  printf("---------------------------server: recieved from browser \n'%s'\n",buf);

  char *HOST;
  char *point;
  char playbuf[MAXDATASIZE];

  memcpy(playbuf,buf,numbytes);

  if(point = strstr(playbuf, "Host:")){
    printf("FOUND*********\n");
    char *token = strtok(point," ");
    token = strtok(NULL,"\n");
    token[strlen(token)-1] ='\0';
    HOST = token;
  } else {
    printf("HOST NAME NOT FOUND ***********");
  }
  // if (send(browser_fd, "Hello, world!", 13, 0) == -1)
  //   perror("send");

    //end of dummy code

    //TODO: make "main" below into this function
  printf("Client side started\n");

  int hostfd;
  struct addrinfo hints;
  client_init_hints(&hints);

  struct addrinfo* hostinfo;
  int rv;
  if((rv = getaddrinfo(HOST,HOSTPORT,&hints,&hostinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));
    return 1;
  }

      //Create socket to port and connct to host
  int host_sock_fd;
  host_sock_fd = client_connect_host(hostinfo);
  freeaddrinfo(hostinfo);

  //Forward HTTP to host(msg)
   printf("---------------------------client: send to host \n'%s'\n",buf);
  if (send(host_sock_fd, buf, sizeof buf, 0) == -1)
    perror("send");

  // Recive response from host
  if((numbytes = recv(host_sock_fd, buf, MAXDATASIZE-1,0)) == -1) {
    perror("recv");
    exit(1);
  }

  printf("%d",numbytes);
  buf[numbytes] = '\0';

  printf("-------------------------------client: recieved from host \n'%s'\n",buf);
  close(host_sock_fd);


  //Forward response to browser
  printf("-------------------------------client: send to browser \n'%s'\n",buf);
  if (send(browser_fd, buf, sizeof buf, 0) == -1)
    perror("send");

  return 0;

}

int client_connect_host(struct addrinfo* hostinfo){
  struct addrinfo* p;
  int hostfd;
  char s[INET6_ADDRSTRLEN];
  for(p = hostinfo; p != NULL; p = p->ai_next){
    if ((hostfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("client: socket");
      continue;
    }

    if (connect(hostfd, p->ai_addr, p->ai_addrlen) == -1){
      close(hostfd);
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

  return hostfd;
}