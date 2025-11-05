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
int find_in_cache(char *uri);
void write_to_cache(char *data, int totalsize, char *uri);

//캐시하기 위한 구조체
struct cache_box{
  char uri[MAXLINE];
  char *data;
  int size;
  int timestamp;
  int valid;
};
struct cache_box cache[10];
static int global_timestamp = 0;

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
  for (int i = 0; i < 10; i++) {
    cache[i].valid = 0;
    cache[i].size = 0;
    cache[i].timestamp = 0;
    cache[i].uri[0] = '\0';
    cache[i].data = NULL; // Malloc 사용 전 NULL로 초기화
  }
  global_timestamp = 0;

  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    pthread_t tid;
    int *fdp = Malloc(sizeof(int));
    *fdp = connfd;
    Pthread_create(&tid, NULL, doit, fdp);

  }
}

void *doit(void *vargp)
{
  rio_t rio_client, rio_server;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], raw_uri[MAXLINE];
  char host[MAXLINE], port[MAXLINE], path[MAXLINE];
  char request_buf[MAXBUF];
  char *p = request_buf;
  int remaining = MAXBUF;
  int n, serverfd, index;

  int fd = *(int *)vargp;
  Free(vargp);

  Pthread_detach(Pthread_self());

  Rio_readinitb(&rio_client, fd);
  if(!Rio_readlineb(&rio_client, buf, MAXLINE)) return NULL;
  printf("Request line from browser:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  strcpy(raw_uri, uri);

  if(strcasecmp(method, "GET")){
    clienterror(fd, method, "501", "NOT implemented", "Tiny does not implement this method");
    return NULL;
  }

  read_requesthdrs(&rio_client);

  parse_uri(uri, host, port, path);

  pthread_rwlock_rdlock(&cache_lock);
  index = find_in_cache(raw_uri);
  pthread_rwlock_unlock(&cache_lock);

  if(index >= 0){
    //cache Hit
    pthread_rwlock_wrlock(&cache_lock);
    cache[index].timestamp = global_timestamp++;
    Rio_writen(fd, cache[index].data, cache[index].size);
    pthread_rwlock_unlock(&cache_lock);
    Close(fd);
    return NULL;
  }
  //cache Miss
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

  char temp_cache_buf[MAX_OBJECT_SIZE];
  int totalsize = 0;
  int can_cache_flag = 0;
  int contentlength = -1;
  char line_buf[MAXLINE];
  char *line_p;

  if (!Rio_readlineb(&rio_server, line_buf, MAXLINE)) return NULL;
  Rio_writen(fd, line_buf, strlen(line_buf));

  if (totalsize + strlen(line_buf) <= MAX_OBJECT_SIZE) {
    memcpy(temp_cache_buf + totalsize, line_buf, strlen(line_buf));
    totalsize += strlen(line_buf);
  }

  do{
    if (!Rio_readlineb(&rio_server, line_buf, MAXLINE)) break;

    if (!strncasecmp(line_buf, "Content-Length:", 15)) {
      char *v = line_buf + 15;
      while (*v == ' ' || *v == '\t') v++;
      contentlength = atoi(v); 
    }
    Rio_writen(fd, line_buf, strlen(line_buf));

    if (totalsize + strlen(line_buf) <= MAX_OBJECT_SIZE) {
        memcpy(temp_cache_buf + totalsize, line_buf, strlen(line_buf));
        totalsize += strlen(line_buf);
    }
  }while(strcmp(line_buf, "\r\n"));

  //캐싱 가능한지 플래그 설정
  if(contentlength > 0 && contentlength <= MAX_OBJECT_SIZE){
    can_cache_flag = 1;
  }

  while((n = Rio_readn(serverfd, buf, MAXLINE)) > 0){
    Rio_writen(fd, buf, n);
    
    if(can_cache_flag == 1){
      if(totalsize + n <= MAX_OBJECT_SIZE){
        memcpy(temp_cache_buf + totalsize, buf, n);
        totalsize += n;
      }else {
            // MAX_OBJECT_SIZE 초과 시 플래그 비활성화 (선택적)
            can_cache_flag = 0;
      }
    }
  }

  if(can_cache_flag == 1){
    pthread_rwlock_wrlock(&cache_lock);
    write_to_cache(temp_cache_buf, totalsize, raw_uri);
    pthread_rwlock_unlock(&cache_lock);
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
    if(cache[i].valid == 1 && strcmp(cache[i].uri, uri) == 0){
      return i;
    }
  }
  return -1;
}

void write_to_cache(char *data, int totalsize, char *uri){

  int temp_time_stamp = INT_MAX;
  int lru_index = -1;
  int empty_slot = -1;

  for(int i=0; i<10; i++){

    if(cache[i].valid == 0){
      empty_slot = i;
      break;
    }
    
    if(cache[i].timestamp < temp_time_stamp){
      temp_time_stamp = cache[i].timestamp;
      lru_index = i;
    }

  }

  int index = (empty_slot != -1) ? empty_slot : lru_index;

  if (cache[index].valid && cache[index].data) {
    Free(cache[index].data); 
  }

  cache[index].data = Malloc(totalsize);
  memcpy(cache[index].data, data, totalsize);
  cache[index].size = totalsize;
  cache[index].timestamp = global_timestamp++;
  strncpy(cache[index].uri, uri, MAXLINE-1);
  cache[index].uri[MAXLINE-1] = '\0';
  cache[index].valid = 1;

}