#include "handler.h"
#include "../client/parser.h" // nao funciona idk why

bool is_valid_PLID(char *PLID) {
    if (strlen(PLID) != PLID_SIZE) return false;

    for (int i = 0; i < PLID_SIZE; i++)
        if (!isdigit(PLID[i])) return false;
    return true;
}

bool is_valid_max_time(char *max_time_str, int len_max_time) {
    if (len_max_time > TIME_SIZE) return false;

    for (int i = 0; i < len_max_time; i++) {
        if (!isdigit(max_time_str[i])) return false;
    }

    int max_time = atoi(max_time_str);
    if (max_time <= 0 || max_time > 600) return false;
    else return true;
}

// MARK: START
int start_s(char **args, char **message, int n_args) {

    char OP_CODE[CODE_SIZE] = "RSG";

    char status[4] = "";
    if (n_args != 3) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
    }

    char PLID[7];
    if (args[1] != NULL) strcpy(PLID, args[1]);
    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
    }
    
    char max_time_str[4];
    if (args[2] != NULL) strcpy(max_time_str, args[2]);
    int max_time = atoi(max_time_str);

    if (!is_valid_max_time(max_time_str, 3)) {
        fprintf(stderr, "Error: invalid max time.\n");
        strcpy(status, "ERR");
    }
    
    /*if (check_ongoing_game(PLID) == 0) {
        if (start_game(PLID, max_time) == 0) // game started successfully
            strcpy(status, "OK")
    } else strcpy(status, "NOK"); // ongoing game already*/

    strcpy(status, "OK");
    
    int status_len = strlen(status);
    *message = (char *) malloc(3 + 1 + strlen(status) + 2);
    if (*message == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }
    char *ptr = *message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, status, status_len);
    ptr += status_len;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0';
    
    return 0;
}

/*// MARK: TRY
int try_s(char **args, char **message, int n_args) {

}

// MARK: SHOW_TRIALS
int show_trials_s(char **args, char **message, int n_args) {

}

// MARK: SCOREBOARD
int scoreboard_s(char **args, char **message, int n_args) {

}

// MARK: QUIT
int quit_s(char **args, char **message, int n_args) {

}

// MARK: DEBUG
int debug_s(char **args, char **message, int n_args) {

}*/