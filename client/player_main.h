#ifndef _PLAYER_MAIN_H_
#define _PLAYER_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#define DEFAULT_PORT "58060"
#define DEFAULT_GSIP "127.0.0.1"
#define INPUT_SIZE 26

void print_usage(const char *prog_name);
int main(int argc, char *argv[]);

#endif