#include "csapp.h"

void echo(int connfd){
    rio_t rio;
    char buf[MAXLINE];
    size_t n;

    rio_readinitb(&rio, connfd);

    while((n = rio_readlineb(&rio, buf, MAXLINE)) != 0){
        rio_writen(connfd, buf, strlen(buf));
        printf("server recieved %d bytes\n", (int)n);
    }
}

