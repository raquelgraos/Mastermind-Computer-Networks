#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <stdio.h>
#include <string.h>

#define CODE_SIZE 3
#define TIME_SIZE 3
#define PLID_SIZE 6
#define TRIALS_SIZE 1 //max_trials = 8
#define COLOURS_SIZE 4

int start_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded);
int try_c(char *GSIP, char *GSport, char *PLID, char *args[5], int n_trials);
int show_trials_c(char *GSIP, char *GSport, char *PLID);
int show_sb_c(char *GSIP, char *GSport);
int quit_c(char *GSIP, char *GSport, char *PLID);
int exit_c(char *GSIP, char *GSport);
int debug_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded, char* args[7]);

#endif