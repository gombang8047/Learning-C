#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "post -> 1 value input please\n");
        exit(0);
    }

    int listenfd, connfd;
    char *post = argv[1];
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    char hostname[MAXLINE];
    char client_port[MAXLINE];

    listenfd = Open_listenfd(post);

    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connection to (%s, %s)\n", hostname, client_port);
        echo(connfd);
        close(connfd);
        printf("Connection with (%s, %s) closed.\n", hostname, client_port);
    }
    exit(0);
}

