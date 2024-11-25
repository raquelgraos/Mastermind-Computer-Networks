#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

int parse_command(char *command, char *GSIP, char *GSport, char *command_line, char *PLID, int *n_trials);
int parse_start_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7]);
int parse_try_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7], int n_trials);
int parse_debug_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7]);

bool is_valid_color(char c);
bool is_valid_PLID(char *PLID);
bool is_valid_max_time(char *max_time_str, int len_max_time);

int deparse_buffer(char *buffer, char ***args, int n_args);


#endif