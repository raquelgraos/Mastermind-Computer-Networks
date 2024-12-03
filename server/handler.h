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
#define HEADER_SIZE 49
#define DATE_SIZE 19
#define FULLTIME_STR_SIZE 20
#define ONGOING_GAME_SIZE 15
#define COLOURS "RGBYOP"

int start_s(char **args, char **message, int n_args, char OP_CODE[CODE_SIZE + 1]);
int start_game(const char PLID[PLID_SIZE + 1], const char max_time[TIME_SIZE + 1], char key[KEY_SIZE + 1]);


int try_s(char **args, char **message, int n_args);
int end_game(int time_passed, char PLID[PLID_SIZE + 1], char key[KEY_SIZE + 1], char mode);
int try_game(char PLID[PLID_SIZE + 1], char given_key[KEY_SIZE + 1], int nT, int time_passed, int *nW, int *nB);

//int show_trials_s(char **args, char **message, int n_args);
//int scoreboard_s(char **args, char **message, int n_args);
int quit_s(char **args, char **message, int n_args);
int debug_s(char **args, char **message, int n_args);

int send_simple_message(char OP_CODE[CODE_SIZE], char status[4], char **message);
int send_end_message(char OP_CODE[CODE_SIZE + 1], char status[4], char key[KEY_SIZE + 1], char **message);
int send_try_message(char OP_CODE[CODE_SIZE + 1], char status[4], char **message, char nT_str[2], int nW, int nB);

bool is_valid_PLID(const char PLID[PLID_SIZE + 1]);
bool is_valid_max_time(const char max_time[TIME_SIZE + 1], int len_max_time);
int get_secret_key(char *key, int fd);
int open_active_game(char PLID[PLID_SIZE + 1], int *fd);
int check_ongoing_game(const char PLID[PLID_SIZE + 1]);
int check_if_in_time(char PLID[PLID_SIZE + 1], int *time_passed);
int check_repeated_or_invalid(int fd, const char given_key[KEY_SIZE + 1], int nT);
void generate_random_key(char *key);
int assemble_header(char *header, const char PLID[PLID_SIZE + 1], char *mode, const char max_time[TIME_SIZE + 1], char key[KEY_SIZE + 1]);
int colour_to_index(char c);

#endif