#ifndef _CLIENT_H_
#define _CLIENT_H_

#define HOSTPORT "80" // Port to connect to
#define MAXDATASIZE 2000000
#define BADWORDS {"SpongeBob","Britney Spears", "Paris Hilton", };//"Norrköping"}


void client_handle_request(int client_fd);

#endif