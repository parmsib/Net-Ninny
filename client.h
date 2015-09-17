#ifndef _CLIENT_H_
#define _CLIENT_H_

#define PORT "3490" // Port to connect to
#define MAXDATASIZE 100


void client_handle_request(int client_fd);

#endif
