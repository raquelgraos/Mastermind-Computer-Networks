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

#define MAX_BUF_SIZE 3072

int udp_conn(char *GSIP, char *GSport, char *message, char buffer[128]);
int tcp_conn(char *GSIP, char *GSPORT, char *message, char buffer[MAX_BUF_SIZE]);

#endif