#include <stdio.h>
#include <stdlib.h>

#include "server.h"


int main(void){

    // init hints for getaddrinfo
    struct addrinfo hints;
    server_init_hints(&hints);

    //use hints to receive servinfo
    struct addrinfo* servinfo;
    int rv;
    if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // create socket and bind to port and address
    int server_sock_fd;
    server_sock_fd = server_bind_socket(servinfo);
    freeaddrinfo(servinfo); // all done with this structure

    // listen with socket
    if (listen(server_sock_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // something something signals
    server_handle_signals();

    printf("server: waiting for connections...\n");

    //accept loop
    server_accept_loop(server_sock_fd);

    return 0;
}
