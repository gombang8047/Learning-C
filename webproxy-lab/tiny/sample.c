#define LISTENQ  1024 
#define MAXLINE 8192
#define MAXBUF 8192
#define stderr stderr
#define S_ISREG(mode) __S_ISTYPE((mode), __S_IFREG)
#define S_IRUSR __S_IREAD
#define S_IXUSR __S_IEXEC
#define RIO_BUFSIZE 8192
struct socklen_t;
struct scokaddr_storage;
typedef struct sockaddr SA;
struct stat;
typedef struct {
    int rio_fd;               
    int rio_cnt;              
    char *rio_bufptr;         
    char rio_buf[RIO_BUFSIZE];
}rio_t;
struct pid_t;
extern char **environ;
Accept();
Bind();
clienterror();
Close();
Connect();
doit();
Dup2();
Execve();
exit();
Fstat();
Fork();
Freeaddrinfo();
fprintf();
gai_error();
Getnameinfo();
Getaddrinfo();
get_filetype();
index();
Inet_ntop();
Inet_pton();
Listen();
Malloc();
Mmap();
Munmap();
Open_listenfd();
Open_clientfd();
Open();
parse_uri();
perror();
printf();
read_requesthdrs();
Rio_readinitb();
Rio_readlineb();
Rio_writen();
sscanf();
strcasecmp();
stat();
Socket();
Setsockopt();
serve_static();
serve_dynamic();
strcmp();
strstr();
strcpy();
strcat();
strlen();
snprintf();
sprintf();
setenv();
waitpid();