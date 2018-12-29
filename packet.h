#ifndef _packet_H
#define _packet_H

#define MAXDATAELEN 256

typedef enum { REQUEST, RESPONSE, INSTRUCT } packetType;
typedef enum { TIME, NAME, LIST, MESSAGE } requestType;
typedef enum { CORRECT, WRONG } responseType;
typedef enum { FORWARD } instructType;

typedef struct request_packet {
	packetType pType;
	int type;
	char data[MAXDATAELEN]; 
} packet;

#endif