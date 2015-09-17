/*
** server.h 
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

#define SERVERPORT "8080"  // port proxyserver
#define CLIENTPORT "80" // Destination port webserver

#define BACKLOG 10     // how many pending connections queue will hold

#define MAXDATASIZE 100