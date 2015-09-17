#ifndef _SERVER_H_
#define _SERVER_H_

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>


void *get_in_addr(struct sockaddr *sa);

#endif 
