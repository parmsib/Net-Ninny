/*
 ** server.c -- a stream socket server demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "server.h"

#include "client.h"
//#include "util.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
  if (sa->sa_family == AF_INET){
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int shand)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


void server_init_hints(struct addrinfo* hints){
    memset(hints, 0, sizeof hints);
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE; // use my IP
}

void server_handle_signals(void){
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void server_accept_loop(int sockfd){

    socklen_t sin_size;
    struct sockaddr_storage their_addr;
    int browser_fd;
    char s[INET6_ADDRSTRLEN];

    while(1) {  // main accept() loop

        sin_size = sizeof their_addr;
        browser_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (browser_fd == -1) {
            perror("accept");
            continue;
        }

        void *temp = get_in_addr((struct sockaddr *)&their_addr);

        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s);

        printf("server: got connection from %s\n", s);


        // fork to new thread to handle our new client
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            // handle the request
            client_handle_request(browser_fd);
            // close the connection to the client
            close(browser_fd);
            // kill the thread, since the client has been dealt with
            exit(0);
        }
        // close the client fd, since the parent has no use for it
        close(browser_fd);
    }
}

int server_bind_socket(struct addrinfo* servinfo){
    int yes = 1;
    struct addrinfo* p;
    int sockfd;
    // loop through all the results of servinfo and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                        p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    return sockfd;
}
