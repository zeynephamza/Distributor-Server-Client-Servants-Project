
#include "distributor.h"

sig_atomic_t exit_requested = 0;

void sig_handler(int sig_no)
{

    if (sig_no == SIGINT){         
        //perror("SIGINT Caught!\n");
        exit_requested = 1;
        
    }
    
}

int main(int argc, char *argv[]) 
{
	int opt;
	char *directoryPath;
	int numberOfServants;
	char *IP;
	int PORT;


    ///// Writing the output to log file
    int byteswritten;
    char writeBuffer[WRITE_TO_LOG_BUFFER];



    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sig_handler;
    sigaction(SIGINT, &sa, NULL);
    


	if (argc!=9){
        perror( "Usage ./distributor -d directoryPath -s numberOfServants -r IP -p PORT\n");
        
        return 0;       
    }

    while((opt = getopt(argc, argv, ":d:s:r:p:")) != -1) 
    {
    	switch(opt) 
        {             
            case 'd': 
                directoryPath = optarg;
                //printf("directoryPath: %s\n", directoryPath); 
                break; 
            case 's':  
                numberOfServants = atoi(optarg);
                //printf("numberOfServants: %d\n", numberOfServants); 
                break; 
            case 'r':  
                IP = optarg;
                //printf("IP: %s\n", IP); 
                break; 
            case 'p':  
                PORT = atoi(optarg);
                //printf("PORT: %d\n", PORT); 
                break; 
            case '?':                 
                perror("Unknown option. Usage ./distributor -d directoryPath -s numberOfServants -r IP -p PORT\n");
                return 0;  
            default:
                perror("Usage ./distributor -d directoryPath -s numberOfServants -r IP -p PORT\n");
                return 0; 
        }
    }

    // Create and open log file

    int logfd;

    if((logfd = open(logfilename, FILE_MODES, USER_PERMS)) == -1){
        perror("Failed to open log file\n");
        return 1;
    }
    //printf("opened fd:%d\n",logfd);
    // Finds number of all directories with city names
	DIR * dirp;
	struct dirent * entry;
	int directoryCount = 0;

	dirp = opendir(directoryPath); 
	if (!directoryPath) {
    	perror("Can't open directory.\n");
    	return 0;
  	}

	while ((entry = readdir(dirp)) != NULL) {
	    if (entry->d_type == DT_DIR){
  			if( strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
  				directoryCount++;
  			}

  		}
	}
	closedir(dirp);

	//printf("directoryCount: %d\n", directoryCount);



	int divisor = directoryCount / numberOfServants; // will be shared precisely

	int remainder = directoryCount % numberOfServants; // 1 will be added to num of 'remainder' servants 

	//printf("divisor: %d, remainder: %d\n",divisor, remainder ); 




    //Writing to the log file
    sprintf(writeBuffer, "Distributing the data of %d cities to each of the %d servants, total %d cities. \n",divisor,numberOfServants,directoryCount );
    if(((byteswritten = write(logfd, writeBuffer, strlen(writeBuffer))) == -1) || (errno == EINTR)){
        //perror("Can't write to log file!\n");
        //return 1;
    }
    memset(writeBuffer, 0, WRITE_TO_LOG_BUFFER);


    int i = 0;
    pid_t pids[numberOfServants];
    int startCityArray[numberOfServants];
    int endCityArray[numberOfServants];

    int start=0, end=0;

    int rem = remainder;
    int div = divisor;
    int flagOfRespawn = 0;
    //printf("AAAAAAAAAAAAAAAAAAAAA - %d ",numberOfServants);
	for (i = 0; i < numberOfServants; ++i)
    {   

        if(!exit_requested){

            if(!flagOfRespawn){
                //printf("Calculating new cities\n");
                flagOfRespawn = 0;

                // Calculating responsible cities:
                start = end +1;

                if(rem > 0){
                    end = start + (div-1) + 1;
                    rem --;
                }else{
                    end = start + (div - 1);
                }
                

            }
            
            startCityArray[i]=start;
            endCityArray[i]=end;

            //printf("cities responsible: %d-%d\n",start,end );

            pid_t childPid = fork();
            if(childPid < 0){

                perror("Fork failed!\n");
                return 0;
            }

            if(childPid == 0) {
            /* child process. */

                int rval;
                ssize_t strsize;
                int fd;
                char fifoname[FIFO_NAME_BUFFER];
                char fifomessage[DATA_LEN_BUFFER];

                //printf("%d. Child: %d, %d\n",i,getpid(), getppid() );
     

                // Create a fifo name for this child            

                sprintf(fifoname,"fifo%d",i+1);

                char *args[]={(&fifoname[0]),NULL};




                rval= sprintf(fifomessage,"%s %d-%d %s %d %d",directoryPath, start,end, IP, PORT,1);
                if (rval < 0) {
                    perror("Failed to make the string:\n");
                    return 1;
                }

                //printf("fifoname: %s, fifomessage: %s\n",fifoname, fifomessage );

                strsize = strlen(fifomessage);
                
                // Fifo file creating
                mkfifo(fifoname, 0666);

                int fd_fifo = open(fifoname, FIFO_MODES);
                if (fd_fifo ==-1)
                {
                    //printf("Cant open fifofile: % d\n", errno);                 
                }
                //printf("opened fd:%d\n",fd_fifo);
                while (((fd = open(fifoname, FIFO_MODES)) == -1) && (errno == EINTR));
                if (fd == -1) {
                    perror("Failed to open named pipe\n");
                    return 1;
                }
                //printf("opened fd:%d\n",fd);
                // Writing the fifo 
                //size_t left_to_write = strsize;
                size_t written = write(fd, fifomessage, strsize);
                if (written == -1)
                    return -1;
                    

                //printf("written bytes: %d\n",written );

                close(fd);

                //memset(fifoname, 0, FIFO_NAME_BUFFER); // fifoname i işin bitince 0 la
                memset(fifomessage, 0, DATA_LEN_BUFFER); // fifomessage i işin bitince 0 la

                //printf("args: %s\n",args[0]);
                //sleep(1000);
                execv("./servant", args);

               // If execv returns,when it failed.
                perror("execv failed.\n");
                unlink(fifoname);
                exit(0);
            }
            else{
                // Parent
                pids[i]=childPid;
                
                //printf("created a child waiting 4 seconds before the next one \n");
                //sleep(4);

                
            }
        }
        else{
            // Kill signal came, exiting
            for (i = 0; i < numberOfServants; ++i)
            {
                kill(pids[i],SIGTERM);
            }
            //free(pids);
            close(logfd);
            exit(0);
        }



    }
    
    pid_t wpid;
    int status=0;
    pid_t newchildpid;
    while ((wpid = wait(&status)) > 0){
        if(exit_requested){
            break;
        }
        //printf("A servant with pid%d=died\n",wpid);
        int xx=0;
        for(xx = 0 ; xx < numberOfServants; ++xx ){
            if(pids[xx]==wpid){
                break;
            }
        }

        //recalculating start and end cities for new servants
             
        if((newchildpid=fork())==0){


                
            int rval;
            ssize_t strsize;
            int fd;
            char fifoname[FIFO_NAME_BUFFER];
            char fifomessage[DATA_LEN_BUFFER];

            //printf("%d. Child: %d, %d\n",i,getpid(), getppid() );
 

            // Create a fifo name for this child            

            sprintf(fifoname,"fifo%d",xx+1);
            //printf("xx=%d \n",xx);
            char *args[]={(&fifoname[0]),NULL};




            rval= sprintf(fifomessage,"%s %d-%d %s %d %d",directoryPath, startCityArray[xx],endCityArray[xx], IP, PORT,0);
            if (rval < 0) {
                perror("Failed to make the string:\n");
                return 1;
            }

            //printf("fifoname: %s, fifomessage: %s\n",fifoname, fifomessage );

            strsize = strlen(fifomessage) + 1;
            
            // Fifo file creating
            //unlink(fifoname);
            //sleep(10);
            mkfifo(fifoname, 0666);

            int fd_fifo = open(fifoname, FIFO_MODES);
            if (fd_fifo ==-1)
            {
                //printf("Cant open fifofile: % d\n", errno);                 
            }

            while (((fd = open(fifoname, FIFO_MODES)) == -1) && (errno == EINTR));
            if (fd == -1) {
                perror("Failed to open named pipe\n");
                return 1;
            }

            // Writing the fifo 
            //size_t left_to_write = strsize;
            size_t written = write(fd, fifomessage, strsize);
            if (written == -1)
                return -1;
                

            //printf("written bytes: %d\n",written );

            close(fd);

            //memset(fifoname, 0, FIFO_NAME_BUFFER); // fifoname i işin bitince 0 la
            memset(fifomessage, 0, DATA_LEN_BUFFER); // fifomessage i işin bitince 0 la

            //printf("args: %s\n",args[0]);

            execv("./servant", args);

           // If execv returns,when it failed.
            perror("execv failed.\n");
            exit(0);
        }
        else{

            sprintf(writeBuffer, "Servant %d has ended, I’m respawning it\n",wpid);
            if(((byteswritten = write(logfd, writeBuffer, strlen(writeBuffer))) == -1) || (errno == EINTR)){
                //perror("Can't write to log file!\n");
                //return 1;
            }
            memset(writeBuffer, 0, WRITE_TO_LOG_BUFFER);
            //printf("A servant with pid%d has died. Respawned it with pid  %d\n",wpid,newchildpid);
            pids[xx]=newchildpid;
        }
    } 

    sprintf(writeBuffer, "SIGINT received, forwarding to servants.\n.");
    if(((byteswritten = write(logfd, writeBuffer, strlen(writeBuffer))) == -1) || (errno == EINTR)){
        //perror("Can't write to log file!\n");
        //return 1;
    }
    memset(writeBuffer, 0, WRITE_TO_LOG_BUFFER);

    for (i = 0; i < numberOfServants; ++i)
    {
        kill(pids[i],SIGINT);
    }

    for (i = 0; i < numberOfServants; ++i)
    {
        int status;
        //printf("pids[i]: %d\n",pids[i] );
        waitpid(pids[i], &status, 0);
    }

    sprintf(writeBuffer, "I'm terminating. Goodbye.\n");
    if(((byteswritten = write(logfd, writeBuffer, strlen(writeBuffer))) == -1) || (errno == EINTR)){
        //perror("Can't write to log file!\n");
        //return 1;
    }
    memset(writeBuffer, 0, WRITE_TO_LOG_BUFFER);

    close(logfd);




    return 0;

}


