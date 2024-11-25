#include "parser.h"
#include "handler.h"

int parse_command(char *command, char *GSIP, char *GSport, char *command_line, char *PLID, int *n_trials) {
    int res;
    if (!strcmp(command, "start")) {
        if (strcmp(PLID, "EMPTY") == 0) {
            res = parse_start_command(GSIP, GSport, command_line, PLID);
            if (res != 0) {
                switch (res) {
                    case 1:
                        fprintf(stderr, "Error: PLID must be a positive 6 digit number.\n");
                        break;
                    case 2:
                        fprintf(stderr, "Error: max_playtime must be positive and musn't exceed 600 seconds.\n");
                        break;
                    case 3:
                        fprintf(stderr, "Error: Start Command requires 3 arguments.\n");
                        break;
                    default:
                        strcpy(PLID, "EMPTY");
                        break;
                }
            }
        } else {
            printf("You are already playing a game with player id %s\n", PLID);
        }
    } else if (!strcmp(command, "try")) {
        res = parse_try_command(GSIP, GSport, command_line, PLID, *n_trials);
        if (res == 0) {
            (*n_trials)++;
        } else {
            switch (res) {
                case 2:
                    fprintf(stderr, "Error: Invalid colour.\n");
                    break;
                case 3:
                    fprintf(stderr, "Error: Try Command requires 4 arguments.\n");
                    break;
                case 4:
                    strcpy(PLID, "EMPTY");
                    break;
            }
        }
    } else if (!strcmp(command, "show_trials") || !strcmp(command, "st")) {
        show_trials_c(GSIP, GSport, PLID);
    } else if (!strcmp(command, "scoreboard") || !strcmp(command, "sb")) {
        show_sb_c(GSIP, GSport);
    } else if (!strcmp(command, "quit") || !strcmp(command, "exit")) {
        if (!quit_c(GSIP, GSport, PLID)) strcpy(PLID, "EMPTY");
        else {
            fprintf(stdout, "Error quiting.\n");
            return 0;
        }
        if (!strcmp(command, "exit")) return 1;
    } else if (!strcmp(command, "debug")) {
        res = parse_debug_command(GSIP, GSport, command_line, PLID);
        switch (res) {
            case 1:
                fprintf(stderr, "Error: PLID must be a positive 6 digit number.\n");
                break;
            case 2:
                fprintf(stderr, "Error: max_playtime must be positive and musn't exceed 600 seconds.\n");
                break;
            case 3:
                fprintf(stderr, "Error: Debug Command requires 6 arguments.\n");
                break;
            case 4:
                fprintf(stderr, "Error: Invalid colour.\n");
                break;
        }
    } else {
        fprintf(stderr, "Error: invalid command.\n");
    }
    return 0;
}

int parse_start_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7]) {
    char *PLID_aux;
    char *max_time_str;
    char *command;

    command = strtok(buffer, " ");

    PLID_aux = strtok(NULL, " ");
    if (PLID_aux == NULL) return 3; // missing PLID arg

    max_time_str = strtok(NULL, "");
    if (max_time_str == NULL) return 3; // missing max_time arg

    char max_time_padded[4];
    int len_max_time = strlen(max_time_str);

    if (!is_valid_PLID(PLID_aux)) return 1; // invalid PLID
    else if (!is_valid_max_time(max_time_str, len_max_time)) return 2; // invalid max_time
    else {
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

int parse_try_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7], int n_trials) {
    char *args[5];
    char *arg;
    int i = 0;

    arg = strtok(buffer, " ");
    while (arg != NULL && i < 5) {
        args[i++] = arg;
        arg = strtok(NULL, " ");
    }

    // if arg isn't NULL or i != 5 there are too many/few args, respectively
    if (arg != NULL || i != 5) return 3;

    for (int j = 1; j < 5; j++) {
        if (strlen(args[j]) != 1 || !is_valid_color(args[j][0])) {
            return 2; // invalid colour (specific return valeus for specific errors)
        }
    }

    return try_c(GSIP, GSport, PLID, args, n_trials);
}

int parse_debug_command(char *GSIP, char *GSport, char buffer[BUFSIZ], char PLID[7]) {
    char *args[7];
    char *arg;
    int i = 0;

    arg = strtok(buffer, " ");
    while (arg != NULL && i < 7) {
        args[i++] = arg;
        arg = strtok(NULL, " ");
    }

    // if arg isn't NULL or i != 7 there are too many/few args, respectively
    if (arg != NULL || i != 7) return 3;

    for (int j = 3; j < 7; j++) {
        if (strlen(args[j]) != 1 || !is_valid_color(args[j][0])) {
            return 4; // invalid colour
        }
    }

    char max_time_padded[4];
    int len_max_time = strlen(args[2]);

    if (!is_valid_PLID(args[1])) return 1; // invalid PLID
    else if (!is_valid_max_time(args[2], len_max_time)) return 2; // invalid max_time
    else {
        strcpy(PLID, args[1]);
        if (len_max_time == 1) {
            strcpy(max_time_padded, "00");
            strcat(max_time_padded, args[2]);
        } else if (len_max_time == 2) {
            strcpy(max_time_padded, "0");
            strcat(max_time_padded, args[2]);
        } else strcpy(max_time_padded, args[2]);

        debug_c(GSIP, GSport, PLID, max_time_padded, args);
        return 0;
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


int deparse_buffer(char *message, char ***args, int n_args) {
    //printf("message received in deparser: %s", message);

    *args = (char **)malloc((n_args + 1) * sizeof(char *)); // allocate space for n_args + 1 (for NULL).
    if (*args == NULL) return 1; // malloc error

    int i = 0;
    char *arg = strtok(message, " ");
    while (arg != NULL && i < n_args) {
        //printf("arg %d: %s\n", i, arg);
        (*args)[i] = (char *)malloc((strlen(arg) + 1) * sizeof(char)); // +1 for null-terminator.
        if ((*args)[i] == NULL) {
            for (int j = 0; j < i; j++) free((*args)[j]);
            free(*args);
            return 1; // malloc error
        }
        strcpy((*args)[i], arg);
        i++;
        if (i == n_args - 1) arg = strtok(NULL, "\0");
        else arg = strtok(NULL, " ");
    }
    // null-terminate the array of arguments.
    (*args)[i] = NULL;
    if (arg != NULL)
        return 2; // invalid number of arguments (except for RST and RSS)
    else{
        // handle newline 
        if (i > 0) {
            char *last_arg = (*args)[i - 1];
            size_t len = strlen(last_arg);
            if (len > 0 && last_arg[len - 1] == '\n') {
                last_arg[len - 1] = '\0'; // Remove the newline character.
            }
        }
    }

    return 0;
}

