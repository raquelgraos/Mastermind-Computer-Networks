#ifndef _GS_MAIN_H
#define _GS_MAIN_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#define DEFAULT_PORT "58060"
#define GAMES_DIR "GAMES"
#define SCORES_DIR "SCORES"

int main(int argc, char *argv[]);

void print_usage(const char *prog_name);

#endif