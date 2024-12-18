#include "parser.h"
#include "handler.h"

/*
parse_input: parses the message sent by the player and performs
the given commands, populating the reply message to be sent to the player

Returns 0 if successful and 1 otherwise

Arguments:
 - input: the message sent by the player
 - message: reply message (to be populated)
 - verbose: flag that indicates if verbose mode is on
*/
int parse_input(char *input, char **message, int VERBOSE) {
    
    int input_len = strlen(input);
    char input_aux[input_len + 1];
    char OP_CODE[CODE_SIZE + 1];
    strcpy(input_aux, input);

    // Splits the input copy by its whitespaces to count its arguments
    int counter = 0;
    char *token = strtok(input_aux, " ");
    while (token != NULL) {
        counter++;
        token = strtok(NULL, " ");
    }

    // Splits the given input by its whitespaces and fills the args array
    char *args[counter];
    char *arg = strtok(input, " ");
    int i = 0;
    while (arg != NULL) {
        // Allocates memory for each argument (ensuring space for null terminator)
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
        last_arg[len - 1] = '\0'; // Remove the newline character
    }

    if (VERBOSE) {
        if (counter == 1) {
            fprintf(stdout, "Request Type: %s\n", args[0]);
        } else if (counter > 1) {
            fprintf(stdout, "PLID: %s\nRequest Type: %s\n", args[1], args[0]);
        }
    }

    int res = 0;
    // Start command
    if (args[0] != NULL && !strcmp(args[0], "SNG")) {
        strcpy(OP_CODE, "RSG");
        res = start_s(args, message, counter, OP_CODE);

    // Try command
    } else if (args[0] != NULL && !strcmp(args[0], "TRY")) {
        res = try_s(args, message, counter);

    // Show trials command
    } else if (args[0] != NULL && !strcmp(args[0], "STR")) {
        res = show_trials_s(args, message, counter);

    // Scoreboard command
    } else if (args[0] != NULL && !strcmp(args[0], "SSB")) {
        res = scoreboard_s(message);

    // Quit (and exit) command
    } else if (args[0] != NULL && !strcmp(args[0], "QUT")) {
       res = quit_s(args, message, counter);

    // Debug command
    } else if (args[0] != NULL && !strcmp(args[0], "DBG")) {
        strcpy(OP_CODE, "RDB");
        res = start_s(args, message, counter, OP_CODE);

    // Default
    } else {
        *message = (char*) malloc(5);
        if (*message == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            res = 1;
        } else strcpy(*message, "ERR\n");
    }

    // Frees the allocated args
    for (int j = 0; j < counter; j++) {
        if (args[j] != NULL)
            free(args[j]);
    }
    return res;
}