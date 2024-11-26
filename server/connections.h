#ifndef _CONNECTIONS_H
#define _CONNECTIONS_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int udp_connection(char *GSport, int VERBOSE);

int tcp_connection(char *GSport, int VERBOSE);

#endif