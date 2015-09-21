#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "http.h"


unsigned int http_whole_header(char* buf){
    char* emptyline = strstr(buf, "\n\n");
    if(emptyline == NULL)
        return 0;
    return (unsigned int) ( (uintptr_t)emptyline - (uintptr_t)buf + 2 );
}

int http_is_html(char* buf){
    unsigned int header_len = http_whole_header(buf);
    if(header_len < 0){
        printf("ERROR: Whole header not received in http_is_text\n");
        return -1;
    }

    char* content_type = strcasestr(buf, "content-type"); 
    if(content_type == NULL){
        printf("ERROR: No content-type found in http header");
    }
    char* text_html_p = strcasestr(content_type, "text/html");
    char* next_newline = strcasestr(content_type, "\n");
    if(text_html_p == NULL)
        return 0;
    if(next_newline < text_html_p)
        //newline found before "text/html", meaning the value was found somewhere else in message
        return 0;
    return 1;
}

int http_msg_size(char* buf){
    unsigned int header_len = http_whole_header(buf);
    if(header_len < 0){
        printf("ERROR: While header not received in http_msg_size\n");
        return -1;
    }

    char* content_length = strcasestr(buf, "content-length");
    if(content_length == NULL){
        printf("Error: no content length found, after whole header check\n");
        return -1;
    }
    content_length = strcasestr(content_length, ":") + 2;
    int content_length_val;
    if(sscanf(content_length, "%d", &content_length_val)){
        perror("Failed to parse content-length value");
    }
    return content_length_val + (int)header_len;
}





