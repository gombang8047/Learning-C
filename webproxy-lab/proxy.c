#include <stdio.h>
#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void *doit(void *vargp);
void read_requesthdrs(rio_t *rp);
void parse_uri(char *uri, char *host, char *port, char *path);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

//캐시하기 위한 구조체
struct cache_box{
  char uri[MAXLINE];
  char *data;
  int size;
  int timestamp;
};
struct cache_box cache[10];

//쓰기를 할 때 읽기를 락하기 위한 장치
pthread_rwlock_t cache_lock;

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
 
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  Signal(SIGPIPE, SIG_IGN);

  //초기화
  pthread_rwlock_init(&cache_lock, NULL);

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    pthread_t tid;
    Pthread_create(&tid, NULL, doit, (void *)connfd);

  }
}

void *doit(void *vargp)
{

  rio_t rio_client, rio_server;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char host[MAXLINE], port[MAXLINE], path[MAXLINE];
  char request_buf[MAXBUF];
  char *p = request_buf;
  int remaining = MAXBUF;
  int n, serverfd, index;

  int fd = (int)vargp;

  Pthread_detach(Pthread_self());

  Rio_readinitb(&rio_client, fd);
  if(!Rio_readlineb(&rio_client, buf, MAXLINE)) return;
  printf("Request line from browser:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  if(strcasecmp(method, "GET")){
    clienterror(fd, method, "501", "NOT implemented", "Tiny does not implement this method");
    return NULL;
  }

  read_requesthdrs(&rio_client);

  parse_uri(uri, host, port, path);

  pthread_rwlock_rdlock(&cache_lock);

  if((index = find_in_cache(uri)) >= 0){
    //cache Hit
    Rio_writen(fd, cache[index].data, cache[index].size);
    Close(fd);
    return NULL;
  }
  //cache Miss
  pthread_rwlock_unlock(&cache_lock);

  n = snprintf(p, remaining, "GET %s HTTP/1.0\r\n", path);
  p += n; remaining -= n;
  n = snprintf(p, remaining, "Host: %s\r\n", host);
  p += n; remaining -= n;
  n = snprintf(p, remaining, "%s", user_agent_hdr);
  p += n; remaining -= n;
  n = snprintf(p, remaining, "Connection: close\r\n");
  p += n; remaining -= n;
  n = snprintf(p, remaining, "Proxy-Connection: close\r\n\r\n");
  p += n; remaining -= n;

  serverfd = open_clientfd(host, port);
  if(serverfd < 0){
    clienterror(fd, host, "502", "Bad Gateway", "Tiny proxy couldn't connect to the server");
    return NULL;
  }

  Rio_writen(serverfd, request_buf, strlen(request_buf));
  Rio_readinitb(&rio_server, serverfd);

  while((n = Rio_readn(serverfd, buf, MAXLINE)) > 0){
    Rio_writen(fd, buf, n);
  }

  Close(serverfd);

  Close(fd);

  return NULL;
}

void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while (strcmp(buf, "\r\n"))
  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

void parse_uri(char *uri, char *host, char *port, char *path){

  char *ptr_start = strstr(uri, "//");
  ptr_start += 2;

  char *ptr_path = strchr(ptr_start, '/');
  if(ptr_path){
    strcpy(path, ptr_path);
    *ptr_path = '\0';
  }
  else strcpy(path, "/");

  char *ptr_port = strchr(ptr_start, ':');
  if(ptr_port){
    strcpy(port, ptr_port+1);
    *ptr_port = '\0';
  }
  else strcpy(port, "80");

  strcpy(host, ptr_start);

}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

int find_in_cache(char *uri){

  for(int i=0; i<10; i++){
    if(strcmp(cache[i].uri, uri) == 0){

      cache[i].timestamp = user_agent_hdr;

      return i;
    }
  }

  return -1;
}