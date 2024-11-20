#include "parser.h"
#include "commands.h"

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

        return 0;
        //if (!start_c(GSIP, GSport, PLID, max_time_padded)) return 0;
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
            return 1; // invalid colour
        }
    }

    char c1, c2, c3, c4;
    c1 = args[1][0];
    c2 = args[2][0];
    c3 = args[3][0];
    c4 = args[4][0];

    return 0;
    // if (!try_c(GSIP, GSport, PLID, c1, c2, c3, c4, n_trials)) return 0;
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

    char c1, c2, c3, c4;
    c1 = args[3][0];
    c2 = args[4][0];
    c3 = args[5][0];
    c4 = args[6][0];

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

        return 0;
        //if (!debug_c(GSIP, GSport, PLID, max_time_padded, c1, c2, c3, c4)) return 0;
    }
}

bool is_valid_color(char c) {
    return c == 'R' || c == 'G' || c == 'B' || c == 'Y' || c == 'O' || c == 'P';
}

bool is_valid_PLID(char *PLID) {
    if (strlen(PLID) != 6) return false;

    for (int i = 0; i < 6; i++)
        if (!isdigit(PLID[i])) return false;
    return true;
}

bool is_valid_max_time(char *max_time_str, int len_max_time) {
    if (len_max_time > 3) return false;

    for (int i = 0; i < len_max_time; i++) {
        if (!isdigit(max_time_str[i])) return false;
    }

    int max_time = atoi(max_time_str);
    if (max_time <= 0 || max_time > 600) return false;
    else return true;
}
