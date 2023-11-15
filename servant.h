#ifndef TCP_SERVANT_H
#define TCP_SERVANT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h> 

#define NAME_BUF 50 //25
#define NUM_OF_DATES 5000 //50->5000 yapilmali
#define MAX_FILE_PATH 500 //500 orj de calisti
#define MAX_FILE_BUF 5000 //5000 orj calisti
#define MAX_LINE_BUF 5000 //500orj->5000 yapilmali
#define CITY_NUM 500 //500 orj calisti
#define MAX 80 //80 orj calisti
#define FIFO_MODES O_RDWR
#define BUFSIZE 1000

/*struct Date_t
{
	int dateNumber;  // Date as calculated day number 
	char transactions[MAX_FILE_BUF];

};*/

/*typedef struct
{	
	char cityName[NAME_BUF];
	int resCityNum;
	struct Date_t dates[NUM_OF_DATES];
	int fileCount;


}Cities_t;*/


typedef struct 
{

	char realEstate[NAME_BUF];
	char city[NAME_BUF];
	int startDateAsDay;
	int endDateAsDay;
	int startSurfaceCount;
	int endSurfaceCount;

}Request;


typedef struct node
{
	int val;
	char cityName[NAME_BUF];
	int dateAsDays;
	int transactionID;
    char realEstate[NAME_BUF];
    char street[NAME_BUF];
    int surface;
    int price;
	struct node *left;
	struct node *right;
	struct node *parent;
	int height;
} node_t;



//Cities_t *cities;
int citiesResponsibleNumber=0;
int totalnodeCount=0;
int startingID=99999999;


void alphabeticallyOrder(char cities[CITY_NUM][NAME_BUF], int cityCounter);
int convertDateToDay(char *dateString);
//int fileOperation1(char *filePath, Cities_t cities[], int cityCounter, int dateCounter);
node_t * fileOperation2(char *filePath, int cityCounter, int dateCounter, int dateAsDays, char cityName[NAME_BUF], node_t *root );
void loadRequest(char requestBuf[MAX_LINE_BUF], Request request);
void *handleThreadJob(void *fd);
int searchEstate(char *searchTransactions, char *estate);
node_t * loadAVLTree(char inputFileStr[MAX_LINE_BUF], int dateAsDays,char cityName[NAME_BUF], node_t *root );
int max(int a, int b);
node_t *find(node_t *root, int val);
int height(node_t *root);
void adjust_height(node_t *root);
node_t *rotate_right(node_t *root);
node_t *rotate_left(node_t *root);
node_t *make_node(int id, node_t *parent, char realEstate[NAME_BUF], char street[NAME_BUF], int surface, int price,char cityName[NAME_BUF], int dateAsDays);
node_t *balance(node_t *root);
node_t *insert(node_t *root, int id, char realEstate[NAME_BUF], char street[NAME_BUF], int surface, int price,char cityName[NAME_BUF],int dateAsDays);
void freeAVLtree(node_t *p);


#endif

/*

Cities_t[Klasör Sayısı].Dates[10].Transactions[LineSayisi].type


for i= x->y 
	if Cities_t[i] == İstenen Sehirn numarası
		for j = 1:10
			if Cities_t[i].Dates[j].dateNumber =? istenen gün araligi
				for k=0 -> Cities_t[i].Dates[j].Linecount
					Cities_t[i].Dates[j].TransactionType[k];
*/
