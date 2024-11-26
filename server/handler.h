#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <stdlib.h>
#include <stdbool.h>

#define CODE_SIZE 3
#define TIME_SIZE 3
#define PLID_SIZE 6

bool is_valid_PLID(char *PLID);
bool is_valid_max_time(char *max_time_str, int len_max_time);

int start_s(char **args, char **message, int n_args);
/*int try_s(char **args, char **message, int n_args);
int show_trials_s(char **args, char **message, int n_args);
int scoreboard_s(char **args, char **message, int n_args);
int quit_s(char **args, char **message, int n_args);
int debug_s(char **args, char **message, int n_args);*/

#endif