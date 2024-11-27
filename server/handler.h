#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>

#define CODE_SIZE 3
#define TIME_SIZE 3
#define PLID_SIZE 6
#define KEY_SIZE 4
#define HEADER_SIZE 38 //40
#define DATE_SIZE 19
#define ONGOING_GAME_SIZE 15
#define COLOURS "RGBYOP"

bool is_valid_PLID(const char PLID[7]);
bool is_valid_max_time(char *max_time_str, int len_max_time);

int start_s(char **args, char **message, int n_args);
int start_game(const char PLID[7], char *max_time);

/*int try_s(char **args, char **message, int n_args);
int show_trials_s(char **args, char **message, int n_args);
int scoreboard_s(char **args, char **message, int n_args);
int quit_s(char **args, char **message, int n_args);
int debug_s(char **args, char **message, int n_args);*/

int check_ongoing_game(const char PLID[7]);
void generate_random_key(char *key);
void assemble_header(char *header, const char PLID[7], char *mode, char *max_time);

#endif