#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_NAME_BUFFER 10
#define DATA_LEN_BUFFER 1000
#define WRITE_TO_LOG_BUFFER 500

#define FIFO_MODES O_RDWR
#define FILE_MODES O_WRONLY | O_APPEND | O_CREAT
#define USER_PERMS S_IRUSR | S_IWUSR



const char *logfilename = "distributor_logfile.log";