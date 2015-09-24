#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "http.h"


unsigned int http_whole_header(char* buf){
    char* endstring = "\r\n\r\n";
    char* emptyline = strstr(buf, endstring);
    if(emptyline == NULL)
        return 0;
    return (unsigned int) ( (uintptr_t)emptyline - (uintptr_t)buf
            + strlen(endstring));
}

int http_is_text(char* buf){
    unsigned int header_len = http_whole_header(buf);
    if(header_len < 0){
        printf("ERROR: Whole header not received in http_is_text\n");
        return -1;
    }

    char* content_type = strcasestr(buf, "content-type"); 
    if(content_type == NULL){
        //no content-type found, so we default to not-text
        return 0;
    }
    char* text_p = strcasestr(content_type, "text");
    char* next_newline = strcasestr(content_type, "\n");
    if(text_p == NULL)
        return 0;
    if(next_newline < text_p)
        //newline found before "text", meaning the value was found somewhere else in message
        return 0;
    return 1;
}

int http_content_length(char* buf){
    unsigned int header_len = http_whole_header(buf);
    if(header_len == 0){
        printf("ERROR: While header not received in http_msg_size\n");
        return -1;
    }

    char* cmp_str = "content-length";
    char* content_length = strcasestr(buf, cmp_str);
    if(content_length == NULL){
//        printf("Error: no content length found, after whole header check\n");
//        return -1;
        //no length found, meaning there is no message body
        printf("http_body_size returning: 0\n");
        return 0;
    }
    content_length = strcasestr(content_length, ":") 
                     + 2;
    int content_length_val = 0;
    if(sscanf(content_length, "%d", &content_length_val) != 1){
        fprintf(stderr, "Failed to parse content-length value\n");
    }
//    printf("http_body_size returning: %d\n", content_length_val);
    return content_length_val;
}

int http_no_encoding(char* buf){
    unsigned int header_len = http_whole_header(buf);
    if(header_len == 0){
        printf("ERROR: Header not received in http_msg_size\n");
        return -1;
    }
    char* cmp_str = "content-encoding";
    char* content_encoding = strcasestr(buf, cmp_str);
    if(content_encoding == NULL){
        return 1;
    }
    return 0;
}





