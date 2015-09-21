#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "client.h"

#include "util.h"

void client_init_hints(struct addrinfo* hints){
    memset(hints, 0, sizeof hints); // make sure it is empty
    hints->ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints->ai_socktype = SOCK_STREAM; // TCP stream sockets
}

int check_bad_content(char *buf, int numbytes){
    char *content;
    strncpy(content,buf,numbytes);

    char *badword[] = BADWORDS; // Defined in client.h
    int size = (sizeof badword)/(sizeof badword[0]);
    int i;
    for(i = 0; i < size; i++){
        if(strstr(content, badword[i])){
            printf("****** Browser Trying to Access Bad URL/Content ******\n");
            return 1;
        }
    }
    return 0;
}

void extract_host_name(char* hostN, char *buf){
    char *startHN;
    char *endHN;
    char playbuf[MAXDATASIZE];

    if(startHN = strstr(buf, "Host: ")){

        startHN = strchr(startHN,':');
        endHN = strchr(startHN,'\n');
        memcpy(hostN,startHN+2,endHN-startHN-2);
        hostN[strlen(hostN)-1] = '\0';
    } else {
        printf("HOST NAME NOT FOUND ***********");
    }
}

void client_handle_request(int browser_fd){
    //entry point from server

    // initialize data buffer
    char buf[MAXDATASIZE];
    int numbytes; // Total number of recieved bytes

      // Get request from browser
    if((numbytes = recv(browser_fd, buf, MAXDATASIZE-1,0)) == -1)
    {
        perror("recv");
        exit(1);
    }
    // Append null character
    buf[numbytes] = '\0';

    printf("---------------------------server: recieved from browser \n'%s'\n",buf);
    if(check_bad_content(buf,numbytes)){
        char *MSG = "HTTP/1.1 302 Found\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error1.html\r\n\r\n"; //"Accessing Bad URL!";
        if (send(browser_fd, MSG, strlen(MSG), 0) == -1)
            perror("send");

    } else {

        char HOST[1000];
        extract_host_name(HOST, buf);

        printf("Client side started\n");

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

        //Forward HTTP to host(msg)
        printf("---------------------------client: send to host \n'%s'\n",buf);
        if (send(host_sock_fd, buf, numbytes, 0) == -1)
            perror("send");

        // Recive response from host
        if((numbytes = recv(host_sock_fd, buf, MAXDATASIZE-1, MSG_WAITALL)) == -1) { // TODO : TAR FÖR MYCKET TID med MSG_WAITALL
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';

        printf("%d",numbytes);
        printf("-------------------------------client: recieved from host \n'%s'\n",buf);
        close(host_sock_fd);

        if(check_bad_content(buf,numbytes)){
            char *MSG = "HTTP/1.1 302 Found\r\nLocation: http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error2.html\r\n\r\n"; //"Accessing Bad URL!";
            if (send(browser_fd, MSG, strlen(MSG), 0) == -1)
                perror("send");
        } else {
            // Forward response to browser
            printf("-------------------------------client: send to browser \n'%s'\n",buf);
            if (send(browser_fd, buf, numbytes, 0) == -1)
                perror("send");

        }
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