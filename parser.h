#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int parse_start_command(char buffer[BUFSIZ], char PLID[7]);
int parse_try_command(char buffer[BUFSIZ], char PLID[7], int n_trials);
int parse_debug_command(char buffer[BUFSIZ], char PLID[7]);

bool is_valid_color(char c);


#endif