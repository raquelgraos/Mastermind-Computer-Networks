#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <stdio.h>

int start_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded);
int try_c(char *GSIP, char *GSport, char *PLID, char c1, char c2, char c3, char c4, int n_trials);
int show_trials_c(char *GSIP, char *GSport);
int scoreboard_c(char *GSIP, char *GSport);
int quit_c(char *GSIP, char *GSport);
int exit_c(char *GSIP, char *GSport);
int debug_c(char *GSIP, char *GSport);

#endif