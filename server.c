

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sockets.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "server.h"

#define PORT "3490"

#define BACKLOG 10

int main(void){
    int sockfd, new_fd;  
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if( rv = getaddrinfo(NULL, PORT, &hints, &servinfo) != 0 ){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));    
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        
    }



    



}









