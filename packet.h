#ifndef _packet_H
#define _packet_H

#define MAXDATAELEN 256
#define PORT 5684
typedef enum { TIME, NAME, LIST, MESSAGE } requestType;
typedef enum { CORRECT, WRONG } responseType;
typedef enum { FORWARD } instructType;

typedef struct request_packet {
	requestType type;
	char data[MAXDATAELEN]; 
} requestPacket;

typedef struct response_packet {
	responseType type;
	char data[MAXDATAELEN]; 
} responsePacket;

typedef struct instruct_packet {
	instructType type;
	char data[MAXDATAELEN]; 
} instructPacket;

#endif