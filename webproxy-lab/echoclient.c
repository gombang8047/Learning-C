#include "csapp.h"

int main(int argc, char **argv){

    if(argc != 3){
        fprintf(stderr, "host, post -> 2 value input please\n");
        exit(0);
    }

    int clientfd;
    char *host = argv[1];
    char *post = argv[2];
    char buf[MAXLINE];
    rio_t rio;

    clientfd = Open_clientfd(host, post);
    rio_readinitb(&rio, clientfd);

    while(Fgets(buf, MAXLINE, stdin) != NULL){
        rio_writen(clientfd, buf, strlen(buf));
        rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }

    close(clientfd);
    exit(0);
}