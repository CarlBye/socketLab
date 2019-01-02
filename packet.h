#ifndef _packet_H
#define _packet_H

#define MAXDATALEN 256

typedef enum { REQUEST, RESPONSE, INSTRUCT } packetType;
typedef enum { TIME, NAME, LIST, MESSAGE ,DISCONNECT} requestType;
typedef enum { CORRECT, WRONG } responseType;
typedef enum { FORWARD, TERMINATE } instructType;

typedef struct spacket {
	packetType pType;
	int type;
	char data[MAXDATALEN]; 
} packet;

#endif
