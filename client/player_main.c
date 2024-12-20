#include "parser.h"
#include "player_main.h"
#include "handler.h"

volatile sig_atomic_t terminate = 0;

void handle_sigint(int sig) {
    (void)sig;
    terminate = 1;
}

void print_usage(const char *prog_name) {
    fprintf(stdout, "Usage: %s [-n GSIP] [-p GSport]\n", prog_name);
    fprintf(stdout, "\t-n GSIP   : IP address of the game server (default: localhost)\n");
    fprintf(stdout, "\t-p GSport : Port of the game server (default: 58000+GN, where GN is the group number)\n");
}

int main(int argc, char *argv[]) {
    char *GSIP = DEFAULT_GSIP;
    char *GSport = DEFAULT_PORT;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-n") && i + 1 < argc && strcmp(argv[i + 1], "-p")) {
            GSIP = argv[++i];
        } else if (!strcmp(argv[i], "-p") && i + 1 < argc) {
            GSport = argv[++i];
        } else {
            fprintf(stderr, "Error: Invalid argument '%s'.\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    char buffer[INPUT_SIZE];
    char input[INPUT_SIZE];
    char *command_line, *command;
    int n_trials = 1;
    char PLID[7];
    int active = 0; // no active game

    signal(SIGINT, handle_sigint);

    while (!terminate) {
        if (fgets(input, sizeof(input), stdin) != NULL) {
            command_line = strtok(input, "\n");
            if (command_line == NULL || strlen(command_line) == 0) continue;  // skip empty input
            strcpy(buffer, command_line);
            command = strtok(buffer, " ");
            if (parse_command(command, GSIP, GSport, command_line, PLID, &n_trials, &active)) return 0; // close terminal
        } else {
            if (terminate) break;
            else fprintf(stderr, "Error: fgets() failed.\n");
        }
    }
    fprintf(stdout, "\n");
    quit_c(GSIP, GSport, PLID);
    fprintf(stdout, "Terminating Client...\n");
    return 0;
}