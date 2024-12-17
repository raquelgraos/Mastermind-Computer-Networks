#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#define CODE_SIZE 3
#define TIME_SIZE 3
#define PLID_SIZE 6
#define TRIALS_SIZE 1 //max_trials = 8
#define COLOURS_SIZE 4

#define SAVED_DIR "client/SAVED/"

int start_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded);
int start_r(char *GSIP, char *GSport, char *message);

int try_c(char *GSIP, char *GSport, char *PLID, char *args[5], int n_trials);
int try_r(char *GSIP, char *GSport, char *message);

int show_trials_c(char *GSIP, char *GSport, char *PLID);
int show_trials_r(char *GSIP, char *GSport, char *message);

void show_sb_c(char *GSIP, char *GSport);
void show_sb_r(char *GSIP, char *GSport, char *message);
int display_scores(int fd, ssize_t fsize);

int quit_c(char *GSIP, char *GSport, char *PLID);
int quit_r(char *GSIP, char *GSport, char *message);

int debug_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded, char* args[7]);
int debug_r(char *GSIP, char *GSport, char *message);

#endif