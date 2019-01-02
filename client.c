#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/shm.h>

#include "packet.h"

#define PORT 2299

void *waitServer(void* socketfd){
	packet pkt;
	while(1) {
		memset(pkt.data, 0, sizeof(pkt.data));       //check!
		recv(*(int*)socketfd, (char *)&pkt, sizeof(pkt), 0);
		printf("%d\n",pkt.type);
		printf("%s\n",pkt.data);
		if(pkt.type == 1) {
			printf("(Client) Server connection terminated!\n");
			pthread_exit(0);
		}
	}
}

void sendDisRequestPacket(int socketfd) {
	packet pkt;
	pkt.pType = REQUEST;
	pkt.type = (int)DISCONNECT;
	memset(pkt.data, 0, sizeof(pkt.data));
	printf("pType:%d type:%d\n",pkt.pType, pkt.type);
	send(socketfd, (char *)&pkt, sizeof(pkt), 0);
}

void sendTimeRequestPacket(int socketfd) {
	packet pkt;
	pkt.pType = REQUEST;
	pkt.type = (int)TIME;
	memset(pkt.data, 0, sizeof(pkt.data));
	printf("pType:%d type:%d\n",pkt.pType, pkt.type);
	send(socketfd, (char *)&pkt, sizeof(pkt), 0);
}

void sendNameRequestPacket(int socketfd) {
	packet pkt;
	pkt.pType = REQUEST;
	pkt.type = (int)NAME;
	memset(pkt.data, 0, sizeof(pkt.data));
	printf("pType:%d type:%d\n",pkt.pType, pkt.type);
	send(socketfd, (char *)&pkt, sizeof(pkt), 0);
}

void sendListRequestPacket(int socketfd) {
	packet pkt;
	pkt.pType = REQUEST;
	pkt.type = (int)LIST;
	memset(pkt.data, 0, sizeof(pkt.data));
	printf("pType:%d type:%d\n",pkt.pType, pkt.type);
	send(socketfd, (char *)&pkt, sizeof(pkt), 0);
}

void sendMessageRequestPacket(int socketfd) {
	int destClient;
	packet pkt;
	pkt.pType = REQUEST;
	pkt.type = (int)MESSAGE;
	memset(pkt.data, 0, sizeof(pkt.data));

	printf("(Client) PLease input client id you want to send: ");
	scanf("%d", &destClient);
	memcpy(pkt.data, &destClient, sizeof(int));

	printf("(Client) PLease input message you want to send: ");
	getchar();//refresh buffer
	fgets(pkt.data + sizeof(int), MAXDATALEN - sizeof(int), stdin);

	send(socketfd, (char *)&pkt, sizeof(pkt), 0);
	printf("pType:%d type:%d\n",pkt.pType, pkt.type);
	printf("(Client) sending message to client %d\n", destClient);
}

int main(void) {
	int choice; //NUM user choose
	char ip[16];
	int port;

	while(1) {
		//start view
start:	printf("        Welcome To Socket Client!       \n");
		printf("**************Start Menu****************\n");
		printf("* 1.  Connect To Server                *\n");
		printf("* 2.  Exit                             *\n");
		printf("****************************************\n");
		printf("(Client) Please input Num to choose corresponding function: ");
		scanf("%d", &choice);
		while (choice != 1 && choice != 2){
			printf("Illegal Input\n");
			printf("(Client) Please input Num to choose corresponding function: ");
			scanf("%d", &choice);
		}
		if(2 == choice) {
			exit(0);
		}
		printf("(Client) Please input server IP: ");
		scanf("%s", ip);

		//define socketClient
	    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	    if(-1 == socketfd) {
	    	printf("(Client) Define socketfd failed!\n");
	    }
	    //define socketaddr_in
	    struct sockaddr_in inAddr;
	    memset(&inAddr, 0, sizeof(inAddr));
	    inAddr.sin_family = AF_INET;
	    inAddr.sin_port = htons(PORT);  //server port
	    inAddr.sin_addr.s_addr = inet_addr(ip);  //server IP

	    //connect server
	    printf("(Client) Connecting!\n");
	    if (connect(socketfd, (struct sockaddr *)&inAddr, sizeof(inAddr)) < 0)
	    {
	        perror("connect");
	        goto start;
	    }
	    printf("(Client) Connected successfully!\n");

	    //create thread
		pthread_t p;
		pthread_create(&p, NULL, &waitServer, (void *)&socketfd);

	    //function view
		printf("***************************Function Menu*************************\n");
		printf("* 1.  Disconnect                                                *\n");
		printf("* 2.  Get Time From Server                                      *\n");
		printf("* 3.  Get Name From Server                                      *\n");
		printf("* 4.  Get Client-List (connecting to Server)                    *\n");
		printf("* 5.  Send Message To Other Clients                             *\n");
		printf("* 6.  Exit                                                      *\n");
		printf("*****************************************************************\n");
		printf("(Client) Please input Num to choose corresponding function: ");
		while(1){
			scanf("%d", &choice);
			while (choice < 1 && choice > 6){
				printf("(Client) Illegal Input\n");
				printf("(Client) Please input Num to choose corresponding function: ");
				scanf("%d", &choice);
			}

			switch(choice){
				case 1: 
					pthread_cancel(p);
					sendDisRequestPacket(socketfd);
					goto start;
				case 2:
					sendTimeRequestPacket(socketfd);
					break;
				case 3:
					sendNameRequestPacket(socketfd);
					break;
				case 4:
					sendListRequestPacket(socketfd);
					break;
				case 5:
					sendMessageRequestPacket(socketfd);
					break;
				case 6: 
					pthread_cancel(p);
					sendDisRequestPacket(socketfd);
					exit(0);
			}
		} 
	    close(socketfd);
	 	pthread_cancel(p);
	}
	return 0;
}
