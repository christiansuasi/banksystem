#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg){
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[]){ 
	//socket start 
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
    

	char check[256];
	char buffer[256];
	char exit1[5];
	
	//error message if localhost and port number is not provided
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	//declare 2nd shell arg as port number	
	portno = atoi(argv[2]);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");
	
	//should be localhost 	
	server = gethostbyname(argv[1]);
	
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	//???
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
	serv_addr.sin_port = htons(portno);
	
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		error("ERROR connecting");
	}

	//start of prompt
	printf("CONNECTED TO SERVER\nWelcome to the Lehman Brothers Bank\nWhat would you like to do?\n");
	
	while (1){
		
		//should only input args every 2 seconds
		printf("Ready for input in 2...\n");
		sleep(1);
		printf("Ready for input in 1...\n");
		sleep(1);
		printf("Ready for input!\n");
	
		//clear buffer before accepting arg	
		bzero(buffer,256);
	
		//fgets the arg from stdin and put into buffer	
		fgets(buffer,255,stdin);
		
		memcpy(&exit1, &buffer, 5);
		n = write(sockfd,buffer,strlen(buffer));
		if (n < 0) error("ERROR writing to socket");
		
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
	
		if (n < 0) error("ERROR reading from socket");
		
		memcpy(&check, &buffer, 256);
    		
		//exit
		if(strcmp("exit\n",exit1)==0){
			printf("Thank you for using Lehman Brothers Bank\nEND OF CLIENT PROCESS\nDISCONNECTED FROM SERVER\n");
			close(sockfd);
			break;
		}
		//clear buffer	
		memset(&buffer[0], 0, sizeof(buffer));
	}
	return 0;
}
