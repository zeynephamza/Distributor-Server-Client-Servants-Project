#include "client.h"


void sig_handler(int sig_no)
{

    if (sig_no == SIGINT){         
        perror("SIGINT Caught!\n");
        exit_requested = 1;
        
    }
    
}

int main(int argc, char **argv)
{
	struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sig_handler;
    sigaction(SIGINT, &sa, NULL);

	int opt;
	char *requestFile;
	

	if (argc<6){
        perror("./client -r requestFile -q PORT2 -s IP -b BARRIER\n");
        
        return 1;       
    }
    while((opt = getopt(argc, argv, ":r:q:s:b:")) != -1) 
    {
        switch(opt) 
        { 
            
            case 'r': 
                requestFile =  optarg;
                //printf("requestFile: %s\n", requestFile); 
                break; 
            case 'q': 
                serverPORT = atoi(optarg);
                //printf("serverPORT: %d\n", serverPORT); 
                break; 
            case 's':  
                serverIP = optarg;
                //printf("serverIP: %s\n", serverIP); 
                break; 
            case 'b':
                barrierCOUNT = atoi(optarg);
                break;
            case '?': 
                perror("Unknown option. ./client -r requestFile -q PORT -s IP\n");
                return 0;  
            default:
                perror("Usage ./client -r requestFile -q PORT -s IP\n");
                return 0; 
        }

    }
    
    ////////////////Reading Request File///////////////////////////////////////////////////////////
    

    int fd;//, bytesread;  

    if((fd = openat(AT_FDCWD, requestFile, O_RDONLY)) == -1){
        perror("Failed to open file\n");
        return 1;
    }
    //printf("opened fd:%d\n",fd);


    int *rows_columns=lines_columns(fd);  // Finding rowcount=Request amount and longest request length for malloc

    rowcount=rows_columns[0];
    columncount=rows_columns[1];
    

    //printf("There are %d Threads and %d Requests, There will be %d questionbarrage and %d additional requests \n",barrierCOUNT,rowcount,requestBarrageCount,residualRequestCount);

    //sleep(5);
    
    //char (*requestList)[columncount+1]=malloc(rowcount*sizeof(*requestList));

    requestList = malloc(rowcount * sizeof(char*));

    int i=0;
    for(i = 0; i < rowcount; i++) {
      requestList[i] = malloc((columncount + 1) * sizeof(char));
      //strcpy(orderIds[i], your_string[i]);
    }
    
    for(i=0;i<rowcount;i++){
        bzero(requestList[i],columncount);
        
        strcpy(requestList[i],read_line(fd,i));
        //printf("%s \n",requestList[i]);
    }
    close(fd);

    ///////////////
    requestCounter=rowcount;

    
    //sleep(1);
    ///// Initializing the initial barrier/////
    //barrageCount=0;

    pthread_barrier_init(&threadBarrier, NULL, (barrierCOUNT)); // 
    pthread_barrier_init(&threadKillBarrier, NULL, (barrierCOUNT));
    

    barrageCount=rowcount/barrierCOUNT;
    //printf("barrageCount %d\n",barrageCount);

    char writeBuffer1[MAX_BUFF_WRITE];
            
    sprintf(writeBuffer1, "Client: I have read %d requests and I’m creating %d threads.\n",requestCounter,barrierCOUNT);
    write(1, writeBuffer1, strlen(writeBuffer1));
    
    
    //sleep(1);
    //***** Creating Threads ******//
    pthread_t threads[barrierCOUNT];

    for( i = 0; i < barrierCOUNT; i++ ) {            
        int rc = pthread_create(&threads[i], NULL, handleThreadJob, (void *)i);
        if (rc) {
            //printf(" %s \n",requestList[i]);
            perror("Error:unable to create thread.\n");    
            fflush(stdout);        
            exit(-1);
        }
    }


    for( i = 0; i < barrierCOUNT; i++ ) {
        //printf("In Join\n");
        pthread_join(threads[i],NULL);

    }
    //printf("jobDone=%d\n ",jobDone);
    //free(threads);
    
    for(i = 0; i < rowcount; i++) {
      free(requestList[i]);
    }
    free(requestList);

    pthread_barrier_destroy(&threadBarrier);
    pthread_barrier_destroy(&threadKillBarrier);
    pthread_mutex_destroy(&mutex_lock);



    sprintf(writeBuffer1, "Client: The client is terminating. Goodbye\n");
    write(1, writeBuffer1, strlen(writeBuffer1));
    
    return 0;
}




void *handleThreadJob(void *i){
    
	int index = (int)i;

	char writeBuffer2[MAX_BUFF_WRITE]; //printing to terminal and,
    
	char tempRequest[columncount*sizeof(char)];  // gonderilecek soru
    
          
    sprintf(writeBuffer2, "Client: Thread-%d has been created\n", index);
    write(1, writeBuffer2, strlen(writeBuffer2));
       
    int sockfd;
    struct sockaddr_in servaddr;
    //struct sockaddr_in servaddr = {0};
   

    int runamount=0;
    int skipbarrier=0;
    int currentRequest=0;
    while(!exit_requested){
        //sleep(index*2+3);
        // Socket creating and initializing
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("Socket creation failed.\n");
            exit(0);
        }
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(serverIP);
        servaddr.sin_port = htons(serverPORT);
        ///

        
        currentRequest=index+barrierCOUNT*(runamount);
        
        /////// CONDITION WAITING BARRIER ////////
        if(skipbarrier==0){
            //printf("Im Thread-%d, his is my %dth run, I will handle %d,\n",index,runamount,currentRequest);
            pthread_barrier_wait(&threadBarrier);  
        }   

        if(skipbarrier){
            pthread_barrier_wait(&threadKillBarrier);   
            //printf("Im Thread-%d skipped barrier and killing\n",index);       
            pthread_exit(NULL);           
        }

        if((currentRequest+1)>requestCounter){            
            //printf("Im Thread-%d no questions left waiting to terminate\n",index);
            pthread_barrier_wait(&threadKillBarrier);
            //printf("Im Thread-%d killing\n",index);
            pthread_exit(NULL);            
        }
        //printf("runamount=%d",runamount);
        if(runamount==barrageCount){
            //printf("Im Thread-%d I got partial questions %d -%d \n",index,runamount,barrageCount);
            skipbarrier=1;
        }
        
        //printf("Im Thread-%d, sending request %d \n",index,currentRequest);

        // connect the servant socket to server socket
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
            perror("Connection with the server failed.\n");
            pthread_exit(NULL);
        }

        //else
        memset(writeBuffer2, 0, MAX_BUFF_WRITE);

        sprintf(writeBuffer2, "Client: Thread-%d is requesting “%s”\n", index, requestList[currentRequest]);
        write(1, writeBuffer2, strlen(writeBuffer2));
        //printf("Client writing infos to server: %s\n", requestList[requestCounter]);
        

        strcpy(tempRequest, requestList[currentRequest]);

        
        write(sockfd, tempRequest, strlen(tempRequest));
        //printf("valwrite: %d\n",valwrite );
        bzero(tempRequest, strlen(tempRequest));

        
        char replyFromServer[REPLY_LEN];
        int valread = read(sockfd, replyFromServer, REPLY_LEN);
        //printf("---valread---%d\n",valread);
        if(valread < 0){

            perror("Client read failed.\n");
            return NULL;
        }

        memset(writeBuffer2, 0, MAX_BUFF_WRITE);
        sprintf(writeBuffer2, "Client: Thread-%d has received response %s\n", index, replyFromServer);

        write(1, writeBuffer2, strlen(writeBuffer2));

        //printf("GELEN CEVAP : ==== %s \n",replyFromServer);

        close(sockfd);
        
        jobDone++;
        runamount++;

    }
    return 0;
}

char *read_line(int fd, int n)
{
  int line = 0, i = 0, k = 0;
  char c;
  static char buffer[MAX_ROW_LEN];
  while (pread(fd, &c, 1, i++) && line <= n)
  {
    if (line == n)
      buffer[k++] = c;

    if (c == '\n')
    {
      buffer[k - 1] = '\0';
      line++;
    }
  }
  buffer[k] = '\0';
  return buffer;
}

int *lines_columns(int fd)
{
    static int lines_maxcolumns[2];
    char c, last = '\n';

    int i = 0, lines = 0;
    int charcount=0;
    int maxcharcount=0;

    while (pread(fd, &c, 1, i++))
    {

    if (c == '\n' && last != '\n'){
        //printf("line letter count ---  %d\n",charcount);
        lines++;
        charcount=0;
    }

    charcount++;
    if(charcount>=maxcharcount){
        maxcharcount=charcount;
    }

    last = c;

    }

    if (last != '\n'){
        lines++;
    }

    lines_maxcolumns[0]=lines;
    lines_maxcolumns[1]=maxcharcount;
    return lines_maxcolumns;
}