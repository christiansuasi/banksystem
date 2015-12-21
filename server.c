/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>

//shm includes
#include <sys/shm.h>
#include <sys/ipc.h>

#include <netinet/in.h>
#include <stdbool.h>
#include "server.h"

//multithread includes
#include <pthread.h>

//shared memory global variables
Account *shm, *s;
Account list[21];

void printAccounts(Account list[]) {
	int i;
	for (i = 0; i < 21; i++) {
		if (strcmp("NULL",list[i].name)==0) break; //might be 0 might null we dont know yet
        	printf("-------------\n");
        	printf("Account Name: %sAccount Balance: $%.2f\n",list[i].name, list[i].balance);
        	if (list[i].session == true){
			printf("IN SESSION\n");
		}
        printf("-------------\n");
	}
}

Account makeFiller(){
	Account *fill = malloc(sizeof(Account));
	strcpy(fill->name, "NULL");
	fill->balance = 0;
	fill->session = false;
	return *fill;
}


Account createAccount(char *clientName) {
	Account *newAcc = malloc(sizeof(Account));
	strcpy(newAcc->name, clientName);
	newAcc->balance = 0;
	newAcc->session = false;
	return *newAcc;
}

int credit(float add, Account list[], int curr) {
	if (add < 0) return 0;
	list[curr].balance = list[curr].balance + add;
	return 1;
}

int debit(float sub, Account list[], int curr) {
	if (sub < 0) return 0;
	if (list[curr].balance < sub) return 0;
	list[curr].balance = list[curr].balance - sub;
	return 1;
}

float balance(int curr, Account list[]) {
	//printf("Your current balance is: $%.2f\n",curr->balance);
	return list[curr].balance;
}

int openAcc(char *name, Account list[]){
	//name is too long  
	if (strlen(name) > 100) return -1;
	
	//name is nothing, so do nothing
	if (strcmp(name, "")==0) return -2;
    	
	//traverse list
	int i;
	for (i = 0; i < 20; i++) {
        	
		//if name is already in list return error msg
		if (strcmp(list[i].name, name)==0){
            		return -3;
		}	
       
		//creates account 
       		if (strcmp(list[i].name, "NULL")==0) {	
			//put parameter name and replaces "NULL" string filler
			strcpy(list[i].name, name);
			memset(name, 0 , strlen(name));
			printAccounts(list);
			return 1;
		}	
	}
	return 0;
}

int start(char *name, Account list[]) {
	int i;
	for (i = 0; i < 21; i++) {
		if (strcmp(name,list[i].name)==0) {
		return i;
		}
	}
	return -1;
}

void error(const char *msg){
	perror(msg);
	exit(1);
}


//print account list every 20 seconds function
void *printEvery20(void *arg){
	int timer =0;	
	while(1){
		//print every 20 seconds;	
		sleep(20);
		timer+=20;
		printf("PRINTING ACCOUNTS\nTIME ELAPSED: %d SECONDS\nREFRESHING IN 20 SECONDS\n",timer);
		printAccounts(list);
	}
	return NULL;
}


//MAIN PROGRAM
int main(int argc, char *argv[]){
	
	//start of shm
	int shmid;
	key_t key;
	
	//values to be put int shm

	//shm segment, whatever key value
	key = 4444;

	//create the segment 
	//1st parameter matches
	//2nd parametr is allocating the size for segment
	
	if ((shmid = shmget(key, 21*sizeof(Account), IPC_CREAT | 0666)) < 0){
		perror("shmid perror shmget");
		exit(1);
	}

	//attach segment to our data space
	if ((shm = shmat(shmid, NULL, 0)) == (Account *) -1){
		perror("perror shmat");
		exit(1);
	}

	//put some things into memory for other processes to find
	s = shm;


	shm = list; 
	//end of shm


	//start of sockets 
	
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	char check[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
    
	//exit if port number is not provided
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
    	}
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	//set port number as first shell arg	
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
    
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		error("ERROR on binding");
	}
	
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	
	if (newsockfd < 0) error("ERROR on accept");
	//clear buffer
	bzero(buffer,256);
   
	//end of socket 
	
	


	//initialize accounts list
	int currAcc = -1;
	int i;
	for (i = 0; i < 21; i++) {
		list[i] = makeFiller();
	}

	//MULTITHREADING
	pthread_t printEvery20_thread; //thread identifier

	if(pthread_create(&printEvery20_thread, NULL, printEvery20, &list)){
		fprintf(stderr, "error creating thread\n");
		return 1;
	}
	
	while (strcmp("exit\n", check)!=0) {
		n = read(newsockfd,buffer,255);
		if (n < 0) error("ERROR reading from socket");
		//n = write(newsockfd,"I got your message",18);
		if (n < 0) error("ERROR writing to socket");

		char open[6];
		char start_instruction[7];
		char exit[5];
		strncpy(open, buffer, 5);
		strncpy(start_instruction, buffer, 6);
		strncpy(exit, buffer, 5);




		//open
		if (strcmp(open, "open ")==0) {
			char acName[100];
			strncpy(acName,buffer + 5, 255 - 5);
			int msg = openAcc(acName,list);
			memset(&acName, 0, sizeof(acName));
			
			//error messages	
			if (msg == -2) n = write(newsockfd,"Invalid Name!\n",14);
			if (msg == -1) n = write(newsockfd,"Name too long!\n",15);
			if (msg == -3) n = write(newsockfd,"There is an Account with that name!\n",36);
			if (msg == 1) n = write(newsockfd,"Account Created!\n",17);
			if (msg == 0) n = write(newsockfd,"ERROR, NOTHING HAPPENED\n", 24);
		
			//clear buffer
			memset(buffer, 0 , sizeof(buffer));
			
        	}


		//START AND ONLY DEBIT, CREDIT, AND BALANCE SHOULD WORK WITHIN START        
		//start
		else if (strcmp("start ",start_instruction)==0) {
			start_instruction[0] = '\0';
			char find[255];
			strncpy(find, buffer + 6, 255 - 6);
	   		//printf("the name %s is %i charaters long\n",find,strlen(find));
	   		int finish = 0;
			if (start(find, list) == -1) {
				finish = 1;
				n = write(newsockfd,"Account does not exist!\n",25);
			}

			else{
				currAcc = start(find,list); 
				n = write(newsockfd,"Session Started!\n",17);
            		} 
			
			while (finish == 0) {
				n = read(newsockfd,buffer,255);
			
				//function declarations for messages
				char credit_instruction[8];
				char debit_instruction[7];
				char balance_instruction[9];
				char finish_instruction[7];
				char list_instruction[5];
				strncpy(credit_instruction, buffer, 7);
				strncpy(debit_instruction, buffer, 6);
				strncpy(balance_instruction, buffer, 8);
				strncpy(finish_instruction, buffer, 6);	
				strncpy(list_instruction, buffer,5);

				
				//credit	
				if (strcmp("credit ",credit_instruction)==0) {
					char amount[32];
		   			strncpy(amount, buffer + 6, 255 - 5);
					float add = atof(amount);
					int msg = credit(add, list, currAcc);
					
					if (msg == 0) n = write(newsockfd,"Invalid Amount!\n",16);
		    			if (msg == 1) n = write(newsockfd,"Account Credited!\n",19);
					
					memset(buffer, 0 , sizeof(buffer));
				}
		
				//debit
				else if (strcmp("debit ",debit_instruction)==0) {
					char amount[32];
					strncpy(amount, buffer + 5, 255 - 4);
					float sub = atof(amount);
					int msg = debit(sub, list, currAcc);
					
					if (msg == 0) n = write(newsockfd,"Invalid Amount!\n",16);
					if (msg == 1) n = write(newsockfd,"Account Debited!\n",17);
					memset(buffer, 0 , sizeof(buffer)); 
				}

				//balance
				else if (strcmp("balance\n",balance_instruction)==0) {
					char float_string[50];
					char intro_balance_string[50];
				
					//convert float to string, and strcpy string to pass through socket
					sprintf(float_string, "%.2f", list[currAcc].balance);
					strcpy(intro_balance_string, "Your Balance is: $");	
					strcat(intro_balance_string, float_string);
					n = write(newsockfd, intro_balance_string, 100);
					memset(buffer, 0 , sizeof(buffer));
				}
		
				//finish
				else if (strcmp("finish", finish_instruction)==0) {
					finish = 1;
					n = write(newsockfd, "Session is finished\n", 20); 
					memset(buffer, 0 , sizeof(buffer));
				}
	

				else{
					n = write(newsockfd, "INVALID INPUT!\n", 32);
					memset(buffer, 0 , sizeof(buffer));
				}
			}  
		} 
   	
		//EXIT
		else if (strcmp("exit\n",exit)==0) {
			n = write(newsockfd,"DISCONNECTED FROM CLIENT\n",31);
     			close(newsockfd);
			close(sockfd);
			break; 

		}
	
		else{
			n = write(newsockfd, "INVALID INPUT\nWhat would you like to do?\n", 42);
			memset(buffer, 0 , sizeof(buffer));
		}
	
	}	
	
	printf("Thank you for using Lehman Brothers Bank, have a nice day.\nDICONNECTED FROM CLIENT\n"); 
   	return 0; 
}
