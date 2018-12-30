#include "packet.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// the size of listen quene is 10
#define LISTENSIZE 10
// the port of the server csocket is 2299
#define SERVERPORT 2299
// the size of packet size 
#define BUFFSIZE 300

typedef struct TIDLIST
{
	pthread_t tid;
	struct TIDLIST *next;
}tidList;

tidList *start_node;
tidList *current_node;

// the array to store socketfd of client, if no connect the element will be 0
struct client_fd
{
	int tail;
	int fd[LISTENSIZE];
}cfd;

struct CLIENTLIST
{
	int fd;
	unsigned short port;
	struct in_addr addr;
}client_list[LISTENSIZE];

int server_fd;


// sy the array of sockfd
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// init the tid list
int initTidList();
// if success return 0 if fail return -1
int isAccept();
// use the new client fd to create the new pthread to handle the connect
int createConnectPthread();
// the function to handle the new connect
void* handleConnect();
//after socket connect succeed, server send a packet to ensure
void sendHelloPacket(int fd);
// analysis the packet and make response
int handlePacket(packet *get_packet, int fd);
// use this function to response each type of request
void handleTimePacket(packet *get_packet, int fd);
void handleNamePacket(packet *get_packet, int fd);
void handleListPacket(packet *get_packet, int fd);
void handleMessagePacket(packet *get_packet, int fd);
int handleDisconnectPacket(packet *get_packet, int fd);

int main(int argc, char *argv[])
{
	int struct_len;
	struct sockaddr_in server_addr;
	
	if(initTidList() == -1)
		return 0;

	// init the array of client socketfd
	cfd.tail = 0;
	memset(cfd.fd, LISTENSIZE, 0);
	for(int i = 0; i < LISTENSIZE; i++)
		client_list[i].fd = 0;
	// init the attribute of sockaddr_in to create new server socket
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVERPORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(server_addr.sin_zero), 8);
	struct_len = sizeof(struct sockaddr_in);

	// generate the new server fd
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	// bind the server socket 
	if(bind(server_fd, (struct sockaddr*)&server_addr, struct_len) < 0){
		// bind error output the error info and exit the server
		printf("BIND ERROR!\n");
		return 0;
	}
	// listen the client connect 
	if(listen(server_fd, LISTENSIZE) < 0){
		printf("LISTEN ERROR!\n");
		return 0;
	}
	while(1){
		sleep(20);
		isAccept();
	}

}

/*
 * if init tid list success return 0
 * if init tid list failed return -1
 */
int initTidList()
{
	if(start_node = (tidList*)malloc(sizeof(struct TIDLIST)) == NULL){
		printf("Init tid list failed!\nPlease reboot the server!\n");
		return -1;
	}
	start_node->next = NULL;
	current_node = start_node;
	return 0;
}

/*
 * if accept success will create a pthread to handle the connect
 * if accept failed will kill this function
 */
int isAccept()
{
	int temp_fd, struct_len;
	struct sockaddr_in client_addr;
	int is_cfd_full;
	
	struct_len = sizeof(struct sockaddr_in);
	if(cfd.tail < LISTENSIZE){
		pthread_mutex_lock(&mutex);
		cfd.fd[cfd.tail] = accept(server_fd, (struct sockaddr*)&client_addr, &struct_len);
		cfd.tail++;
		is_cfd_full = 0;
		for(int i = 0; i < LISTENSIZE; i++){
			if(client_list[i].fd == 0){
				client_list[i].fd == cfd.fd[cfd.tail - 1];
				client_list[i].port = client_addr.sin_port;
				client_list[i].addr = client_addr.sin_addr;
				is_cfd_full = 0;
			}
		}
		if(is_cfd_full){
			printf("socketfd:%d connect failed!\n", cfd.fd[cfd.tail - 1]);
		}else{
			printf("socketfd:%d connect succeed!\n", cfd.fd[cfd.tail - 1]);
			createConnectPthread();
		}
		pthread_mutex_unlock(&mutex);
		
	}else{
		printf("Client Connect is full!\n");
	}
	return 0;
}

/*
 * to create a new pthread for each socket connect
 */
int createConnectPthread()
{
	pthread_attr_t attr;
	pthread_t tid;
	
	pthread_attr_init(&attr);
	pthread_create(&tid, &attr, handleConnect, (void*)NULL);
	if(current_node->next = (tidList*)malloc(sizeof(struct TIDLIST)) == NULL){
		printf("Create new node failed!");
		return -1;
	}
	current_node->next->tid = tid;
	current_node->next->next = NULL;
	return 0;
}

/*
 * use this function to handle each socket connect
 */
void* handleConnect()
{
	int fd, num_bytes;
	char buff[BUFFSIZE];
	packet *get_packet;

	// get the client fd from the cfd
	pthread_mutex_lock(&mutex);
	cfd.tail--;
	fd = cfd.fd[cfd.tail];
	pthread_mutex_unlock(&mutex);
	// after connect succeed send a packet to esure
	sendHelloPacket(fd);

	while(1){
		num_bytes = recv(fd, buff, BUFFSIZE, 0);
		if(num_bytes < 0){
			printf("from socketfd:%d get error packet\n", fd);
			break;
		}
		get_packet = (packet*)buff;
		if(handlePacket(get_packet, fd) == -1){
			return;
		}
	}


}

/*
 * after socket connect succeed, server send a packet to ensure
 */
void sendHelloPacket(int fd)
{
	packet hello_packet;
	hello_packet.pType = RESPONSE;
	hello_packet.type = CORRECT;
	int num_bytes = sprintf(hello_packet.data, "Socketfd:%d connect succeed!", fd);
	send(fd, (char*)&hello_packet, sizeof(hello_packet), 0);
}

/*
 *	to analysis the packet and make response
 */
int handlePacket(packet *get_packet, int fd)
{

	if(get_packet->pType == REQUEST){
		requestType r_type = (requestType)get_packet->type;
		if(r_type == TIME){
			handleTimePacket(get_packet, fd);
		}else if(r_type == NAME){
			handleNamePacket(get_packet, fd);
		}else if(r_type == LIST){
			//need
		}else if(r_type == MESSAGE){
			handleMessagePacket(get_packet, fd);
		}else if(r_type == DISCONNECT){// handle client quit and close socket
			return handleDisconnectPacket(get_packet, fd);
		}else{
			printf("Error type!\n");
		}
	}else if(get_packet->pType == RESPONSE){
		printf("Can't recv RESPONSE packet!\n");
	}else if(get_packet->pType == INSTRUCT){
		printf("Can't recv INSTRUCT packet!\n");
	}else{
		printf("Error Packet!\n");
	}
	return 0;
}

/*
 * generate the response packet for time request
 */
void handleTimePacket(packet *get_packet, int fd)
{
	packet s_packet;
	time_t t;
	struct tm *lt;
	time(&t);
	lt = localtime(&t);
	int num_bytes = sprintf(s_packet.data, "Date:%d/%d/%d Time:%d:%d:%d", lt->tm_year + 1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
	s_packet.pType = RESPONSE;
	s_packet.type = CORRECT;
	send(fd, (char*)&s_packet, sizeof(s_packet), 0);
	return;
}

/*
 * generate the response packet fpr name request
 */
void handleNamePacket(packet *get_packet, int fd)
{
	packet s_packet;
	int res = gethostname(s_packet.data, sizeof(s_packet.data));
	s_packet.pType = RESPONSE;
	s_packet.type = CORRECT;
	send(fd, (char*)&s_packet, sizeof(s_packet), 0);
	return;
}

/*
 * generate the response packet for List request
 */
void handleListPacket(packet *get_packet, int fd)
{
	packet s_packet;
	s_packet.pType = RESPONSE;
	s_packet.type = CORRECT;
	s_packet.data[0] = '\0';
	pthread_mutex_lock(&mutex);
		for(int i = 0; i < LISTENSIZE; i++){
			if(client_list[i].fd > 0){
				int num_bytes = sprintf(s_packet.data, "%sSockfd:%d Port:%hu IP:%s\n", s_packet.data, client_list[i].fd, client_list[i].port, inet_ntoa(client_list[i].addr));
			}
		}
	pthread_mutex_lock(&mutex);
	send(fd, (char*)&s_packet, sizeof(s_packet), 0);
	return;
}

/*
 * generate the response packet for Message request
 */
void handleMessagePacket(packet *get_packet, int fd)
{
	packet s_packet;
	char *des_fd_str;
	memcpy(des_fd_str, get_packet->data, sizeof(int));
	int des_fd = *((int*)des_fd_str);// need to test
	int isExist = 0;
	pthread_mutex_lock(&mutex);
		for(int i = 0; i < LISTENSIZE; i++){
			if(client_list[i].fd == des_fd){
				isExist = 1;
				break;
			}
		}
	pthread_mutex_unlock(&mutex);
	if(isExist == 1){
		s_packet.pType = INSTRUCT;
		s_packet.type = FORWARD;
		memcpy(s_packet.data, &(get_packet->data[sizeof(int)]), strlen(&(get_packet->data[sizeof(int)])));
		send(des_fd, (char*)&s_packet, sizeof(s_packet), 0);
		packet r_packet;
		r_packet.pType = RESPONSE;
		r_packet.type = CORRECT;
		int num_bytes = sprintf(r_packet.data, "message to socketfd:%d send succeed!", des_fd);
		send(fd, (char*)&r_packet, sizeof(r_packet), 0);

	}else{
		s_packet.pType = RESPONSE;
		s_packet.type = CORRECT;
		int num_bytes = sprintf(s_packet.data, "No socketfd:%d", des_fd);
		send(fd, (char*)&s_packet, sizeof(s_packet), 0);
	}
	return;
}

/*
 * update the client_list for disconnect request
 */
int handleDisconnectPacket(packet *get_packet, int fd)
{
	printf("disconnect socketfd:%d\n", fd);
	
	pthread_mutex_lock(&mutex);
	for(int i = 0; i < LISTENSIZE; i++){
		if(client_list[i].fd == fd){
			client_list[i].fd = 0;
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
	close(fd);
	return -1;
}

