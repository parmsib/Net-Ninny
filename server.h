/*
** server.h
*/

#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



#define BACKLOG 10     // how many pending connections queue will hold

char *SERVERPORT;
void server_init_hints(struct addrinfo* hints);
void server_handle_signals(void);
void server_accept_loop(int sockfd);
int server_bind_socket(struct addrinfo* servinfo);

#endif
