#include "parser.h"
#include "handler.h"

/*
parse_command: parses the command input by the player

Arguments:
 - command: command (start, try, show trials, scoreboard, debug, quit or exit)
 - GSIP:
 - GSport:
 - command_line: full input
 - PLID: player ID (to be populated)
 - n_trials: current trial number
 - active: 1 if there is an active game, 0 if there isn't
*/
int parse_command(char *command, char *GSIP, char *GSport, char *command_line, char *PLID, int *n_trials, int *active) {
    int res;

    if (!strcmp(command, "start")) {
        if (*active == 0) {
            res = parse_start_command(GSIP, GSport, command_line, PLID);
            if (res == 0) {
                (*active) = 1; // there is now an active game
                (*n_trials) = 1; //reset trials
            } 
        } else printf("You are already playing a game with player id %s\n", PLID);

    } else if (!strcmp(command, "try")) {
        res = parse_try_command(GSIP, GSport, command_line, PLID, *n_trials);
        if (res == 0) (*n_trials)++;
        else if (res == 2) (*active) = 0; // game ended

    } else if (!strcmp(command, "show_trials") || !strcmp(command, "st")) {
        if (show_trials_c(GSIP, GSport, PLID) == 2) { //WARNING: mudei isto para == 2 em vez de == 0 porque == 2 é quando é FIN
            (*active) = 0; // game ended
        }

    } else if (!strcmp(command, "scoreboard") || !strcmp(command, "sb")) {
        show_sb_c(GSIP, GSport);

    } else if (!strcmp(command, "quit") || !strcmp(command, "exit")) {
        if (!quit_c(GSIP, GSport, PLID)) (*active) = 0;
            
        if (!strcmp(command, "exit")) return 1; // close terminal

    } else if (!strcmp(command, "debug")) {
        if (*active == 0) {
            res = parse_debug_command(GSIP, GSport, command_line, PLID);
            if (res == 0) {
                (*active) = 1; // there is now an active game
                (*n_trials) = 1; //reset trials
            } 
        } else printf("You are already playing a game with player id %s\n", PLID);
    } else {
        fprintf(stderr, "Error: invalid command.\n");
    }
    return 0;
}

/* 
parse_start_command: validates the input and, if valid, performs the start command.

Returns 0 if successful and 1 otherwise.

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - buffer: the player input
 - PLID: the player ID
*/
int parse_start_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7]) {
    char *PLID_aux;
    char *max_time_str;
    
    // splits the input by its white spaces
    strtok(buffer, " ");
    
    PLID_aux = strtok(NULL, " ");
    if (PLID_aux == NULL) {
        fprintf(stderr, "Error: Start Command requires 3 arguments.\n");
        return 1;
    }

    max_time_str = strtok(NULL, "");
    if (max_time_str == NULL) {
        fprintf(stderr, "Error: Start Command requires 3 arguments.\n");
        return 1;
    }

    char max_time_padded[4];
    int len_max_time = strlen(max_time_str);

    // verifies the validity of the PLID
    if (!is_valid_PLID(PLID_aux)) {
        fprintf(stderr, "Error: PLID must be a positive 6 digit number.\n");
        return 1;

    // verifies the validity of the maximum playtime
    } else if (!is_valid_max_time(max_time_str, len_max_time)) {
        fprintf(stderr, "Error: max_playtime must be positive and musn't exceed 600 seconds.\n");
        return 1;

    // if both valid, pads the maximum playtime with necessary 0s
    } else {
        strcpy(PLID, PLID_aux);
        if (len_max_time == 1) {
            strcpy(max_time_padded, "00");
            strcat(max_time_padded, max_time_str);
        } else if (len_max_time == 2) {
            strcpy(max_time_padded, "0");
            strcat(max_time_padded, max_time_str);
        } else strcpy(max_time_padded, max_time_str);

        return start_c(GSIP, GSport, PLID, max_time_padded);
    }
}

/* 
parse_try_command: validates the input and, if valid, performs the try command.

Returns 0 if successful, 2 if game ended and 1 otherwise.

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - buffer: the player input
 - PLID: the player ID
 - n_trials: current trial number
*/
int parse_try_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7], int n_trials) {
    char *args[5];
    char *arg;
    int i = 0;

    // splits the input by its white spaces
    arg = strtok(buffer, " ");
    while (arg != NULL && i < 5) {
        args[i++] = arg;
        arg = strtok(NULL, " ");
    }

    // if arg isn't NULL or i != 5 there are too many/few args, respectively
    if (arg != NULL || i != 5) {
        fprintf(stderr, "Error: Try Command requires 4 arguments.\n");
        return 1;
    }

    // verifies the validity of the colors
    for (int j = 1; j < 5; j++) {
        if (strlen(args[j]) != 1 || !is_valid_color(args[j][0])) {
            fprintf(stderr, "Error: Invalid colour.\n");
            return 1;
        }
    }

    return try_c(GSIP, GSport, PLID, args, n_trials);
}

/* 
parse_debug_command: validates the input and, if valid, performs the debug command.

Returns 0 if successful and 1 otherwise.

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - buffer: the player input
 - PLID: the player ID
*/
int parse_debug_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7]) {
    char *args[7];
    char *arg;
    int i = 0;

    // splits the input by its white spaces
    arg = strtok(buffer, " ");
    while (arg != NULL && i < 7) {
        args[i++] = arg;
        arg = strtok(NULL, " ");
    }

    // if arg isn't NULL or i != 7 there are too many/few args, respectively
    if (arg != NULL || i != 7) {
        fprintf(stderr, "Error: Debug Command requires 7 arguments.\n");
        return 1;
    }

    // verifies the validity of the colors
    for (int j = 3; j < 7; j++) {
        if (strlen(args[j]) != 1 || !is_valid_color(args[j][0])) {
            fprintf(stderr, "Error: Invalid colour.\n");
            return 1;
        }
    }

    char max_time_padded[4];
    int len_max_time = strlen(args[2]);

    // verifies the validity of the PLID
    if (!is_valid_PLID(args[1])) {
        fprintf(stderr, "Error: PLID %s must be a positive 6 digit number.\n", args[1]);
        return 1;
    }

    // verifies the validity of the maximum playtime
    else if (!is_valid_max_time(args[2], len_max_time)) {
        fprintf(stderr, "Error: max_playtime must be positive and musn't exceed 600 seconds.\n");
        return 1;

    // if both valid, pads the maximum playtime with necessary 0s
    } else {
        strcpy(PLID, args[1]);
        if (len_max_time == 1) {
            strcpy(max_time_padded, "00");
            strcat(max_time_padded, args[2]);
        } else if (len_max_time == 2) {
            strcpy(max_time_padded, "0");
            strcat(max_time_padded, args[2]);
        } else strcpy(max_time_padded, args[2]);

        return debug_c(GSIP, GSport, PLID, max_time_padded, args);
    }
}

bool is_valid_color(char c) {
    return c == 'R' || c == 'G' || c == 'B' || c == 'Y' || c == 'O' || c == 'P';
}

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

/*
parse_reply: parses the GS reply

Returns 0 if successful and 1 otherwise.

Arguments:
 - message: the message sent by the GS
 - args: the message's arguments (to be populated)
 - n_args: given (max) number of message's arguments
*/
int parse_reply(char *message, char ***args, int n_args) {

    // allocate space for n_args + 1 (for NULL)
    *args = (char **)malloc((n_args + 1) * sizeof(char *));
    if (*args == NULL) return 1;

    int i = 0;

    //splits the input by its whitespaces
    char *arg = strtok(message, " ");
    while (arg != NULL && i < n_args) {
        // ensures space for null terminator
        (*args)[i] = (char *)malloc((strlen(arg) + 1) * sizeof(char));
        if ((*args)[i] == NULL) {
            for (int j = 0; j < i; j++) free((*args)[j]);
            free(*args);
            fprintf(stderr, "Error: memory allocation failed.\n");
            return 1;
        }
        
        // copies the parsed arg to the args array
        strcpy((*args)[i], arg);
        i++;
        if (i == n_args - 1) arg = strtok(NULL, "\0");
        else arg = strtok(NULL, " ");
    }

    // null-terminate the array of arguments
    (*args)[i] = NULL;
    if (arg != NULL) {
        fprintf(stderr, "Error: received invalid message.\n");
        return 1;
    } else {
        // handle newline 
        if (i > 0) {
            char *last_arg = (*args)[i - 1];
            size_t len = strlen(last_arg);
            if (len > 0 && last_arg[len - 1] == '\n') {
                // remove the newline character
                last_arg[len - 1] = '\0';
            }
        }
    }

    return 0;
}

