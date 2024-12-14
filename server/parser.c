#include "parser.h"
#include "handler.h"

int parse_input(char *input, char **message, int VERBOSE) {
    
    int input_len = strlen(input);
    char input_aux[input_len + 1];
    char OP_CODE[CODE_SIZE + 1];
    strcpy(input_aux, input);

    int counter = 0;
    char *token = strtok(input_aux, " ");
    while (token != NULL) {
        counter++;
        token = strtok(NULL, " ");
    }

    char *args[counter];
    char *arg = strtok(input, " ");
    int i = 0;
    while (arg != NULL) {
        args[i] = (char*) malloc(strlen(arg) + 1);
        if (args[i] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return 1;
        }
        strcpy(args[i], arg);
        i++;
        arg = strtok(NULL, " ");
    }

    char *last_arg = args[i - 1];
    size_t len = strlen(last_arg);
    if (len > 0 && last_arg[len - 1] == '\n') {
        last_arg[len - 1] = '\0'; // Remove the newline character.
    }

    if (VERBOSE) {
        if (counter == 1) {
            fprintf(stdout, "Request Type: %s\n", args[0]);
        } else if (counter > 1) {
            fprintf(stdout, "PLID: %s\nRequest Type: %s\n", args[1], args[0]);
        }
    }

    int res = 0;
    if (args[0] != NULL && !strcmp(args[0], "SNG")) {
        strcpy(OP_CODE, "RSG");
        res = start_s(args, message, counter, OP_CODE);

    } else if (args[0] != NULL && !strcmp(args[0], "TRY")) {
        res = try_s(args, message, counter);

    } else if (args[0] != NULL && !strcmp(args[0], "STR")) {
        res = show_trials_s(args, message, counter);

    } else if (args[0] != NULL && !strcmp(args[0], "SSB")) {
        res = scoreboard_s(args, message, counter);

    } else if (args[0] != NULL && !strcmp(args[0], "QUT")) {
       res = quit_s(args, message, counter);

    } else if (args[0] != NULL && !strcmp(args[0], "DBG")) {
        strcpy(OP_CODE, "RDB");
        res = start_s(args, message, counter, OP_CODE);

    } else {
        *message = (char*) malloc(5);
        if (*message == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            res = 1;
        } else strcpy(*message, "ERR\n");
    }

    for (int j = 0; j < counter; j++) {
        if (args[j] != NULL)
            free(args[j]);
    }
    return res;
}