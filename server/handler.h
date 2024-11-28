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
#define HEADER_SIZE 68 // 38 + 20
#define DATE_SIZE 19
#define FULLTIME_STR_SIZE 20
#define ONGOING_GAME_SIZE 15
#define COLOURS "RGBYOP"

bool is_valid_PLID(const char PLID[PLID_SIZE + 1]);
bool is_valid_max_time(const char max_time[TIME_SIZE + 1], int len_max_time);

int start_s(char **args, char **message, int n_args);
int start_game(const char PLID[PLID_SIZE + 1], const char max_time[TIME_SIZE + 1]);
int send_start_message(char OP_CODE[CODE_SIZE], char status[4], char **message);

int try_s(char **args, char **message, int n_args);
int end_game_after_try();
//int try_game();
int send_end_try_message(char OP_CODE[CODE_SIZE], char status[4], char PLID[PLID_SIZE + 1], char **message);
//int send_try_message();

/*int show_trials_s(char **args, char **message, int n_args);
int scoreboard_s(char **args, char **message, int n_args);
int quit_s(char **args, char **message, int n_args);
int debug_s(char **args, char **message, int n_args);*/

int open_game_file(char PLID[PLID_SIZE + 1], int *fd, char *path);
int check_ongoing_game(const char PLID[PLID_SIZE + 1]);
int check_if_in_time(char PLID[7]);
void generate_random_key(char *key);
int assemble_header(char *header, const char PLID[PLID_SIZE + 1], char *mode, const char max_time[TIME_SIZE + 1]);

#endif