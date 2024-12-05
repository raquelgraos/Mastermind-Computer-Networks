#ifndef _PLAYER_MAIN_H_
#define _PLAYER_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#define DEFAULT_PORT "58060" //"58011" //tejo
#define DEFAULT_GSIP "127.0.0.1" //"192.168.1.1" "193.136.138.142" //tejo

void print_usage(const char *prog_name);
int main(int argc, char *argv[]);

#endif