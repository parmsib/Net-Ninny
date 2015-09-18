#ifndef _CLIENT_H_
#define _CLIENT_H_

#define PORT "80" // Port to connect to
#define MAXDATASIZE 10000


void client_handle_request(int client_fd);

#endif
