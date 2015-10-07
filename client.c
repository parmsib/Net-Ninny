#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#include <arpa/inet.h>

#include "client.h"

#include "util.h"
#include "http.h"

extern int errno; //error code from failed syscalls

void client_init_hints(struct addrinfo* hints){
    memset(hints, 0, sizeof hints); // make sure it is empty
    hints->ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints->ai_socktype = SOCK_STREAM; // TCP stream sockets
}

// non NULL terminated strstr
char *strstr_nnt(char *shoe, int shoesize, char *pebble){
    int pebblesize = strlen(pebble);
    int shoepos;
    for(shoepos = 0; shoepos <= shoesize; shoepos++){
        if(!memcmp(shoe + shoepos, pebble, pebblesize)){
            return shoe + shoepos;
        }
    }

    return NULL;

}

int check_bad_content(char *buf, int numbytes){
    char content[MAXDATASIZE];
    memcpy(content,buf,numbytes);
    content[MAXDATASIZE] = '\0';
    char *badword[] = BADWORDS; // Defined in client.h
    int size = (sizeof badword)/(sizeof badword[0]);
    int i;
    for(i = 0; i < size; i++){
        if(strcasestr(content, badword[i])){
            printf("****** Browser Trying to Access Bad URL/Content ******\n");
            return 1;
        }
    }
    return 0;
}

void extract_host_name(char* hostN, char *buf){
    char *startHN;
    char *endHN;

    if(startHN = strstr(buf, "Host: ")){
        startHN = strchr(startHN,':');
        endHN = strchr(startHN,'\n');
        memcpy(hostN,startHN+2,endHN-startHN-2);
        hostN[strlen(hostN)-1] = '\0';
    } else {
        printf("******* HOST NAME NOT FOUND ***********");
    }
}

int host_receive(int host_sock_fd, int browser_fd, char* buf, int* buffered, int* buf_len){
    memset(buf, 0, MAXDATASIZE); //not optimal, but makes the code below simpler
    int rsp_is_text = 0;
    int rsp_buf_end = 0; //the last index of buf where data has been rcvd
    int rsp_header_rcvd = 0; //whether the rsps header has been received fully
    int rsp_content_length = 0;
    int rsp_no_encoding = 0;
    int rsp_monitor = 0;
    printf("------------------UNMONITORED receiving from host?:---------------------\n");
    while(1){
        // Recieve from host
        int res = recv(host_sock_fd, buf + rsp_buf_end, MAXDATASIZE-1,0);
        if(res < 0 && errno != EAGAIN){ //EAGAIN just means there was nothing to read
            perror("recv from browser failed.");
            return -1;
        }
        if(res == 0){
            //tcp connection closed
            //nothing more to receive
            break;
        }
        rsp_buf_end += res;
        if(rsp_header_rcvd == 0 && (http_whole_header(buf))){
            //this is the first time we have the whole header received
            rsp_header_rcvd = 1;
            //find out if we can monitor this response
            rsp_content_length = http_content_length(buf);
            rsp_is_text = http_is_text(buf);
            rsp_no_encoding = http_no_encoding(buf);
            rsp_monitor = rsp_content_length && rsp_is_text
                            && rsp_no_encoding;
            *buffered = rsp_monitor;
        }
        if(!rsp_monitor){
            char* c;
            for(c = buf + rsp_buf_end - res; c < buf + rsp_buf_end; c++)
                printf("%c", *c);
            printf("|%s", buf + rsp_buf_end);
            printf("rsp does NOT need to be monitored\n");
            //since we do not have to monitor this rsp,
            // just send it asap
            int res = send(browser_fd, buf, rsp_buf_end, 0);
            if (res < 0){
                perror("sending non-monitored response part to browser failed");
                return -1;
            }
            rsp_buf_end = 0;
        }
    }
    *buf_len = rsp_buf_end;
    if(!rsp_monitor) printf("\n");
    printf("left the receive loop \n");
    return 0;
}
int replace_first(char *content,int cont_nb, char *from, char *to){
    char temp[MAXDATASIZE];
    char *p;
    content[MAXDATASIZE]='\0';
    // Is the word even in the string
    if(!(p = strcasestr(content,from))){
        return 0;
    }
    // Copy the part before the first occurence of the word
    strncpy(temp,content,p-content);
    temp[p-content] = '\0';

    // Append the new word and the string behind the new word on temp
    sprintf(temp + (p-content), "%s%s", to, p+strlen(from));

    strcpy(content,temp);
    return 1;
}

void change_connection_type(char *head, int *numbytes){
    char *from = "Connection: keep-alive";
    char *to = "Connection: close";

    if(!replace_first(head,*numbytes,from,to)){
        printf("********* No connection type found *****");
    }
    return;
}

void client_handle_request(int browser_fd){
    //entry point from server

    // initialize data buffer
    char buf[MAXDATASIZE];
    memset(buf,0,MAXDATASIZE);

    fcntl(browser_fd, F_SETFL, O_NONBLOCK);

    int GET_size = 0;
    while(1){

/*
******************** RECIEVE HEADER FROM BROWSER ******************************
*/

        //recv whole header
        int res = recv(browser_fd, buf + GET_size, MAXDATASIZE-1,0);
        if(res < 0 && errno != EAGAIN){ //EAGAIN just means there was nothing to read
            perror("recv from browser failed.");
            exit(1);
        }
        if(res <= 0)
            continue;

        GET_size += res;

        if (http_whole_header(buf)){
            //buf contains the whole header (request)
            break;
        }
    }

    printf("---------------------------server: received from browser \n'%s'\n",buf);
    buf[GET_size]='\0'; // For string handling
    if(check_bad_content(buf,GET_size)){
        char *MSG = "HTTP/1.1 302 Found\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error1.html\r\n\r\n"; //"Accessing Bad URL!";
        if (send(browser_fd, MSG, strlen(MSG), 0) == -1)
            perror("send");
        return;
    }

/*
*********************** SET UP CONNECTION WITH HOST ********************
*/
    char HOST[1000];
    extract_host_name(HOST, buf);

    if (strstr(HOST,":"))
    {
        printf("Only standard \'HTTP port 80\' traffic supported by proxy\n");
        printf("Tried to access \'%s\'\n", HOST);
        return;
    }

    // Dont allow keep-alive
    change_connection_type(buf,&GET_size);

    int hostfd;
    struct addrinfo hints;
    client_init_hints(&hints);

        // Get address information from host
    struct addrinfo* hostinfo;
    int rv;
    if((rv = getaddrinfo(HOST,HOSTPORT,&hints,&hostinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));
        exit(1);
    }

        //Create socket to port and connct to host
    int host_sock_fd;
    host_sock_fd = client_connect_host(hostinfo);
    freeaddrinfo(hostinfo);

/*
**************************** Forward HTTP to host(msg) *******************
*/
    printf("---------------------------client: send to host \n'%s'\n",buf);
    if (send(host_sock_fd, buf, GET_size, 0) == -1)
        perror("send");

/*
******************** Receive response from host ****************
*/

    int buffered = 0; //set to true if rsp should be monitored
                      //if not set, rsp has already been sent
    int buf_len = 0;
    if(host_receive(host_sock_fd, browser_fd, buf, &buffered, &buf_len)){
        fprintf(stderr, "Error while receiving from host \n");
        exit(1);
//        return -1;
    }

/*
******************** MONITOR RECIEVED CONTENT *****************
*/

    if(buffered){
        //TODO: monitor
        printf("-------------------------------client: MONITORED recieved from host \n'%s'\n",buf);
        close(host_sock_fd);

        if(check_bad_content(buf,buf_len)){
            char *MSG = "HTTP/1.1 302 Found\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error2.html\r\n\r\n"; //"Accessing Bad URL!";
            if (send(browser_fd, MSG, strlen(MSG), 0) == -1)
                perror("send");
            return;
        }
/*
*************** Forward response to browser *********************
*/
        printf("-------------------------------client: send to browser \n'%s'\n",buf);
        if (send(browser_fd, buf, buf_len, 0) == -1)
            perror("send");

    }
    return;
}

int client_connect_host(struct addrinfo* hostinfo){
    struct addrinfo* p;
    int hostfd;
    char s[INET6_ADDRSTRLEN];
    for(p = hostinfo; p != NULL; p = p->ai_next){
        if ((hostfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }

        if (connect(hostfd, p->ai_addr, p->ai_addrlen) == -1){
            close(hostfd);
            perror("client: connect");
            continue;
        }

        break;
    }
    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    return hostfd;
}
