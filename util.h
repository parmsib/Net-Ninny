#ifndef _UTIL_H_
#define _UTIL_H_

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAXDATASIZE 2000000

void *get_in_addr(struct sockaddr *sa);

#endif
