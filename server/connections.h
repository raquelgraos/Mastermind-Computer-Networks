#ifndef _CONNECTIONS_H
#define _CONNECTIONS_H

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>

#define UDP_MAX_BUF_SIZE 24 // na vdd acho que é 24 mas nao va o diabo tece las
#define TCP_MAX_BUF_SIZE 11

int udp_connection(char *GSport, int VERBOSE);

int tcp_connection(char *GSport, int VERBOSE);

#endif