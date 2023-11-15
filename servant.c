#include "servant.h"

sig_atomic_t exit_requested = 0;

void sig_handler(int sig_no)
{

    if (sig_no == SIGINT){         
        perror("SIGINT Caught!\n");
        exit_requested = 1;

    }
    if (sig_no == SIGTERM){         
        perror("SIGTERM Caught!\n");
        exit_requested = 1;

    }

}

node_t *root ;

int main(int argc, char **argv)
{
    //printf("I AM boRN %d \n",getpid());
    //int i=0;
    int fd=-1;
    char buf[BUFSIZE];
    char *fifoname = argv[0];
    char directoryPath[BUFSIZE];
    char citiesResponsible[BUFSIZE];
    char *serverIP;
    int serverPORT;
    int startingCityResponsible=0;
    int endingCityResponsible=0;
    int contactToServer=1;


    char allCityNames[CITY_NUM][NAME_BUF];
    

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sig_handler;
    sigaction(SIGINT, &sa, NULL);


    // Opens the fifo
    

    while (((fd = open(fifoname, FIFO_MODES)) == -1) && (errno == EINTR));
    
    if (fd == -1) {
        perror("Failed to open named pipe for read\n");
        return 1;
    }
    //printf("opened fd:%d %d \n",fd,getpid());
    //printf("firstpart--2--%d \n",getpid());
    
    // Reading from fifo
    //memset(buf, 0, sizeof(buf));
    int valread = read(fd, buf, BUFSIZE);  
    if(valread < 0){
        perror("Error: read from fifo.\n");
        exit_requested = 1;
    }  
    buf[valread]='\0'; //DENEMEE
    //printf("%s ---%d \n",buf,valread); //VALGRIND
   // int buflen = strlen(buf);
   // buf[buflen]


    //int bufsize = strlen(buf) + 1;

    // tokenize the buffer came from fifo
    //printf("firstpart--3--%d \n",getpid());

    if(exit_requested){
        close(fd);
        free(fifoname);
        unlink(fifoname);
        exit(0);
    }

    //free(fifoname);
    unlink(fifoname);

    //printf("bytes read: %d, readed: %s\n", bytes_read,buf);

   

    char * token = strtok(buf, " ");

    if( token != NULL ) {
        //printf( "'%s'\n", token ); 
        strcpy(directoryPath,token);
        token = strtok(NULL, " ");
        //printf( "'%s'\n", directoryPath );
    }
    if( token != NULL ) {
        //printf( "aaa '%s'\n", token ); 
        strcpy(citiesResponsible,token);
        token = strtok(NULL, " ");
        //printf( "%s\n", citiesResponsible );
    }
    if( token != NULL ) {
        //printf( "aaa '%s'\n", token ); 
        serverIP = token;
        token = strtok(NULL, " ");
        //printf( "'%s'\n", serverIP );
    }

    if( token != NULL ) {
        //printf( "aaa '%s'\n", token ); 
        serverPORT = atoi(token);
        token = strtok(NULL, " ");
        //printf( "'%d'\n", serverPORT );
    }
    if( token != NULL ) {
        //printf( "aaa '%s'\n", token ); 
        contactToServer = atoi(token);
        token = strtok(NULL, " ");
        //printf( "contactToServer '%d'\n", contactToServer );
    }

    //printf("-------------------------firstpart--5--%d \n",getpid());

    // Split the numbers of cities
    char citiesResponsibleTemp[BUFSIZE];
    strcpy(citiesResponsibleTemp,citiesResponsible);
    //citiesResponsibleTemp[BUFSIZE]='\0'; //DENEMEE
    token = strtok(citiesResponsibleTemp, "-");
    if( token != NULL ) {
        //printf( "aaa '%s'\n", token ); 
        startingCityResponsible = atoi(token) -1;
        token = strtok(NULL, "-");
    }
    if( token != NULL ) {
        //printf( "aaa '%s'\n", token ); 
        endingCityResponsible = atoi(token) -1;
        token = strtok(NULL, "-");
    }


    //printf("start: %d, end: %d\n",startingCityResponsible, endingCityResponsible );


    /*******************  Load Data Set  *******************/

    int cityCounter=0;
    

    DIR *citiesDirectory;
    struct dirent *entry;
    char entrypath[MAX_FILE_PATH];

    citiesDirectory = opendir(directoryPath);
    if (!citiesDirectory) {
        perror("Can't open directoryyyy.\n");
        return 0;
    }

    while ((entry = readdir(citiesDirectory)) != NULL) {
        //sprintf(entrypath, "./%s/%s", directoryPath, entry->d_name);
        if (entry->d_type == DT_DIR){
            if( strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
                //printf("entry->d_name: %s\n", entry->d_name);

                strcpy(allCityNames[cityCounter], entry->d_name);
                //printf("allCityNames: %s\n", allCityNames[cityCounter]);

                cityCounter++;
            }
            else{
                //printf("---: %s\n", entry->d_name);                
            }
        }
        //printf("entry->d_name: %s\n", entry->d_name);
        //printf("entry %s  \n", entrypath);      

    }
    //printf("allcityCounter: %d\n",cityCounter );
    closedir(citiesDirectory);
    
    alphabeticallyOrder(allCityNames, cityCounter); // Make it alphabetically order  
    
    


    /******************* Load the responsibled data into the structure ********************/

    citiesResponsibleNumber = endingCityResponsible - startingCityResponsible +1;


    //printf("%d\n",citiesResponsibleNumber );
    //cities = malloc(citiesResponsibleNumber * sizeof(Cities_t));
    root = make_node(0, NULL, "","",0,0,"",0);

    int dateCounter=0; 
    
    char startCityName[NAME_BUF];
    char endCityName[NAME_BUF];

    strcpy(startCityName,allCityNames[startingCityResponsible]);
    strcpy(endCityName,allCityNames[startingCityResponsible+citiesResponsibleNumber-1]);
    //printf("Flag 1\n");

    //printf("1111111111111111111111\n");
    for (cityCounter = 0; cityCounter < citiesResponsibleNumber; ++cityCounter)
    {   
        //printf("Flag 2\n");
        //printf("%d city: %s\n",cityCounter, allCityNames[startingCityResponsible + cityCounter]);
        //strcpy(cities[cityCounter].cityName, allCityNames[startingCityResponsible + cityCounter]);
        //printf("%d %s\n",cityCounter, cities[cityCounter].cityName);

        sprintf(entrypath, "./%s/%s", directoryPath, allCityNames[startingCityResponsible + cityCounter]);
        //printf("entrypath: %s\n", entrypath );
        

        citiesDirectory = opendir(entrypath);
        if (!citiesDirectory) {
            perror("Can't open directory.\n");
            return 0;
        }
        //printf("Flag 3\n");

        dateCounter = 0;
        while ((entry = readdir(citiesDirectory)) != NULL) {
            if (entry->d_type == DT_REG){   // regular file control for date files
                //printf("%d : %s\n",cityCounter, entry->d_name);
                char entryDatePath[MAX_FILE_PATH];
                char tempEntry[MAX_FILE_PATH];

                //printf("ENTRY: %s\n",entrypath );

                strcpy(tempEntry, entry->d_name);
                //tempEntry[MAX_FILE_PATH]='\0'; //DENEMEE
                int dateAsDays = convertDateToDay(tempEntry);
                //printf("date as days: %d\n", dateAsDays );
                //printf("Flag 4\n");
                //cities[cityCounter].dates[dateCounter].dateNumber = dateAsDays;
                //printf("- %d\n", cities[cityCounter].dates[dateCounter].dateNumber );

                sprintf(entryDatePath, "./%s/%s", entrypath, entry->d_name);
                //printf("entryDatePath: %s\n", entryDatePath);

                
                // Opening input files and reading and load to struct
                root = fileOperation2(entryDatePath, cityCounter, dateCounter, dateAsDays, allCityNames[startingCityResponsible + cityCounter], root);
                

                dateCounter++;
            }
        }
        closedir(citiesDirectory);
        //cities[cityCounter].fileCount = dateCounter;
        //printf("this city files: %d\n", cities[cityCounter].fileCount);
        
    }
    
    //printf("cityCounter: %d\n",cityCounter );
    char writeBuffer[MAX_LINE_BUF];
    //sprintf(writeBuffer, "Servant %d: loaded dataset, cities %s-%s\n",getpid(), cities[0].cityName, cities[cityCounter-1].cityName);

    //write(1, writeBuffer, strlen(writeBuffer));
    //printf("Dataset loaded pid:%d \n",getpid());


    //print_tree(root);

    //sleep(5);

    //node_t *findnode = find(root,200);

    //printf("findnode %d, p: %s\n",findnode->transactionID, findnode->realEstate);
    //printf("Flag--1--%d \n",getpid());
    //sleep(2);
    /**** Creating a unique port number ****/
    int servantPort = serverPORT +  startingCityResponsible+100;

    //printf("servantPort: %d\n",servantPort );

    int sockfd;
    struct sockaddr_in servaddr;
   
    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed!\n");
        freeAVLtree(root);
        //free();
        exit(0);
    }
    //else
        //printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(serverIP);
    servaddr.sin_port = htons(serverPORT);
    //printf("CONNECT TO SERVER?%d my sock is%d \n",contactToServer,sockfd);
    if(contactToServer){
        //printf("will try to connect to server %d\n",getpid());
        // connect the servant socket to server socket
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
            perror("Connection with the server failed.\n");
            //printf("couldnt connect to server sleeping for 10 seconds then exiting\n");
            //sleep(10);
            freeAVLtree(root);
            //free();
            exit(0);
        }
        //else
            //printf("connected to the server..\n");


        char buff[MAX];
        bzero(buff, sizeof(buff));
        //printf("cc %s\n",citiesResponsible );
        sprintf(buff, "servant %s %s %d %d",startCityName, endCityName, servantPort,citiesResponsibleNumber);
      
        //printf("Writing infos to server: %s\n", buff);

        write(sockfd, buff, sizeof(buff));
        //printf("valwrite: %d\n",valwrite );
        bzero(buff, sizeof(buff));
        
        //sleep(10);
        // close the socket
    }
    close(sockfd);
    //close(connfd);


    ////// Connection Waiting ///////

    int requestNum = 0;
    //pthread_t *servantThreads = malloc(sizeof(pthread_t)*(requestNum+1)); //
    

    socklen_t lenSr;
    struct sockaddr_in servantAddr, servMsg;
    int sockSrvtFd;

    

    int opti = 1;
   //sleep(100);
    // socket create and verification
    sockSrvtFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockSrvtFd == -1) {
        perror("Socket creation failed.\n");
        freeAVLtree(root);
        exit(0);
    }/*
    else
        printf("Socket successfully created..\n");*/
    //if (setsockopt(sockSrvtFd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    //    error("setsockopt(SO_REUSEADDR) failed");


    bzero(&servantAddr, sizeof(servantAddr));
    
    if (setsockopt(sockSrvtFd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opti,
                   sizeof(opti))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    bzero(&servantAddr, sizeof(servantAddr));

    // assign IP, PORT
    servantAddr.sin_family = AF_INET;
    servantAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servantAddr.sin_port = htons(servantPort);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockSrvtFd, (struct sockaddr*)&servantAddr, sizeof(servantAddr))) != 0) {
        perror("Socket bind failed!\n");
        freeAVLtree(root);
        //free();
        exit(0);
    }

    memset(writeBuffer, 0, MAX_LINE_BUF);
    //sprintf(writeBuffer, "Servant %d: listening at port %d\n", getpid(), servantPort);
    
    // Now servant is ready to listen and verification
    if ((listen(sockSrvtFd, 128)) != 0) {
        perror("Servant listen failed!\n");
        freeAVLtree(root);
        exit(0);
    }
    //else
        //write(1, writeBuffer, strlen(writeBuffer));


    while(!exit_requested){
        lenSr = sizeof(servMsg);

        // Accept the data packet from server and verification
        //sleep(100);
        int connSrvtFd = accept(sockSrvtFd, (struct sockaddr*)&servMsg, &lenSr);
        //printf("Servant accepted a request connection: %d\n", connSrvtFd); 

        if(connSrvtFd > 0 ){
            requestNum++;
            //printf("Sevant accepted a connection: %d\n ",connSrvtFd);   
        }else{
            if(!exit_requested)
            perror("Servant acception failed.\n ");
            //printf("freeingg\n");
            freeAVLtree(root);
            exit(0);
        }         
        
        


        //////// Creating threads /////
        //printf("mem %d\n %d",sizeof(pthread_t)*(requestNum+1),i);

        //servantThreads =  realloc(servantThreads,sizeof(pthread_t)*(requestNum+1));
        pthread_t servantThread;
        int rc = pthread_create(&servantThread, NULL, handleThreadJob, (void *)connSrvtFd);

        if (rc) {
            perror("Error:unable to create thread.\n");    
            fflush(stdout);        
            exit(-1);

        }

        
    }
    //printf("AAAAAAAAAA\n");
    memset(writeBuffer, 0, MAX_LINE_BUF);
    sprintf(writeBuffer, "Servant %d: termination message received, handled %d requests in total.\n",getpid(),requestNum);

/*
    for (i = 0; i < requestNum; ++i)
    {
        //printf("In Join\n");
        pthread_join(servantThreads[i],NULL);
    }

    for ( i = 0; i < requestNum+1; ++i)
    {
        free(&servantThreads[i]);
    }
    */
    //free(servantThreads);
    //close(connSrvtFd);
    //close(connfd);
    //printf("freeingg\n");
    freeAVLtree(root);
    //free();
    close(sockfd);
    close(sockSrvtFd);

    return 0;
}

void *handleThreadJob(void *fd){
    pthread_detach(pthread_self());
    char requestBuf[MAX_LINE_BUF];
    int connectionfd = (int)fd;
    Request request;
    if(!exit_requested){

        //printf("connectionfd: %d\n",(int)connectionfd );

        memset(requestBuf,0, MAX_LINE_BUF);

        // read the message from server and copy it in buffer
        //memset(requestBuf, 0, sizeof(requestBuf));
        int valread = read(connectionfd, requestBuf, MAX_LINE_BUF);
        if(valread < 0){
            perror("Server read failed.\n");
            return NULL;
        }

        //printf("read from server: %s\n",requestBuf );

    }
        

    /////// Parsing and loading the request /////
    //printf("Request Arrived %s",requestBuf);
    //requestBuf[MAX_LINE_BUF]= '\0'; //DENEMEE
    char * token = strtok(requestBuf, " ");

    char questionIdentify[20];
    
    strcpy(questionIdentify,token);
    int transactionIDD = atoi(questionIdentify);

    if((strcmp(questionIdentify,"transactionCount"))==0){//transactionCOUNT soruldu
        token = strtok(NULL, " ");
        if( token != NULL ) {
             
            strcpy(request.realEstate, token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            request.startDateAsDay = atoi(token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            //printf( "'%s'\n", token ); 
            request.endDateAsDay = atoi(token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            //printf( "'%s'\n", token ); 
            strcpy(request.city,token);
            token = strtok(NULL, " ");
        }

    
        //printf( "%s, %d, %d, %s \n", request.realEstate, request.startDateAsDay, request.endDateAsDay, request.city);
        //int i,j;
        int foundTransactions = 0;
        int searchidx=0+startingID;
        int skipthissearch=0;
        int searchcount=0;
        node_t *foundNode;
       // printf("requested city is %s first searchidx is %d\n",request.city,searchidx);
        //printf("requested realestate is %s\n",request.realEstate);

        if(strcmp(request.city , "") != 0){ // TRANSACTION COUNT CITY SORUSU GELDI

            while(searchcount<=totalnodeCount){
                searchcount++;
                skipthissearch=0;
                foundNode=find(root,searchidx);
                //printf("%d %d %d %d \n",searchcount,totalnodeCount,foundNode->transactionID,foundNode->dateAsDays);
                if(foundNode==NULL){
                    //printf("THIS ID DOESNT EXIST \n");
                    skipthissearch=1;
                    searchidx++;
                    //break;
                }
                if(skipthissearch==0){
                    //printf("foundNode->cityName= %s \n",foundNode->cityName);
                    if(strcmp(foundNode->cityName,request.city)==0){
                        
                        if( (request.startDateAsDay) <= (foundNode->dateAsDays) &&
                                    (request.endDateAsDay)   >= (foundNode->dateAsDays)){
                            if(strcmp(foundNode->realEstate,request.realEstate)==0){
                                foundTransactions++;
                            }
                        }
                        
                    }
                    searchidx++;
                }   
                
            }
        }
        else{// TRANSACTION COUNT CITY YOK SORUS GELDI USTTEKIYLE AYNI SAYILIR
            while(searchcount<=totalnodeCount){
                searchcount++;
                skipthissearch=0;
                foundNode=find(root,searchidx);
                if(foundNode==NULL){
                    //printf("THIS ID DOESNT EXIST \n");
                    skipthissearch=1;
                    searchidx++;
                    //break;
                }
                if(skipthissearch==0){                    
                    if( (request.startDateAsDay) <= (foundNode->dateAsDays) &&
                                (request.endDateAsDay)   >= (foundNode->dateAsDays)){
                        if(strcmp(foundNode->realEstate,request.realEstate)==0){
                            foundTransactions++;
                        }
                    }                                            
                    searchidx++;
                }                  
            }

        }
        //printf("SANA CEVABIM %d KANKA \n", foundTransactions);
        //////////////////////
        char replyToServer[MAX_LINE_BUF];
        memset(replyToServer, 0, MAX_LINE_BUF);
        sprintf(replyToServer, "%d", foundTransactions);
        //printf("connectionfd: %d, foundTransactions: %d, replyToServer: %s\n",connectionfd, foundTransactions, replyToServer );
        //printf("SANA CEVABIM %s KANKA \n", replyToServer);
        write(connectionfd, replyToServer, MAX_LINE_BUF);
        //printf("valwrite: %d\n",valwrite );

    //printf("Flag--18-1--%d \n",getpid());
    }else if((strcmp(questionIdentify,"topTransaction"))==0){
        //printf("topTransactionSorusuGeldi \n");
       



        token = strtok(NULL, " ");
        if( token != NULL ) {
             
            strcpy(request.realEstate, token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            request.startDateAsDay = atoi(token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            //printf( "'%s'\n", token ); 
            request.endDateAsDay = atoi(token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            //printf( "'%s'\n", token ); 
            strcpy(request.city,token);
            token = strtok(NULL, " ");
        }

    
        //printf( "%s, %d, %d, %s \n", request.realEstate, request.startDateAsDay, request.endDateAsDay, request.city);
        //int i,j;
        char topTransactionTEMP[MAX_LINE_BUF];
        char newtopTransaction[MAX_LINE_BUF];

        strcpy(topTransactionTEMP,"noanswer"); //default answer to server unless an answer was found

        int topTransactionPriceOLD=-1;

        int searchidx=0+startingID;
        int skipthissearch=0;
        int searchcount=0;

        node_t *foundNode;
        //printf("requested city is %s\n",request.city);
        //printf("requested realestate is %s\n",request.realEstate);
        if(strcmp(request.city , "") != 0){ // TopTransaction CITY SORUSU GELDI
           // printf("-flag1 \n");
            while(searchcount<=totalnodeCount){
                searchcount++;
                skipthissearch=0;
                foundNode=find(root,searchidx);
                //printf("%d %d %d %d \n",searchcount,totalnodeCount,foundNode->transactionID,foundNode->dateAsDays);
                if(foundNode==NULL){
                    //printf("THIS ID DOESNT EXIST \n");
                    skipthissearch=1;
                    searchidx++;
                    //break;
                }
                if(skipthissearch==0){
                    //printf("--flag2 \n");
                    if(strcmp(foundNode->cityName,request.city)==0){
                        //printf("----flag3 %d -- %d -- %d \n " ,request.startDateAsDay,foundNode->dateAsDays,request.endDateAsDay);
                        if( ((request.startDateAsDay) <= (foundNode->dateAsDays)) &&
                                    ((request.endDateAsDay)   >= (foundNode->dateAsDays))){
                          //  printf("-----flag4 \n");
                            if((strcmp(foundNode->realEstate,request.realEstate))==0){
                                //printf("%d>?%d \n ",foundNode->price,topTransactionPriceOLD);
                                if((foundNode->price)>topTransactionPriceOLD){
                                    memset(newtopTransaction, 0, MAX_LINE_BUF);
                                    sprintf(newtopTransaction,"%d %s %s %d %d",foundNode->transactionID,foundNode->realEstate,foundNode->street,foundNode->surface,foundNode->price);
                                   // printf("aaa1= %d %s %s %d %d \n",foundNode->transactionID,foundNode->realEstate,foundNode->street,foundNode->surface,foundNode->price);
                                    memset(topTransactionTEMP, 0, MAX_LINE_BUF);
                                    strcpy(topTransactionTEMP,newtopTransaction);
                                    topTransactionPriceOLD=foundNode->price;
                                }
                                else if((foundNode->price)==topTransactionPriceOLD){
                                    //printf("bbb1 \n");
                                    memset(newtopTransaction, 0, MAX_LINE_BUF);
                                    sprintf(newtopTransaction,"%d %s %s %d %d",foundNode->transactionID,foundNode->realEstate,foundNode->street,foundNode->surface,foundNode->price);

                                    sprintf(topTransactionTEMP,"%s %s",topTransactionTEMP,newtopTransaction);
                                }

                            }
                        }
                        
                    }
                    searchidx++;
                }   
                
            }
        }
        else{// TopTransaction CITY YOK SORUS GELDI USTTEKIYLE AYNI SAYILIR
            while(searchcount<=totalnodeCount){
                searchcount++;
                skipthissearch=0;
                foundNode=find(root,searchidx);
                if(foundNode==NULL){
                    //printf("THIS ID DOESNT EXIST \n");
                    skipthissearch=1;
                    searchidx++;
                    //break;
                }
                if(skipthissearch==0){                    
                    if( (request.startDateAsDay) <= (foundNode->dateAsDays) &&
                                (request.endDateAsDay)   >= (foundNode->dateAsDays)){
                        if(strcmp(foundNode->realEstate,request.realEstate)==0){
                                if((foundNode->price)>topTransactionPriceOLD){
                                    memset(newtopTransaction, 0, MAX_LINE_BUF);
                                    sprintf(newtopTransaction,"%d %s %s %d %d",foundNode->transactionID,foundNode->realEstate,foundNode->street,foundNode->surface,foundNode->price);
                                    
                                    memset(topTransactionTEMP, 0, MAX_LINE_BUF);
                                    strcpy(topTransactionTEMP,newtopTransaction);
                                    topTransactionPriceOLD=foundNode->price;
                                    //printf("aaa2 ·\n");
                                }
                                else if((foundNode->price)==topTransactionPriceOLD){
                                    memset(newtopTransaction, 0, MAX_LINE_BUF);
                                    sprintf(newtopTransaction,"%d %s %s %d %d",foundNode->transactionID,foundNode->realEstate,foundNode->street,foundNode->surface,foundNode->price);

                                    sprintf(topTransactionTEMP,"%s %s",topTransactionTEMP,newtopTransaction);
                                   // printf("bbb2 ·\n");
                                }

                            }
                    }                                            
                    searchidx++;
                }                  
            }

        }
        
        //////////////////////
        char replyToServer[MAX_LINE_BUF];
        memset(replyToServer, 0, MAX_LINE_BUF);
        strcpy(replyToServer,topTransactionTEMP);
        //printf("connectionfd: %d, foundTransactions: %d, replyToServer: %s\n",connectionfd, foundTransactions, replyToServer );
        //printf("SANA CEVABIM %s KANKA \n", replyToServer);
        write(connectionfd, replyToServer, MAX_LINE_BUF);
        //printf("valwrite: %d\n",valwrite );

    //printf("Flag--18-1--%d \n",getpid());


    }
    else if((strcmp(questionIdentify,"surfaceCount"))==0){
        //printf("hebele=> %s \n",token);
        token = strtok(NULL, " ");
        if( token != NULL ) {
            //printf("estate=> %s \n",token); 
            strcpy(request.realEstate, token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
           // printf("startSurfaceCount=> %s \n",token);
            request.startSurfaceCount = atoi(token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            //printf("endSurfaceCount=> %s \n",token);
            request.endSurfaceCount = atoi(token);
            token = strtok(NULL, " ");
        }
        if( token != NULL ) {
            //printf("city=> %s \n",token);
            strcpy(request.city,token);
            token = strtok(NULL, " ");
        }

    
        //printf( "%s, %d, %d, %s \n", request.realEstate, request.startDateAsDay, request.endDateAsDay, request.city);
        //int i,j;
        int foundTransactions = 0;
        int searchidx=0+startingID;;
        int skipthissearch=0;
        int searchcount=0;
        node_t *foundNode;
        //printf("requested city is %s\n",request.city);
        //printf("requested realestate is %s\n",request.realEstate);
        if(strcmp(request.city , "") != 0){ // TRANSACTION COUNT CITY SORUSU GELDI
            //printf("SearchCount City Var sorusu Geldi\n");
            while(searchcount<=totalnodeCount){
                searchcount++;
                skipthissearch=0;
                foundNode=find(root,searchidx);
                //printf("%d %d %d %d \n",searchcount,totalnodeCount,foundNode->transactionID,foundNode->dateAsDays);
                if(foundNode==NULL){
                    //printf("THIS ID DOESNT EXIST \n");
                    skipthissearch=1;
                    searchidx++;
                    //break;
                }
                if(skipthissearch==0){
                    if(strcmp(foundNode->cityName,request.city)==0){
                       // printf("----flag3 %d -- %d -- %d \n " ,request.startSurfaceCount,foundNode->surface,request.endSurfaceCount);
                        if( (request.startSurfaceCount) <= (foundNode->surface) &&
                                    (request.endSurfaceCount)   >= (foundNode->surface)){
                            if(strcmp(foundNode->realEstate,request.realEstate)==0){
                                foundTransactions++;
                            }
                        }
                        
                    }
                    searchidx++;
                }   
                
            }
        }
        else{// Search COUNT CITY YOK SORUS GELDI USTTEKIYLE AYNI SAYILIR
           //printf("SearchCount City Yok sorusu\n");
            while(searchcount<=totalnodeCount){
                searchcount++;
                skipthissearch=0;
                foundNode=find(root,searchidx);
                if(foundNode==NULL){
                    //printf("THIS ID DOESNT EXIST \n");
                    skipthissearch=1;
                    searchidx++;
                    //break;
                }
                if(skipthissearch==0){                    
                    if( (request.startSurfaceCount) <= (foundNode->surface) &&
                                (request.endSurfaceCount)   >= (foundNode->surface)){
                        if(strcmp(foundNode->realEstate,request.realEstate)==0){
                            foundTransactions++;
                        }
                    }                                            
                    searchidx++;
                }                  
            }

        }
        //printf("SANA CEVABIM %d KANKA \n", foundTransactions);
        //////////////////////
        char replyToServer[MAX_LINE_BUF];
        memset(replyToServer, 0, MAX_LINE_BUF);
        sprintf(replyToServer, "%d", foundTransactions);
        //printf("connectionfd: %d, foundTransactions: %d, replyToServer: %s\n",connectionfd, foundTransactions, replyToServer );
       // printf("SANA CEVABIM %s KANKA \n", replyToServer);
        write(connectionfd, replyToServer, MAX_LINE_BUF);
        //printf("valwrite: %d\n",valwrite );

    //printf("Flag--18-1--%d \n",getpid());
    }
    else if(transactionIDD!=0){ //ID SORULDU
        char replyToServer[MAX_LINE_BUF];
        memset(replyToServer, 0, MAX_LINE_BUF);
        int foundanswer=0;
        node_t *foundNode = find(root,transactionIDD);

        if(foundNode==NULL && (foundanswer==0)){
            //printf("THIS ID DOESNT EXIST \n");
            strcpy(replyToServer,"no");
            //break;
        }
        else{
            foundanswer=1;
            sprintf(replyToServer,"%d %s %s %d %d",foundNode->transactionID,foundNode->realEstate,foundNode->street,foundNode->surface,foundNode->price);
        }
        //sprintf(replyToServer, "%s ", );
        //printf("connectionfd: %d, foundTransactions: %d, replyToServer: %s\n",connectionfd, foundTransactions, replyToServer );
       // printf("SANA CEVABIM %s KANKA \n", replyToServer);
        write(connectionfd, replyToServer, MAX_LINE_BUF);
        //printf("valwrite: %d\n",valwrite );
        //printf("Flag--18-2--%d \n",getpid());
        /////?????
    }
    
    close(connectionfd);

    return NULL;

}

int searchEstate(char *searchTransactions, char *estate){
    int len1 = strlen(searchTransactions);
    int len2 = strlen(estate);

    //printf("%s \nlen:%d\n",searchTransactions ,len1);
    //printf("%s len:%d\n",estate ,len2);

    
    int i,j;
    int k = 0;
    int l = 0;
    for (i = 0; i < len1;)
    {
        j = 0;
        k = 0;
        while ((searchTransactions[i] == estate[j]))
        {
            k++;
            i++;
            j++;
        }
        if (k == len2)
        {
            l++;                                   
            k = 0;
        }
        else
            i++;
    } 
    //printf("buldugum sayi%d\n",l );
    return l;
}


void alphabeticallyOrder(char allCityNames[CITY_NUM][NAME_BUF], int cityCounter){
    char tempCities[CITY_NUM];

    int i,j;
    for(i = 0; i<cityCounter; i++){
        for(j=i+1;j<cityCounter;j++){
            if(strcmp(allCityNames[i],allCityNames[j])>0){
                strcpy(tempCities,allCityNames[i]);
                strcpy(allCityNames[i],allCityNames[j]);
                strcpy(allCityNames[j],tempCities);
                //printf("%s\n",cities[j]);
            }
        }
    }
}

int convertDateToDay(char *dateString){

    int day, month, year;
    //dateString[]
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

// Records to nested structure for easily finding by date and city



//Records to AVL Tree for transaction id searching
node_t *fileOperation2(char *filePath, int cityCounter, int dateCounter,int dateAsDays, char cityName[NAME_BUF],node_t *root ){ 

    int fd, bytesread;
    int i =0;
    char inputFileStr[MAX_LINE_BUF];
    char inputFileStrT[MAX_LINE_BUF];
    char chr;
    int lineLen = 0;
    //printf("IN FILEOP2\n");
    if((fd = openat(AT_FDCWD, filePath, O_RDONLY)) == -1){
        perror("Failed to open file\n");
        return 0;
    }
    //printf("opened fd:%d\n",fd);
    //memset(&chr, 0, 1);
    while(((bytesread = read(fd, &chr, 1)) > 0)){
        if(chr == '\n'){

            //printf("i: %d\n",i );
            inputFileStr[i] = '\0';
            //printf("inBuffer: %s\n",inBuffer );

            if(i != 0){
                
                //printf("lineLen: %d, %s\n",lineLen , inputFileStr);
                strcpy(inputFileStrT, inputFileStr);
                root = loadAVLTree(inputFileStrT, dateAsDays, cityName, root );
                lineLen = 0;
                
                
            }
            memset(inputFileStr, 0, MAX_LINE_BUF);
            i = 0;
            continue;
        }

        inputFileStr[i]= chr;
        i++;
        lineLen++;

    }

    //printf("---\n%s\n---",inputFileStr );


    //strcpy(cities[cityCounter].dates[dateCounter].transactions, inputFileStr);

    //loadAVLTree(inputFileStr, dateAsDays);

    //printf("---\n%s\n---",cities[cityCounter].dates[dateCounter].transactions );

    //sleep(5);
    //node_t *findnode = find(root,50);
    //printf("FFFFFindnode %d,\n p: %s\n",findnode->transactionID, findnode->realEstate);
    //

    //sleep(5);



    close(fd);

    //sleep(5);

    return root;
}

node_t *loadAVLTree(char inputFileStr[MAX_LINE_BUF], int dateAsDays, char cityName[NAME_BUF], node_t *root ){
    char inputFileStrTemp[MAX_LINE_BUF];
    //int dateAsNumber;
    int transactionID;
    char realEstate[NAME_BUF];
    char street[NAME_BUF];
    int surface;
    int price;

    strcpy(inputFileStrTemp, inputFileStr);
    //printf("Tmp: %s\n",inputFileStr );
    //sleep(5);
    //inputFileStrTemp[MAX_LINE_BUF]='\0'; //DENEMEE
    char * token = strtok(inputFileStrTemp, " ");
    if( token != NULL ) {
        //printf( " token 1 %s\n", token ); 
        transactionID = atoi(token);
        if(transactionID<startingID){
            startingID=transactionID;
        }
        token = strtok(NULL, " ");
    }
    if( token != NULL ) {
        //printf( "token 2 %s\n", token ); 
        strcpy(realEstate,token);
        token = strtok(NULL, " ");
    }
    if( token != NULL ) {
        //printf( "token 3 %s\n", token ); 
        strcpy(street,token);
        token = strtok(NULL, " ");

    }
    if( token != NULL ) {
        //printf( "token 4 %s\n", token ); 
        surface=atoi(token);
        token = strtok(NULL, " ");

    }
    //printf( "token 5 %s\n", token ); 
    
    price=atoi(token);

    //sleep(1);

    root = insert(root, transactionID, realEstate, street, surface, price, cityName, dateAsDays);
    
    //printf("id: %d, realEstate: %s\n", root->transactionID, root->realEstate );
    //print_tree(root);
    //sleep(2);

    return root;

}


int max(int a, int b) { return a > b ? a : b; }

node_t *find(node_t *root, int val)
{
    if (root == NULL) return NULL;
    if (val < root->transactionID)
        return find(root->left, val);
    else if (val > root->transactionID)
        return find(root->right, val);
    else
        return root;
}

int height(node_t *root)
{
    return root ? root->height : 0;
}

void adjust_height(node_t *root)
{
    root->height = 1 + max(height(root->left), height(root->right));
}

/* We can assume node->left is non-null due to how this is called */
node_t *rotate_right(node_t *root)
{
    /* Fix pointers */
    node_t *new_root = root->left;
    if (root->parent)
    {
        if (root->parent->left == root) root->parent->left = new_root;
        else root->parent->right = new_root;
    }
    new_root->parent = root->parent;
    root->parent = new_root;
    root->left = new_root->right;
    if (root->left) root->left->parent = root;
    new_root->right = root;

    /* Fix heights; root and new_root may be wrong. Do bottom-up */
    adjust_height(root);
    adjust_height(new_root);
    return new_root;
}

/* We can assume node->right is non-null due to how this is called */
node_t *rotate_left(node_t *root)
{
    /* Fix pointers */
    node_t *new_root = root->right;
    if (root->parent)
    {
        if (root->parent->right == root) root->parent->right = new_root;
        else root->parent->left = new_root;
    }
    new_root->parent = root->parent;
    root->parent = new_root;
    root->right = new_root->left;
    if (root->right) root->right->parent = root;
    new_root->left = root;

    /* Fix heights; root and new_root may be wrong */
    adjust_height(root);
    adjust_height(new_root);
    return new_root;
}

node_t *make_node(int id, node_t *parent, char realEstate[NAME_BUF], char street[NAME_BUF], int surface, int price,char cityName[NAME_BUF],int dateAsDays)
{

    node_t *n = malloc(sizeof(node_t));
    //printf("1 id: %d, realEstate: %s\n",id, realEstate );
    n->transactionID = id;
    strcpy(n->realEstate, realEstate);
    //printf("2 id: %d, realEstate: %s\n", n->transactionID, n->realEstate );
    //printf("%s\n", n->realEstate);
    strcpy(n->street, street);
    n->surface= surface;
    n->price= price;
    strcpy(n->cityName,cityName );
    n->dateAsDays = dateAsDays;
    n->parent = parent;
    n->height = 1;
    n->left = NULL;
    n->right = NULL;

    return n;
}

node_t *balance(node_t *root)
{
    if (height(root->left) - height(root->right) > 1)
    {
        if (height(root->left->left) > height(root->left->right))
        {
            root = rotate_right(root);
        }
        else
        {
            rotate_left(root->left);
            root = rotate_right(root);
        }
    }
    else if (height(root->right) - height(root->left) > 1)
    {
        if (height(root->right->right) > height(root->right->left))
        {
            root = rotate_left(root);
        }
        else
        {
            rotate_right(root->right);
            root = rotate_left(root);
        }
    }
    return root;
}

node_t *insert(node_t *root, int id, char realEstate[NAME_BUF], char street[NAME_BUF], int surface, int price,char cityName[NAME_BUF],int dateAsDays)
{
    totalnodeCount++;
    node_t *current = root;
    //printf("string to be inserted: %s\n",realEstate );
    //strcpy(current->string,string);
    while (current->transactionID != id)
    {
        if (id < current->transactionID)
        {
            if (current->left) current = current->left;
            else
            {
                current->left = make_node(id, current, realEstate, street, surface, price, cityName,dateAsDays);
                current = current->left;
            }
        }
        else if (id > current->transactionID)
        {
            if (current->right) current = current->right;
            else
            {
                current->right = make_node(id, current, realEstate, street, surface, price, cityName,dateAsDays);
                current = current->right;
            }
        }
        else return root; 
    }
    
    do
    {
        current  = current->parent;
        adjust_height(current);
        current = balance(current);
    } while (current->parent);
    
    

    return current;
}


void freeAVLtree(node_t *p)
{
    if(p==NULL)
        return;

    freeAVLtree(p->right);
    freeAVLtree(p->left);
    //printf("freed\n");
    free(p);
}


