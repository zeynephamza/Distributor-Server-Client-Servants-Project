#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_BUFF_WRITE 300 // max size to print to screen
#define REPLY_LEN 300	//max size received from server
#define MAX_ROW_LEN 2048 //expected max row amount in requestList

char **requestList;
//char **requestList2;
int requestCounter;
int barrierCOUNT;
int barrageCount;
int jobDone=0;

int rowcount;
int columncount;

pthread_barrier_t threadBarrier;
pthread_barrier_t threadKillBarrier;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

sig_atomic_t exit_requested = 0;

int serverPORT;
char *serverIP;
int sockfd, connfd;
//struct sockaddr_in servaddr = {0};
struct sockaddr_in servaddr;

void *handleThreadJob(void *i);
char *read_line(int fd, int n);
int *lines_columns(int fd);
