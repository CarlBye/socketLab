#ifndef _packet_H
#define _packet_H

#define MAXDATALEN 256

typedef enum { REQUEST = 1, RESPONSE, INSTRUCT } packetType;
typedef enum { TIME = 1, NAME, LIST, MESSAGE ,DISCONNECT} requestType;
typedef enum { CORRECT = 1, WRONG } responseType;
typedef enum { FORWARD = 1, TERMINATE } instructType;

typedef struct spacket {
	packetType pType;
	int type;
	char data[MAXDATALEN]; 
} packet;

#endif
