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
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_MODES O_WRONLY | O_APPEND | O_CREAT
#define USER_PERMS S_IRUSR | S_IWUSR
#define MAX_BUFF_READ 300
#define MAX_BUFF_FD 5000
#define DATE_LEN 500


typedef struct 
{

	char startCity[MAX_BUFF_READ];
	char endCity[MAX_BUFF_READ];
	int servantPORT;
	char servantIP[DATE_LEN];
	int counter;

	//struct ServantInfo *next;

}ServantInfo;

typedef struct 
{

	char fdArray[MAX_BUFF_FD][MAX_BUFF_FD];
	//int fdArray[MAX_BUFF_FD];
	int front;
	int rear;
	int size;

	//struct ServantInfo *next;

}FileDescriptors;


typedef struct 
{
	char realEstate[MAX_BUFF_READ];
	int startDate;  // converted to day
	int endDate;
	int startSurfaceCount;
	int endSurfaceCount;
	char cityName[MAX_BUFF_READ];

}RequestList;

FileDescriptors fileDesc1;
FileDescriptors fileDesc2;

ServantInfo servantInfo[MAX_BUFF_FD];

int servantCounter;
int detectedServantCount=0;
const char *logfilename = "server_logfile.log";
int totalcitycount=0;
int failedCount = 0;

sig_atomic_t exit_requested = 0;

//pthread_mutex_t thread_start_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_barrier_t threadBarrier;
pthread_barrier_t clientThreadBarrier;

pthread_cond_t cond_job1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_job1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lock1 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_job2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_job2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lock2 = PTHREAD_MUTEX_INITIALIZER;


void *handleThreadJob(void *fd);
void initFileDescs1(int f, int r, int s);
void initFileDescs2(int f, int r, int s);
void insertFd1(char data[MAX_BUFF_FD]);
void insertFd2(char data[MAX_BUFF_FD]);
void removeFd1();
void removeFd2();
int convertDateToDay(char dateString[DATE_LEN]);
int sort_words(char *word1,char *word2, char *requestedcity);
