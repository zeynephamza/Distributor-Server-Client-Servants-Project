#include "server.h"


int serverIP;   
int sockfd1;
int sockfd2;
int logfd;
//connfd;

void sig_handler(int sig_no)
{

    if (sig_no == SIGINT){         
        perror("SIGINT Caught!\n");
        exit_requested = 1;
        
        //close(sockfd1);
        //close(sockfd2);
        //close(logfd);
        //close(connfd);
        //exit(0);
    }

}

int main(int argc, char **argv)
{

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sig_handler;
    sigaction(SIGINT, &sa, NULL);



    int serverPORT1;
    int serverPORT2;
    int numOfThreads;
    

    int opt;

    if (argc<4){
        perror("Usage ./server -p PORT1 -q PORT2 -t numberOfThreads\n");
        
        return 1;       
    }
    while((opt = getopt(argc, argv, ":p:q:t:")) != -1) 
    {
        switch(opt) 
        { 
            
            case 'p': 
                serverPORT2 = atoi(optarg);
                //printf("serverPORT1: %d\n", serverPORT1); 
                break; 
            case 'q': 
                serverPORT1 = atoi(optarg);
                //printf("serverPORT2: %d\n", serverPORT2); 
                break;                 
            case 't': 
                numOfThreads = atoi(optarg);
                //printf("numOfThreads: %d\n", numOfThreads); 
                break;
            case '?': 
                perror("Unknown option. Usage ./server -p PORT1 -q PORT2 -t numberOfThreads\n");
                return 0;  
            default:
                perror("Usage ./server -p PORT1 -q PORT2 -t numberOfThreads\n");
                return 0; 
        }

    }

    //creating/opening the log file
    

    if((logfd = open(logfilename, FILE_MODES, USER_PERMS)) == -1){
        perror("Failed to open log file\n");
        return 1;
    }

    
    // Initializing the file descripters struct queue
    initFileDescs1(0,-1,0);
    initFileDescs2(0,-1,0);
    servantCounter = 0;
    pthread_barrier_init(&threadBarrier, NULL, (numOfThreads+1));
    pthread_barrier_init(&clientThreadBarrier,NULL, (numOfThreads+1));    

    /***** Creating Threads *****/
    pthread_t threads[numOfThreads];
    int i;
    for( i = 0; i < numOfThreads; i++ ) {
        //sleep(1);
        

        int rc = pthread_create(&threads[i], NULL, handleThreadJob, (void *)i);
        if (rc) {
            perror("Error:unable to create thread.\n");    
            fflush(stdout);        
            exit(-1);
        }
    }


    socklen_t len;
    struct sockaddr_in servaddr, cli;

    int opti = 1;
   
    // socket create and verification
    sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd1 == -1) {
        perror("Server socket creation failed.\n");
        exit(0);
    }

    sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd2 == -1) {
        perror("Server socket creation failed.\n");
        exit(0);
    }

    //else
        //printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    
    if (setsockopt(sockfd1, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opti))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serverPORT1);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd1, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        perror("Server socket bind failed.\n");
        exit(0);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serverPORT2);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd2, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        perror("Server socket bind failed.\n");
        exit(0);
    }
   // else
        //printf("Socket successfully binded..\n");

    
    
    // Now server is ready to listen and verification
    if ((listen(sockfd1, 128)) != 0) {
        perror("Listen failed.\n");
        exit(0);
    }

    if ((listen(sockfd2, 128)) != 0) {
        perror("Listen failed.\n");
        exit(0);
    }
    //else
        //printf("Server listening..\n");

    //setting up multiple accept tech
    fd_set readfd; 
    int byteswritten;
    int maxfd = -1; 
    int retval = 0;
    int connfd=-1;
    char connfdString[10];
    detectedServantCount=0;
    int detectedRequestCount=0;
    int totalConnection = 0;
    int connectionFromServant=0;
    int connectionFromClient=0;
    int firstClientCame=0;

    char writeBuffer[MAX_BUFF_READ];
    while(!exit_requested){

        //DENEME
        FD_ZERO(&readfd);
        FD_SET(sockfd1, &readfd); //master of sockets
        FD_SET(sockfd2, &readfd);

        maxfd = sockfd1;
        
        if(sockfd2 > sockfd1)
            maxfd = sockfd2;  

        retval = select(maxfd +1 , &readfd, NULL, NULL, NULL);
        if(exit_requested)
            break;

        if ((retval < 0) && (errno!=EINTR))
        {
            printf("select error\n");
            exit(0);
            
        }
        if(retval == 0){
            printf("select() returns 0.\n");
            exit(0);
        }

        if(FD_ISSET(sockfd1, &readfd)){
            len = sizeof(cli);
            connectionFromClient=1;
            connectionFromServant=0;
            //printf("---------TOPLAM BAGLANTI KABULU----------:%d\n");
            // Accept the data packet from client and verification          
            connfd = accept(sockfd1, (struct sockaddr*)&cli, &len);
            //printf("AAA1 question fd is->%d\n",connfd);
        }

        if (FD_ISSET(sockfd2, &readfd)) {
            len = sizeof(cli);
            connectionFromClient=0;
            connectionFromServant=1;            
            //printf("---------TOPLAM BAGLANTI KABULU----------:%d\n");          
            connfd = accept(sockfd2, (struct sockaddr*)&cli, &len);
            detectedServantCount++;
            //printf("AAA2 question fd is->%d\n",connfd);
        }


        //DENEME
        //printf("aaaaaa %s \n" ,connfdString);
       sprintf(connfdString,"%d",connfd);
       //printf("aaaaaa %s \n" ,connfdString);
        if(connfd > 0 ){
            if(connectionFromClient){
                if(firstClientCame==0){//when first client arrives let go of servant threads to let them save themselves

                    pthread_barrier_wait(&threadBarrier);  // ilk defa client geldigi zaman threadler ise baslayabilir artik                    
                    pthread_barrier_wait(&clientThreadBarrier);
                    

                    sprintf(writeBuffer, "\nServer: I’m connected to a total of %d servant processes handling a total of %d cities.\n", detectedServantCount,totalcitycount);
                    write(1, writeBuffer, strlen(writeBuffer));
                    if(((byteswritten = write(logfd, writeBuffer, strlen(writeBuffer))) == -1) || (errno == EINTR)){
                        //perror("Can't write to log file!\n");
                        return 1;
                    }
                    firstClientCame=1;
                }
                detectedRequestCount++;
                totalConnection = totalConnection+1;
                //printf("Server accepted a connection: %d\n ",connfd);            
                
                //printf("trying to accept socket1 \n"); //BURDAYIZ
                if(!exit_requested){
                pthread_mutex_lock(&mutex_job1);
                //printf("AAA2 question fd is->%s\n",connfdString);
                insertFd1(connfdString); 
                //printf("INSERTED TO FD1 \n");
                //printf("fd: %d\n",fileDesc.fdArray[(fileDesc.front)] );
                pthread_cond_signal(&cond_job1);
                pthread_mutex_unlock(&mutex_job1);
                


                //printf("Signalling...\n");
                 // Signal new job
                }
                else{
                    break;
                }


            }
            else if(connectionFromServant){
                //detectedServantCount++;
                totalConnection = totalConnection+1;
                //printf("Server accepted a connection: %d\n ",connfd);            
                
                //printf("trying to accept socket2 \n");
                if(!exit_requested){
                pthread_mutex_lock(&mutex_job2);
                //printf("%s \n",connfdString);
                insertFd2(connfdString); 
                //printf("INSERTED TO FD2 \n");
                //printf("fd: %d\n",fileDesc.fdArray[(fileDesc.front)] );
                pthread_cond_signal(&cond_job2);
                pthread_mutex_unlock(&mutex_job2);


                 // Signal new job  
                }
                else{
                    break;
                }  
            }

        }

        bzero(connfdString, 10);

    }
    
    
    int requestCount= detectedRequestCount;
    
    

    sprintf(writeBuffer, "Server: I've received a total of %d requests. %d of them have been handled. %d has failed. \nGoodbye.\n", requestCount,(requestCount-failedCount),failedCount);
    write(1, writeBuffer, strlen(writeBuffer));    
    if(((byteswritten = write(logfd, writeBuffer, strlen(writeBuffer))) == -1) || (errno == EINTR)){
        //perror("Can't write to log file!\n");
        //return 1;
    }

    close(logfd);


    //printf("\n");
    

    exit_requested = 1;
    pthread_cond_broadcast(&cond_job1);
    pthread_cond_broadcast(&cond_job2); // XXX
    if(exit_requested){ // covering for ctrl+c for before any connection, after servant before client
        //printf("servant %d, request %d \n",detectedServantCount,detectedRequestCount);
        if(firstClientCame==0){
            pthread_barrier_wait(&threadBarrier);
        }
    }
    //printf("DONE Kill %d\n",i);
    pthread_barrier_destroy(&threadBarrier);
    pthread_barrier_destroy(&clientThreadBarrier);

    for( i = 0; i < numOfThreads; i++ ) {
        //printf("In Join\n %d " ,i);
        pthread_join(threads[i],NULL);
    }



    close(sockfd2);
    close(sockfd1);
    return 0;
}



void *handleThreadJob(void *fd){
    char buff[MAX_BUFF_READ];
    char readFromFDarr[MAX_BUFF_READ];
    int connfd;
    int threadIndex= (int)fd;
    int isServant=0;

    //printf("waiting id %d \n ",threadIndex);
    pthread_barrier_wait(&threadBarrier); 
    if(exit_requested){
        exit_requested = 1;
        pthread_cond_broadcast(&cond_job1);
        pthread_cond_broadcast(&cond_job2);
        pthread_exit(NULL);
    }

   // printf("passed id %d \n ",threadIndex);
    //printf("detectedServantCount id %d \n ",detectedServantCount);
    //printf("fileDesc size: %d\n", fileDesc.size);
    RequestList requestList;
    if(threadIndex<detectedServantCount){
        isServant=1;
        //printf("Iam thread- %d Im servant thread\n",threadIndex );
    }else{
        //printf("Iam thread- %d Im client thread\n",threadIndex );
    }

        
    if(isServant==0){ //this part is for client handling threads
        //printf("FLAG1 \n");
        //printf("threadIndex=%d \n", threadIndex);
        pthread_barrier_wait(&clientThreadBarrier); //wait untill all servants are saved
        //printf("THREADINDEX=%d \n", threadIndex);
        //printf("FLAG2 \n");
        if(exit_requested){
            //printf("aaa \n");
            exit_requested = 1;
            pthread_cond_broadcast(&cond_job1);
            pthread_cond_broadcast(&cond_job2); 
            pthread_exit(NULL);;
        }        


        char writingToFD2[MAX_BUFF_READ];
        while (!exit_requested)
        {
            //printf("----\n");

            pthread_mutex_lock(&mutex_job1); //mutex lock
            while ((fileDesc1.size) == 0)
            {
                //printf("In while\n");
                //print_log("Thread #%d: waiting for job.", td->id);
                pthread_cond_wait(&cond_job1, &mutex_job1); //wait for the condition
                if (exit_requested)
                {
                    //printf("Thread terminatingsadasd.\n");
                    pthread_mutex_unlock(&mutex_job1);
                    return NULL;
                }
            }

            // Function for chatting between client and server
            bzero(buff, MAX_BUFF_READ);

            //printf("fileDesc size: %d\n", fileDesc.size);

            strcpy(readFromFDarr,fileDesc1.fdArray[(fileDesc1.front)]);  
            removeFd1();                        // if question is there take it and unlock the fd array
            pthread_mutex_unlock(&mutex_job1);

            //printf("XXXX--fileDesc.fdArray fd: %s\n", readFromFDarr );

            //DETERMINE IF FETCHED STRING IS FROM CLIENT OR FORWARDED ANSWER FROM SERVANT
            
            char * token = strtok(readFromFDarr, " ");
            //char fdtoken[10];
            char fdAnswer[MAX_BUFF_FD];
            
            if( token != NULL ) { //tokenizing string from fd array in to FD and Question
                //printf( "Im thread %d read FD2, included fd is '%s'\n",threadIndex, token );   
                connfd=atoi(token); 
                //printf("AAA3 question fd is->%d\n",connfd);
                //printf("XXXXXXX- confd=%d\n", connfd);   

                token = strtok(NULL, "\n");
            }

            if( token != NULL ) { //CHECK IF THERE IS ANSWER INCLUDED
                //fd haric birsey varsa buraya giriyom
                //printf( "Im thread %d read FD2, included request is '%s'\n",threadIndex, token );                 
                //printf("there is answer fd Answer=%s \n",token);
                strcpy(fdAnswer, token);              
            }

            /////////////////////////////////////////////////////////////////////////

            if (token == NULL){ // IF NO ANSWER IS THERE, THE CONNECTION IS FROM CLIENT READ THE CONTENTS FORWARD IT TO FD2
                //printf("FLAG69699\n");
                //printf("AAA4 question fd is->%d\n",connfd);

                int valread = read(connfd, buff, MAX_BUFF_READ);
                if(valread < 0){
                    perror("Server read failed3.\n");
                    return NULL;
                }

                sprintf(writingToFD2,"%d %s",connfd,buff);


                pthread_mutex_lock(&mutex_job2);

                //printf("AAA5 question fd is->%d\n",connfd);
                insertFd2(writingToFD2);
                pthread_cond_signal(&cond_job2);
                pthread_mutex_unlock(&mutex_job2);
                 

                //printf("From client: %s, %d\n", buff, valread);


                

                //printf("serOrCli: %s\n", serOrCli );
                


            }
            else{// if there is answer, take the fd part and send it fdAnswer
                //printf("Sending fdAnswer to the Clientfd%d-> %s\n",connfd,fdAnswer);
                //printf("AAA8 question fd is->%d\n",connfd);
                char buffToSendClient[MAX_BUFF_READ];
                memset(buffToSendClient, 0, MAX_BUFF_READ);
                strcpy(buffToSendClient,fdAnswer);

                //printf("sending this thing %s\n",buffToSendClient);
                write(connfd, buffToSendClient, sizeof(buffToSendClient)); //cliente gonderiyorum cevabi
                //printf("Oh dear, something went wrong with write()! %s\n", strerror(errno));
                bzero(buffToSendClient, strlen(buffToSendClient));
                close(connfd); //yeni
            }


        }

        //////clientThreadJobs;
    }//next part is for servant handling threads
    else if(isServant==1){


        char writingToFD1[MAX_BUFF_READ];
        while (!exit_requested)
        {
            //printf("----\n");

            pthread_mutex_lock(&mutex_job2); //mutex lock
            while ((fileDesc2.size) == 0)
            {
                //printf("In while\n");
                //print_log("Thread #%d: waiting for job.", td->id);
                pthread_cond_wait(&cond_job2, &mutex_job2); //wait for the condition
                if (exit_requested)
                {
                    //printf("Thread terminating.\n");
                    pthread_mutex_unlock(&mutex_job2);
                    return NULL;
                }
            }

            // Function for chatting between client and server
            bzero(buff, MAX_BUFF_READ);

            //printf("fileDesc size: %d\n", fileDesc.size);

            strcpy(readFromFDarr,fileDesc2.fdArray[(fileDesc2.front)]);  
            removeFd2();                        // if question is there take it and unlock the fd array
            if(threadIndex==0){
                //sleep(5);
            }
            //printf("IM INSIDE CRITICAL SECTION \n");
            pthread_mutex_unlock(&mutex_job2);

            //printf("fileDesc.fdArray fd: %d\n", connfd );

            //DETERMINE IF FETCHED STRING IS FROM SERVANT OR FORWARDED QUESTION FROM CLIENT
            char * token = strtok(readFromFDarr, " ");
            //char fdtoken[10];
            char fdRequest[MAX_BUFF_FD];
            //printf( "Im thread %d read this FROM FD2 '%s'\n",threadIndex, readFromFDarr );

            if( token != NULL ) { //tokenizing string from fd array in to FD and Question
                //printf( "Im thread %d read FD2, included fd is '%s'\n",threadIndex, token ); 
                connfd=atoi(token); 
                //printf("%d\n", connfd);   

                token = strtok(NULL, "\n");
            }

            if( token != NULL ) { 
                //fd haric birsey varsa buraya giriyom
                //printf( "Im thread %d read FD2, included request is '%s'\n",threadIndex, token );                 
                //printf("there is question \n");
                strcpy(fdRequest, token);   
                //printf("AAA6 question fd is->%d\n",connfd);           
 
            }
            
            //if(threadIndex==0)
             //   printf("Im saving this servant \n");



            if (token == NULL){ // IF NO QUESTION IS THERE, THE CONNECTION IS FROM SERVANT  SAVE IT TO THE REHBER
                //printf("FLAG3113\n");

                int valread = read(connfd, buff, MAX_BUFF_READ);
                if(valread < 0){
                    perror("Server read failed4.\n");
                    return NULL;
                }

                char bufferTemp[MAX_BUFF_READ];
                char serOrCli[MAX_BUFF_READ];

                strcpy(bufferTemp, buff);

                char * token = strtok(bufferTemp, " ");
                if( token != NULL ) {
                    //printf( "aaa '%s'\n", token ); 
                    //startingCityResponsible = atoi(token) -1;
                    strcpy(serOrCli, token);            

                    token = strtok(NULL, " ");
                    
                }


                //printf("From client: %s, %d\n", buff, valread);


                //eger servanttan ise rehbere kaydet

                //printf("serOrCli: %s\n", serOrCli );

                if((strcmp(serOrCli, "servant")) == 0){   // Socket info came from servant
                    //printf("Servant\n");
                    //if(threadIndex==0) //thread 0 logging
                    //    printf("Im saving this servant \n");

                    if( token != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        strcpy((servantInfo[servantCounter].startCity), token);
                        //endingCityResponsible = atoi(token) -1;
                        token = strtok(NULL, " ");
                    }
                    if( token != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        //endingCityResponsible = atoi(token) -1;
                        strcpy((servantInfo[servantCounter].endCity), token);
                        //printf("city %s \n",servantInfo[servantCounter].endCity);
                        token = strtok(NULL, " ");
                    }
                    if( token != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        //endingCityResponsible = atoi(token) -1;
                        (servantInfo[servantCounter].servantPORT) = atoi(token);
                        token = strtok(NULL, " ");
                    }
                    if( token != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        //endingCityResponsible = atoi(token) -1;
                        totalcitycount+= atoi(token);
                        token = strtok(NULL, " ");
                    }
                    // print buffer which contains the client contents

                    
                    //printf("fileDesc size: %d, fd: %d\n", fileDesc.size, fileDesc.fdArray[(fileDesc.front)]);
                    servantCounter++;
                    //printf("Im ServantThread%d saved a servant and waiting other servants to save \n",threadIndex);
                    //printf("threadIndex=%d \n", threadIndex);
                    pthread_barrier_wait(&clientThreadBarrier); //waiting in client barrier to let them pass after saving  // FD ARRAYI STRING YAPIP ICINI TOKEN YAPIP FD MI YOKSA SORU MU ANLA
                    //printf("THREADINDEX=%d \n", threadIndex);
                    if(exit_requested){
                        exit_requested = 1;
                        pthread_cond_broadcast(&cond_job1);
                        pthread_cond_broadcast(&cond_job2);
                        pthread_exit(NULL);;
                    }
                }


            }
            else{
                //printf("There is question and it is %s\n,",fdRequest);
                char questionType[MAX_BUFF_READ];

                char * tokenreq = strtok(fdRequest, " ");
                if( tokenreq != NULL ) {
                    //printf( "aaa '%s'\n", token ); 
                    //startingCityResponsible = atoi(token) -1;
                    strcpy(questionType, tokenreq);            

                    tokenreq = strtok(NULL, " ");
                }

                char writeBuffer[MAX_BUFF_READ];
                int i;
                int notfailed=0;

                if((strcmp(questionType, "/transactionCount")) == 0){   // TransactionCount sorusu
                    //printf("Client\n");
                    char startDate[DATE_LEN];
                    char endDate[DATE_LEN];
                    int retStrtDay;
                    int retEndDay;
                    
                    

                    sprintf(writeBuffer, "Request arrived “%s”\n", fdRequest);

                    //write(1, writeBuffer, strlen(writeBuffer));
                                
                    if( tokenreq != NULL ) {
                        
                        strcpy((requestList.realEstate), tokenreq);
                        //printf( "estatetype: '%s'\n", (requestList.realEstate) ); 
                       
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        //endingCityResponsible = atoi(token) -1;
                        strcpy(startDate, tokenreq);
                        //strcpy((servantInfo[servantCounter].endCity), token);
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        strcpy(endDate, tokenreq);
                        //endingCityResponsible = atoi(token) -1;
                        //(servantInfo[servantCounter].servantPORT) = atoi(token);
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        //endingCityResponsible = atoi(token) -1;
                        //(servantInfo[servantCounter].servantPORT) = atoi(token);
                        strcpy((requestList.cityName), tokenreq);
                        //printf( "city name:  '%s'\n", (requestList.cityName) );

                        tokenreq = strtok(NULL, " ");
                    }else{
                        strcpy((requestList.cityName), "");
                        //printf( "cityname is empty: '%s'\n", (requestList.cityName) );
                    }
                    // print buffer which contains the client contents
                    //printf("fileDesc size: %d, fd: %d\n", fileDesc.size, fileDesc.fdArray1[(fileDesc.front)]);
                    

                    retStrtDay = convertDateToDay(startDate);
                    retEndDay  = convertDateToDay(endDate);

                    requestList.startDate = retStrtDay;
                    requestList.endDate   = retEndDay;


                    //printf("Loaded request:\nestate: %s, strt: %d, end: %d, city: %s\n",requestList.realEstate, requestList.startDate, requestList.endDate, requestList.cityName);
                    //sleep(3);



                    
                    //servanta soruyu ilet
                    if(strcmp(requestList.cityName , "") == 0){ // NO CITY MENTIONED ASK EVERYONE
                        //printf("Send em all\n");


                        char buffToSendServant[MAX_BUFF_READ];
                        memset(buffToSendServant, 0, MAX_BUFF_READ);
                        sprintf(buffToSendServant, "%s %s %d %d", "transactionCount",
                                                                requestList.realEstate,
                                                               requestList.startDate,
                                                               requestList.endDate );


                        int servsockfd, servconnfd;
                        struct sockaddr_in servaddr;
                        int totalTransaction = 0;
                        char replyFromServant[DATE_LEN];
                        
                        //printf("buffToSendServant: %s\n", buffToSendServant);
                        for( i = 0; i< servantCounter; ++i){


                                                                  
                            // socket create and varification
                            servsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (servsockfd == -1) {
                                perror("Server thread socket creation failed.\n");
                                exit(0);
                            }
                            //else
                                //printf("Socket successfully created..\n");
                            bzero(&servaddr, sizeof(servaddr));
                           
                            // assign IP, PORT
                            servaddr.sin_family = AF_INET;
                            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                            servaddr.sin_port = htons(servantInfo[i].servantPORT);

                            memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer, "Contacting servant %d\n", servantInfo[i].servantPORT);

                            // connect the servant socket to server socket
                            if ((servconnfd=connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
                                perror("Connection with the servant failed.\n");
                                exit(0);
                            }
                            else
                                //write(1, writeBuffer, strlen(writeBuffer));

                          

                            write(servsockfd, buffToSendServant, sizeof(buffToSendServant));
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("servsockfd: %d\n",servsockfd);
                            //}

                            //memset(buffToSendServant,0, MAX_BUFF_READ );


                            // Reading response from the servant
                            
                            int valread = read(servsockfd, replyFromServant, DATE_LEN);


                            totalTransaction += atoi(replyFromServant);
                            //printf("totalTransaction: %d\n", totalTransaction );
                            //close(servsockfd); //yeni
                            if(valread < 0){
                                perror("Server read failed1.\n");
                                return NULL;
                            }

                            

                            close(servsockfd);


                        }
                        //printf("TOTAL: totalTransaction: %d\n",totalTransaction );
                        memset(replyFromServant, 0, sizeof(replyFromServant));
                        sprintf(replyFromServant, "%d",totalTransaction);

                        memset(writeBuffer, 0, MAX_BUFF_READ);
                        sprintf(writeBuffer,"Response received: %s, forwarded to client\n",replyFromServant);
                        //write(1, writeBuffer, strlen(writeBuffer));

                        // Sending the servant's answer to the client.
                        //printf("AAA7 question fd is->%d\n",connfd);

                        // BURADA ALINAN CEVAP FDARRAY1 e connfd reply olarak insert edilecek
                        sprintf(writingToFD1,"%d %s",connfd,replyFromServant);
                        //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                        
                        pthread_mutex_lock(&mutex_job1);
                        insertFd1(writingToFD1);
                        pthread_cond_signal(&cond_job1);
                        pthread_mutex_unlock(&mutex_job1);
                         
                        //write(connfd, replyFromServant, sizeof(replyFromServant));


                        //close(servconnfd);
                        //close(servsockfd);

                    }
                    else{   //CITY MENTIONED, ASK WHO IS RESPONSIBLE
                        //printf("Servant searching for: %s\n", (requestList.cityName));
                        
                        int servantIndexToSend;
                        char buffToSendServant[MAX_BUFF_READ];
                        int sendflag =0;
                        int servantPortToSend= -1;
                        // Socket creating and initializing     
                        int servsockfd, servconnfd;
                        struct sockaddr_in servaddr;
                        //printf("%d\n ",servantCounter);
                        for (i = 0; i < servantCounter; ++i)
                        {
                            //printf("word1: %s\n", (servantInfo[i].startCity));
                            //printf("word2: %s\n", (servantInfo[i].endCity));
                            //printf("word3: %s\n", ((requestList.cityName)));
                            int x=sort_words((servantInfo[i].startCity),(servantInfo[i].endCity),(requestList.cityName));

                            //printf ("%d\n",x);

                            if(x == 0){
                                servantIndexToSend = i;
                                sendflag=1; //sehirden sorumlu servant bulunur ise
                                //printf("Servant found:%d %s-%s\n",i,(servantInfo[i].startCity), (servantInfo[i].endCity));
                                break;
                            }
                        }
                        if(sendflag==0){
                           //There is no servant that responsible from the city. Sending client '0'.    
                           
                            write(connfd, "0", DATE_LEN);
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("ERRNO %s\n", strerror(errno));
                                //printf("servsockfd: %d\n",servsockfd);
                            //} 
                        }
                        else if(sendflag==1){ //Responsibled servant found.
                            sendflag=0;
                            memset(buffToSendServant, 0, MAX_BUFF_READ);
                            sprintf(buffToSendServant, "%s %s %d %d %s", "transactionCount",
                                                                    requestList.realEstate,
                                                                   requestList.startDate,
                                                                   requestList.endDate ,
                                                                   requestList.cityName);
                            


                           
                            // socket create and varification
                            servsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (servsockfd == -1) {
                                perror("Socket creation failed.\n");
                                exit(0);
                            }
                            //else
                                //printf("Socket successfully created..\n");
                            bzero(&servaddr, sizeof(servaddr));
                           
                            // assign IP, PORT
                            servantPortToSend = servantInfo[servantIndexToSend].servantPORT;

                            servaddr.sin_family = AF_INET;
                            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                            servaddr.sin_port = htons(servantPortToSend);
                            //printf("will connecto servant: %d\n",servantPortToSend);
                            
                            memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer, "Contacting servant %d\n", servantInfo[i].servantPORT);

                            // connect the servant socket to server socket
                            if ((servconnfd = connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
                                perror("Connection with the servant failed.\n");
                                exit(0);
                            }
                            //else
                                //write(1, writeBuffer, strlen(writeBuffer));

                            
                            
                            int valwrite;
                            while(!exit_requested){
                                valwrite = write(servsockfd, buffToSendServant, sizeof(buffToSendServant));
                                if(valwrite>1){
                                    break;
                                }
                            } 

                                     
                            char replyFromServant[DATE_LEN];
                            int valread = read(servsockfd, replyFromServant, DATE_LEN);
                            if(valread < 0){
                                perror("Server read from servant failed.\n");
                                return NULL;
                            }
                            
                            /*memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer,"Response received from Servant: %s,\n",replyFromServant);
                            write(1, writeBuffer, strlen(writeBuffer));
                            */
                           // printf("AAA7 question fd is->%d\n",connfd);
                            // Sending the servant's answer to the client.
                            sprintf(writingToFD1,"%d %s",connfd,replyFromServant);
                            //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                            
                            pthread_mutex_lock(&mutex_job1);
                            insertFd1(writingToFD1);
                            pthread_cond_signal(&cond_job1);
                            pthread_mutex_unlock(&mutex_job1);
                             


                            //valwrite = write(connfd, replyFromServant, sizeof(replyFromServant));
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("ERRNO %s\n", strerror(errno));
                                //printf("servsockfd: %d\n",servsockfd);
                            //}

                            close(servsockfd);

                        }

                                        

                    }
                    
                    

                    //servanttan cevabıbekle

                    //aldıgın int sayıyı cliente gönder 
                    
                     
                    
                }
                else if((strcmp(questionType, "/transactionId")) == 0){
                    
                    
                    /*char requestedID[DATE_LEN];         
                    if( token != NULL ) {
                        
                        strcpy((requestedID), token);
                        //printf( "estatetype: '%s'\n", (requestList.realEstate) ); 
                       
                        token = strtok(NULL, " ");
                    }*/
                    //printf("THE QUESTION IS ABOUT ID%s \n",requestedID);
                        
                        char buffToSendServant[MAX_BUFF_READ];
                        memset(buffToSendServant, 0, MAX_BUFF_READ);
                        strcpy((buffToSendServant), tokenreq);
                        //sprintf(buffToSendServant, "IDQUESTION %s",requestedID);


                        int servsockfd, servconnfd;
                        struct sockaddr_in servaddr;
                        //int totalTransaction = 0;
                        char replyFromServant[DATE_LEN];
                        char replytoForwardtoClient[DATE_LEN];
                        
                        //printf("buffToSendServant: %s\n", buffToSendServant);
                        for( i = 0; i< servantCounter; ++i){


                                                                  
                            // socket create and varification
                            servsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (servsockfd == -1) {
                                perror("Server thread socket creation failed.\n");
                                exit(0);
                            }
                            //else
                                //printf("Socket successfully created..\n");
                            bzero(&servaddr, sizeof(servaddr));
                           
                            // assign IP, PORT
                            servaddr.sin_family = AF_INET;
                            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                            servaddr.sin_port = htons(servantInfo[i].servantPORT);

                            memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer, "Contacting servant %d\n", servantInfo[i].servantPORT);

                            // connect the servant socket to server socket
                            if ((servconnfd=connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
                                perror("Connection with the servant failed.\n");
                                exit(0);
                            }
                            else
                                //write(1, writeBuffer, strlen(writeBuffer));

                          

                            write(servsockfd, buffToSendServant, sizeof(buffToSendServant));
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("servsockfd: %d\n",servsockfd);
                            //}

                            //memset(buffToSendServant,0, MAX_BUFF_READ );

                            //printf("AAAAAAAAAAAAAAAAAAA\n");
                            // Reading response from the servant
                            
                            int valread = read(servsockfd, replyFromServant, DATE_LEN);  // ALDIGIM CEVAP STRING OLACAK BURDAYIM                   
                            
                            
                            if(strcmp(replyFromServant,"no")==0){ //reply is string
                                

                            }else{
                                strcpy(replytoForwardtoClient,replyFromServant);
                                notfailed=1;
                            }


                            //printf("totalTransaction: %d\n", totalTransaction );
                            if(valread < 0){
                                perror("Server read failed2.\n");
                                return NULL;
                            }
                            close(servsockfd);

                        }

                        if(notfailed==0){
                            failedCount++;
                            memset(replytoForwardtoClient, 0, sizeof(replytoForwardtoClient));
                            sprintf(replytoForwardtoClient, "%s","This ID does not exist");

                        }
                        //printf("TOTAL: totalTransaction: %d\n",totalTransaction );


                        //memset(writeBuffer, 0, MAX_BUFF_READ);
                       // sprintf(writeBuffer,"Response received: %s, forwarded to client\n",replytoForwardtoClient);
                        //write(1, writeBuffer, strlen(writeBuffer));

                        sprintf(writingToFD1,"%d %s",connfd,replytoForwardtoClient);
                            //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                            
                        pthread_mutex_lock(&mutex_job1);
                        insertFd1(writingToFD1);
                        pthread_cond_signal(&cond_job1);
                        pthread_mutex_unlock(&mutex_job1);
                         

                       //close(servsockfd);//yeni
                        /*
                */
                }
                else if((strcmp(questionType, "/topTransaction")) == 0){   // Socket info came from client
                    //printf("Client\n");
                    char startDate[DATE_LEN];
                    char endDate[DATE_LEN];
                    int retStrtDay;
                    int retEndDay;
                    
                    

                    sprintf(writeBuffer, "Request arrived “%s”\n", fdRequest);

                    //write(1, writeBuffer, strlen(writeBuffer));
                                
                    if( tokenreq != NULL ) {
                        
                        strcpy((requestList.realEstate), tokenreq);
                        //printf( "estatetype: '%s'\n", (requestList.realEstate) ); 
                       
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        //endingCityResponsible = atoi(token) -1;
                        strcpy(startDate, tokenreq);
                        //strcpy((servantInfo[servantCounter].endCity), token);
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        strcpy(endDate, tokenreq);
                        //endingCityResponsible = atoi(token) -1;
                        //(servantInfo[servantCounter].servantPORT) = atoi(token);
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        //printf( "aaa '%s'\n", token ); 
                        //endingCityResponsible = atoi(token) -1;
                        //(servantInfo[servantCounter].servantPORT) = atoi(token);
                        strcpy((requestList.cityName), tokenreq);
                        //printf( "city name:  '%s'\n", (requestList.cityName) );

                        tokenreq = strtok(NULL, " ");
                    }else{
                        strcpy((requestList.cityName), "");
                        //printf( "cityname is empty: '%s'\n", (requestList.cityName) );
                    }
                    // print buffer which contains the client contents
                    //printf("fileDesc size: %d, fd: %d\n", fileDesc.size, fileDesc.fdArray1[(fileDesc.front)]);
                    

                    retStrtDay = convertDateToDay(startDate);
                    retEndDay  = convertDateToDay(endDate);

                    requestList.startDate = retStrtDay;
                    requestList.endDate   = retEndDay;


                    //printf("Loaded request:\nestate: %s, strt: %d, end: %d, city: %s\n",requestList.realEstate, requestList.startDate, requestList.endDate, requestList.cityName);
                    //sleep(3);



                    
                    //servanta soruyu ilet
                    if(strcmp(requestList.cityName , "") == 0){ // NO CITY MENTIONED ASK EVERYONE
                        //printf("Send em all\n");


                        char buffToSendServant[MAX_BUFF_READ];
                        memset(buffToSendServant, 0, MAX_BUFF_READ);
                        sprintf(buffToSendServant, "%s %s %d %d", "topTransaction",
                                                                requestList.realEstate,
                                                               requestList.startDate,
                                                               requestList.endDate );

                        
                        int servsockfd, servconnfd;
                        struct sockaddr_in servaddr;
                        //int totalTransaction = 0;
                        char replyFromServant[DATE_LEN];
                        memset(replyFromServant, 0, DATE_LEN);

                        int replyReceived=0;
                        char replyFromServantTEMP[DATE_LEN];
                        memset(replyFromServantTEMP, 0, DATE_LEN);
                        char stringtoForward[DATE_LEN];
                        memset(stringtoForward, 0, DATE_LEN);

                        char stringtoForwardTEMP[DATE_LEN];
                        memset(stringtoForwardTEMP, 0, DATE_LEN);
                        //to compare with incoming reply
                        int priceTEMP=-1;
                        int priceOLD=-1;
                        //printf("buffToSendServant: %s\n", buffToSendServant);
                        for( i = 0; i< servantCounter; ++i){


                                                                  
                            // socket create and varification
                            servsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (servsockfd == -1) {
                                perror("Server thread socket creation failed.\n");
                                exit(0);
                            }
                            //else
                                //printf("Socket successfully created..\n");
                            bzero(&servaddr, sizeof(servaddr));
                           
                            // assign IP, PORT
                            servaddr.sin_family = AF_INET;
                            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                            servaddr.sin_port = htons(servantInfo[i].servantPORT);

                            memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer, "Contacting servant %d\n", servantInfo[i].servantPORT);

                            // connect the servant socket to server socket
                            if ((servconnfd=connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
                                perror("Connection with the servant failed.\n");
                                exit(0);
                            }
                            else
                                //write(1, writeBuffer, strlen(writeBuffer));

                          

                            write(servsockfd, buffToSendServant, sizeof(buffToSendServant));
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("servsockfd: %d\n",servsockfd);
                            //}

                            //memset(buffToSendServant,0, MAX_BUFF_READ );


                            // Reading response from the servant
                            
                            int valread = read(servsockfd, replyFromServant, DATE_LEN);
                            //printf("reply from servant for TOPCOUNT %s \n", replyFromServant);
                            if(strcmp(replyFromServant,"noanswer")==0){

                            }
                            else{
                                replyReceived=1;
                                strcpy(replyFromServantTEMP,replyFromServant);
                                char * token = strtok(replyFromServantTEMP, " ");
                                if( token != NULL ) {
                                    token = strtok(NULL, " ");
                                }
                                if( token != NULL ) {
                                    token = strtok(NULL, " ");
                                }
                                if( token != NULL ) {
                                    token = strtok(NULL, " ");

                                }
                                if( token != NULL ) {
                                    token = strtok(NULL, " ");
                                }                                
                                priceTEMP=atoi(token);

                                if(priceTEMP>priceOLD){

                                    strcpy(stringtoForward,replyFromServant);
                                    priceOLD=priceTEMP;
                                }
                                else if(priceTEMP==priceOLD){
                                    strcpy(stringtoForwardTEMP,stringtoForward);
                                    memset(stringtoForward, 0, DATE_LEN);
                                    sprintf(stringtoForward,"%s %s",stringtoForwardTEMP,replyFromServant);
                                    memset(stringtoForwardTEMP, 0, DATE_LEN);
                                }
                                else if(priceTEMP<priceOLD){

                                }


                            }



                            
                            //printf("totalTransaction: %d\n", totalTransaction );
                            //close(servsockfd); //yeni
                            if(valread < 0){
                                perror("Server read failed1.\n");
                                return NULL;
                            }

                            
                            close(servsockfd);
                            


                        }


                        if(replyReceived==0){
                            strcpy(stringtoForward,"No Top Transactions");
                            failedCount++;
                        }

                        //printf("TOTAL: totalTransaction: %d\n",totalTransaction );
                        memset(replyFromServant, 0, sizeof(replyFromServant));
                        strcpy(replyFromServant, stringtoForward);

                        memset(writeBuffer, 0, MAX_BUFF_READ);
                        sprintf(writeBuffer,"Response received: %s, forwarded to client\n",replyFromServant);

                        //write(1, writeBuffer, strlen(writeBuffer));

                        // Sending the servant's answer to the client.
                        //printf("AAA7 question fd is->%d\n",connfd);

                        // BURADA ALINAN CEVAP FDARRAY1 e connfd reply olarak insert edilecek
                        sprintf(writingToFD1,"%d %s",connfd,replyFromServant);
                        //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                        
                        pthread_mutex_lock(&mutex_job1);
                        insertFd1(writingToFD1);
                        pthread_cond_signal(&cond_job1);
                        pthread_mutex_unlock(&mutex_job1);
                         
                        //write(connfd, replyFromServant, sizeof(replyFromServant));


                        //close(servconnfd);
                        //close(servsockfd);

                    }
                    else{   //CITY MENTIONED, ASK WHO IS RESPONSIBLE
                        //printf("Servant searching for: %s\n", (requestList.cityName));
                        
                        int servantIndexToSend;
                        char buffToSendServant[MAX_BUFF_READ];
                        int sendflag =0;
                        int servantPortToSend= -1;
                        // Socket creating and initializing     
                        int servsockfd, servconnfd;
                        struct sockaddr_in servaddr;
                        for (i = 0; i < servantCounter; ++i)
                        {
                            //printf("word1: %s\n", (servantInfo[i].startCity));
                            //printf("word2: %s\n", (servantInfo[i].endCity));
                            //printf("word3: %s\n", ((requestList.cityName)));
                            int x=sort_words((servantInfo[i].startCity),(servantInfo[i].endCity),(requestList.cityName));

                            //printf ("%d\n",x);

                            if(x == 0){
                                servantIndexToSend = i;
                                sendflag=1; //sehirden sorumlu servant bulunur ise
                                //printf("Servant found:%d %s-%s\n",i,(servantInfo[i].startCity), (servantInfo[i].endCity));
                                break;
                            }
                        }
                        if(sendflag==0){
                           //There is no servant that responsible from the city. Sending client '0'.    
                           
                            write(connfd, "0", DATE_LEN);
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("ERRNO %s\n", strerror(errno));
                                //printf("servsockfd: %d\n",servsockfd);
                            //} 
                        }
                        else if(sendflag==1){ //Responsibled servant found.
                            sendflag=0;
                            memset(buffToSendServant, 0, MAX_BUFF_READ);
                            sprintf(buffToSendServant, "%s %s %d %d %s", "topTransaction",
                                                                    requestList.realEstate,
                                                                    requestList.startDate,
                                                                    requestList.endDate ,
                                                                    requestList.cityName);
                            


                           
                            // socket create and varification
                            servsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (servsockfd == -1) {
                                perror("Socket creation failed.\n");
                                exit(0);
                            }
                            //else
                                //printf("Socket successfully created..\n");
                            bzero(&servaddr, sizeof(servaddr));
                           
                            // assign IP, PORT
                            servantPortToSend = servantInfo[servantIndexToSend].servantPORT;

                            servaddr.sin_family = AF_INET;
                            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                            servaddr.sin_port = htons(servantPortToSend);
                            //printf("will connecto servant: %d\n",servantPortToSend);
                            
                            memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer, "Contacting servant %d\n", servantInfo[i].servantPORT);

                            // connect the servant socket to server socket
                            if ((servconnfd = connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
                                perror("Connection with the servant failed.\n");
                                exit(0);
                            }
                            //else
                                //write(1, writeBuffer, strlen(writeBuffer));

                            
                            
                            int valwrite;
                            while(!exit_requested){
                                valwrite = write(servsockfd, buffToSendServant, sizeof(buffToSendServant));
                                if(valwrite>1){
                                    break;
                                }
                            } 

                                     
                            char replyFromServant[DATE_LEN];
                            int valread = read(servsockfd, replyFromServant, DATE_LEN);
                            //printf("reply from servant for TOPCOUNT %s %d \n", replyFromServant,strcmp(replyFromServant,"noanswer"));
                            if(valread < 0){
                                perror("Server read from servant failed.\n");
                                return NULL;
                            }

                            if(strcmp(replyFromServant,"noanswer")==0){
                                strcpy(replyFromServant,"No Top Transactions");
                                failedCount++;
                            }

                            
                            /*memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer,"Response received from Servant: %s,\n",replyFromServant);
                            write(1, writeBuffer, strlen(writeBuffer));
                            */
                           // printf("AAA7 question fd is->%d\n",connfd);
                            // Sending the servant's answer to the client.
                            sprintf(writingToFD1,"%d %s",connfd,replyFromServant);
                            //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                            
                            pthread_mutex_lock(&mutex_job1);
                            insertFd1(writingToFD1);
                            pthread_cond_signal(&cond_job1); 
                            pthread_mutex_unlock(&mutex_job1);
                            


                            //valwrite = write(connfd, replyFromServant, sizeof(replyFromServant));
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("ERRNO %s\n", strerror(errno));
                                //printf("servsockfd: %d\n",servsockfd);
                            //}

                            close(servsockfd);

                        }

                                        

                    }
                    
                    

                    //servanttan cevabıbekle

                    //aldıgın int sayıyı cliente gönder 
                    
                     
                    
                }                     
                else if((strcmp(questionType, "/surfaceCount")) == 0){   // Socket info came from client
                    //printf("Client\n");
                

                    sprintf(writeBuffer, "Request arrived “%s”\n", fdRequest);

                    //write(1, writeBuffer, strlen(writeBuffer));
                                
                    if( tokenreq != NULL ) {
                        
                        strcpy((requestList.realEstate), tokenreq);
                        //printf( "estatetype: '%s'\n", (requestList.realEstate) ); 
                       
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {                      
                        requestList.startSurfaceCount=atoi(tokenreq);
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        requestList.endSurfaceCount=atoi(tokenreq);
                        tokenreq = strtok(NULL, " ");
                    }
                    if( tokenreq != NULL ) {
                        strcpy((requestList.cityName), tokenreq);
                        tokenreq = strtok(NULL, " ");
                    }else{
                        strcpy((requestList.cityName), "");
                        //printf( "cityname is empty: '%s'\n", (requestList.cityName) );
                    }
                    // print buffer which contains the client contents
                    //printf("fileDesc size: %d, fd: %d\n", fileDesc.size, fileDesc.fdArray1[(fileDesc.front)]);


                    //printf("Loaded request:\nestate: %s, strt: %d, end: %d, city: %s\n",requestList.realEstate, requestList.startDate, requestList.endDate, requestList.cityName);
                    //sleep(3);



                    
                    //servanta soruyu ilet
                    if(strcmp(requestList.cityName , "") == 0){ // NO CITY MENTIONED ASK EVERYONE
                        //printf("Send em all\n");


                        char buffToSendServant[MAX_BUFF_READ];
                        memset(buffToSendServant, 0, MAX_BUFF_READ);
                        sprintf(buffToSendServant, "%s %s %d %d", "surfaceCount",
                                                                requestList.realEstate,
                                                               requestList.startSurfaceCount,
                                                               requestList.endSurfaceCount );

                        

                        int servsockfd, servconnfd;
                        struct sockaddr_in servaddr;
                        int totalTransaction = 0;
                        char replyFromServant[DATE_LEN];
                           //printf("buffToSendServant: %s\n", buffToSendServant);
                        for( i = 0; i< servantCounter; ++i){


                                                                  
                            // socket create and varification
                            servsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (servsockfd == -1) {
                                perror("Server thread socket creation failed.\n");
                                exit(0);
                            }
                            //else
                                //printf("Socket successfully created..\n");
                            bzero(&servaddr, sizeof(servaddr));
                           
                            // assign IP, PORT
                            servaddr.sin_family = AF_INET;
                            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                            servaddr.sin_port = htons(servantInfo[i].servantPORT);

                            memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer, "Contacting servant %d\n", servantInfo[i].servantPORT);

                            // connect the servant socket to server socket
                            if ((servconnfd=connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
                                perror("Connection with the servant failed.\n");
                                exit(0);
                            }
                            else
                                //write(1, writeBuffer, strlen(writeBuffer));

                          

                            write(servsockfd, buffToSendServant, sizeof(buffToSendServant));
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("servsockfd: %d\n",servsockfd);
                            //}

                            //memset(buffToSendServant,0, MAX_BUFF_READ );


                            // Reading response from the servant
                            
                           int valread = read(servsockfd, replyFromServant, DATE_LEN);


                            totalTransaction += atoi(replyFromServant);
                            //printf("totalTransaction: %d\n", totalTransaction );
                            //close(servsockfd); //yeni
                            if(valread < 0){
                                perror("Server read failed1.\n");
                                return NULL;
                            }
                            
                            close(servsockfd);

                        }


                        //printf("TOTAL: totalTransaction: %d\n",totalTransaction );
                        memset(replyFromServant, 0, sizeof(replyFromServant));
                        sprintf(replyFromServant, "%d",totalTransaction);

                        memset(writeBuffer, 0, MAX_BUFF_READ);
                        sprintf(writeBuffer,"Response received: %s, forwarded to client\n",replyFromServant);
                        //write(1, writeBuffer, strlen(writeBuffer));

                        // Sending the servant's answer to the client.
                        //printf("AAA7 question fd is->%d\n",connfd);

                        // BURADA ALINAN CEVAP FDARRAY1 e connfd reply olarak insert edilecek
                        sprintf(writingToFD1,"%d %s",connfd,replyFromServant);
                        //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                        
                        pthread_mutex_lock(&mutex_job1);
                        insertFd1(writingToFD1);
                        pthread_cond_signal(&cond_job1); 
                        pthread_mutex_unlock(&mutex_job1);
                        
                        //write(connfd, replyFromServant, sizeof(replyFromServant));


                        //close(servconnfd);
                        //close(servsockfd);

                    }
                    else{   //CITY MENTIONED, ASK WHO IS RESPONSIBLE
                        //printf("Servant searching for: %s\n", (requestList.cityName));
                        
                        int servantIndexToSend;
                        char buffToSendServant[MAX_BUFF_READ];
                        int sendflag =0;
                        int servantPortToSend= -1;
                        // Socket creating and initializing     
                        int servsockfd, servconnfd;
                        struct sockaddr_in servaddr;
                        for (i = 0; i < servantCounter; ++i)
                        {
                            //printf("word1: %s\n", (servantInfo[i].startCity));
                            //printf("word2: %s\n", (servantInfo[i].endCity));
                            //printf("word3: %s\n", ((requestList.cityName)));
                            int x=sort_words((servantInfo[i].startCity),(servantInfo[i].endCity),(requestList.cityName));

                            //printf ("%d\n",x);

                            if(x == 0){
                                servantIndexToSend = i;
                                sendflag=1; //sehirden sorumlu servant bulunur ise
                                //printf("Servant found:%d %s-%s\n",i,(servantInfo[i].startCity), (servantInfo[i].endCity));
                                break;
                            }
                        }
                        if(sendflag==0){
                           //There is no servant that responsible from the city. Sending client '0'.    
                           
                            write(connfd, "0", DATE_LEN);
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("ERRNO %s\n", strerror(errno));
                                //printf("servsockfd: %d\n",servsockfd);
                            //} 
                        }
                        else if(sendflag==1){ //Responsibled servant found.
                            sendflag=0;
                            memset(buffToSendServant, 0, MAX_BUFF_READ);
                            sprintf(buffToSendServant, "%s %s %d %d %s", "surfaceCount",
                                                                    requestList.realEstate,
                                                                    requestList.startSurfaceCount,
                                                                    requestList.endSurfaceCount ,
                                                                    requestList.cityName);
                            


                           
                            // socket create and varification
                            servsockfd = socket(AF_INET, SOCK_STREAM, 0);
                            if (servsockfd == -1) {
                                perror("Socket creation failed.\n");
                                exit(0);
                            }
                            //else
                                //printf("Socket successfully created..\n");
                            bzero(&servaddr, sizeof(servaddr));
                           
                            // assign IP, PORT
                            servantPortToSend = servantInfo[servantIndexToSend].servantPORT;

                            servaddr.sin_family = AF_INET;
                            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                            servaddr.sin_port = htons(servantPortToSend);
                            //printf("will connecto servant: %d\n",servantPortToSend);
                            
                            memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer, "Contacting servant %d\n", servantInfo[i].servantPORT);

                            // connect the servant socket to server socket
                            if ((servconnfd = connect(servsockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
                                perror("Connection with the servant failed.\n");
                                exit(0);
                            }
                            //else
                                //write(1, writeBuffer, strlen(writeBuffer));

                            
                            
                            int valwrite;
                            while(!exit_requested){
                                valwrite = write(servsockfd, buffToSendServant, sizeof(buffToSendServant));
                                if(valwrite>1){
                                    break;
                                }
                            } 

                                     
                            char replyFromServant[DATE_LEN];
                            int valread = read(servsockfd, replyFromServant, DATE_LEN);
                            if(valread < 0){
                                perror("Server read from servant failed.\n");
                                return NULL;
                            }

                            
                            /*memset(writeBuffer, 0, MAX_BUFF_READ);
                            sprintf(writeBuffer,"Response received from Servant: %s,\n",replyFromServant);
                            write(1, writeBuffer, strlen(writeBuffer));
                            */
                           // printf("AAA7 question fd is->%d\n",connfd);
                            // Sending the servant's answer to the client.
                            sprintf(writingToFD1,"%d %s",connfd,replyFromServant);
                            //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                            
                            pthread_mutex_lock(&mutex_job1);
                            insertFd1(writingToFD1);
                            pthread_cond_signal(&cond_job1); 
                            pthread_mutex_unlock(&mutex_job1);
                            


                            //valwrite = write(connfd, replyFromServant, sizeof(replyFromServant));
                            //printf("valwrite: %d\n",valwrite );
                            //if(valwrite<0){
                                //printf("ERRNO %s\n", strerror(errno));
                                //printf("servsockfd: %d\n",servsockfd);
                            //}

                            close(servsockfd);

                        }

                                        

                    }
                    
                    

                    //servanttan cevabıbekle

                    //aldıgın int sayıyı cliente gönder 
                    
                     
                    
                }  
                else{
                    
                    char buffToSendServant[MAX_BUFF_READ];
                    memset(buffToSendServant, 0, MAX_BUFF_READ);
                    strcpy((buffToSendServant), tokenreq);


                    char replytoForwardtoClient[DATE_LEN];
                    
                    memset(replytoForwardtoClient, 0, sizeof(replytoForwardtoClient));
                    sprintf(replytoForwardtoClient, "%s","This Question Is not Valid");

                    sprintf(writingToFD1,"%d %s",connfd,replytoForwardtoClient);
                        //printf("FD1e Yazdigim cevap bu->%s \n ", writingToFD1);
                        
                    pthread_mutex_lock(&mutex_job1);
                    insertFd1(writingToFD1);
                    pthread_cond_signal(&cond_job1);
                    pthread_mutex_unlock(&mutex_job1);

                    failedCount++;
                }

            }

            

            // Read the message from Client or Servant and copy it in buffer


       
            //printf("Server reading from socket: %s\n",buff );





            //printf("kaydedilen servant sayisi %d\n",servantCounter);



         
            // degil ise rehberden gerekli servanta iletisim kur, sehir belirtilmediyse hepsiyle kur
            //close(servsockfd);
                
            //close(connfd); //yeni

        }
    }
    

    exit_requested = 1;
    pthread_cond_broadcast(&cond_job1);
    pthread_cond_broadcast(&cond_job2);
    close(connfd);
    close(sockfd1);
    close(sockfd2); 
    return NULL;
   
}

int sort_words(char *word1,char *word2, char *requestedcity)
{
    char *x;
    char *words[] = {word1,word2,requestedcity};
    //printf("word1: %s\n",word1 );
    int i,j;
    for (i = 0; i<3; i++)
    {
        for (j = i + 1; j<3; j++)
        {
            if (strcmp(words[i], words[j]) < 0)
            {
                x = words[j];
                words[j] = words[i];
                words[i] = x;
            }
        }

    }
    return strcmp(words[1],requestedcity);
}

int convertDateToDay(char dateString[]){

    int day, month, year;
    char * token = strtok(dateString, "-");
    if( token != NULL ) {
        //printf( "'%s'\n", token ); 
        day = atoi(token);
        token = strtok(NULL, "-");
    }
    if( token != NULL ) {
        //printf( "'%s'\n", token ); 
        month = atoi(token);
        token = strtok(NULL, "-");
    }
    if( token != NULL ) {
        //printf( "'%s'\n", token ); 
        year = atoi(token);
        token = strtok(NULL, "-");
    }

    //printf("d: %d m: %d y:%d\n",day, month,year );

    return day + (30*(month-1)) + (12*30* (year-2000));
}



void insertFd1(char data[MAX_BUFF_FD]) {


    strcpy(fileDesc1.fdArray[++(fileDesc1.rear)],data);
    (fileDesc1.size)++;
   
}

void insertFd2(char data[MAX_BUFF_FD]) {
    //printf("INSERTFD ICINDE KI DATA = %s \n",data);
    strcpy(fileDesc2.fdArray[++(fileDesc2.rear)],data);
    //printf("INSERTFD ICINDE KI DIGERI = %s \n",fileDesc2.fdArray[(fileDesc2.rear)]);
    (fileDesc2.size)++;
   
}

void removeFd1() {
    //char str[MAX_BUFF_FD];
    //strcpy(str,fileDesc1.fdArray[(fileDesc1.front)++]);

    (fileDesc1.front)++;
   if((fileDesc1.front) == MAX_BUFF_FD) {
      (fileDesc1.front) = 0;
   }
    
   (fileDesc1.size)--;
   //return data;  
}

void removeFd2() {
   //char* data = fileDesc2.fdArray[(fileDesc2.front)++];
    (fileDesc2.front)++;
   if((fileDesc2.front) == MAX_BUFF_FD) {
      (fileDesc2.front) = 0;
   }
    
   (fileDesc2.size)--;
     
}

void initFileDescs1(int f, int r, int s){
    fileDesc1.front = f;
    fileDesc1.rear = r;
    fileDesc1.size = s;
    strcpy(fileDesc1.fdArray[0],"");
}

void initFileDescs2(int f, int r, int s){
    fileDesc2.front = f;
    fileDesc2.rear = r;
    fileDesc2.size = s;
    strcpy(fileDesc2.fdArray[0],"");
}
