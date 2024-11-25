#ifndef _CONNECTIONS_H_
#define _CONNECTIONS_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define OP_CODE_SIZE 3
#define STATUS_SIZE 5
#define FNAME_SIZE 24
#define FSIZE_DIG 4
#define FSIZE 2048
#define MAX_BUF_SIZE 2090 //confirmar

int udp_conn(char *GSIP, char *GSport, char *message, char buffer[128]);
int tcp_conn(char *GSIP, char *GSPORT, char *message, char buffer[MAX_BUF_SIZE]);

#endif