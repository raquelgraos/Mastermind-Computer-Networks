#include "parser.h"
#include "player_main.h"

void print_usage(const char *prog_name) {
    fprintf(stdout, "Usage: %s [-n GSIP] [-p GSport]\n", prog_name);
    fprintf(stdout, "\t-n GSIP   : IP address of the game server (default: localhost)\n");
    fprintf(stdout, "\t-p GSport : Port of the game server (default: 58000+GN, where GN is the group number)\n");
}

int main(int argc, char *argv[]) {

    char *GSIP = DEFAULT_GSIP;
    char *GSport = DEFAULT_PORT;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-n")) {
            if (i + 1 < argc && strcmp(argv[i + 1], "-p")) 
                GSIP = argv[++i];
            else {
                fprintf(stderr, "Error: missing argument for -n.\n");
                print_usage(argv[0]);
                return 1;
            }
        } else if (!strcmp(argv[i], "-p")) {
            if (i + 1 < argc) 
                GSport = argv[++i];
            else {
                fprintf(stderr, "Error: missing argument for -p.\n");
                print_usage(argv[0]);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: Unknown argument '%s'.\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    char buffer[BUFSIZ];
    char input[BUFSIZ];
    char *command_line;
    char *command;
    int n_trials = 0;
    char PLID[7];
    
    while (1) {
        if (fgets(input, sizeof(input), stdin) != NULL) {
            command_line = strtok(input, "\n");
            strcpy(buffer, command_line);
            command = strtok(buffer, " ");
            int res;

            if (!strcmp(command, "start")) {
                res = parse_start_command(GSIP, GSport, command_line, PLID);
                if (res == 0) fprintf(stdout, "success\n");
                else if (res == 1)
                    fprintf(stderr, "Error: PLID must be a positive 6 digit number.\n");
                else if (res == 2)
                    fprintf(stderr, "Error: max_playtime must be positive and musn't exceed 600 seconds.\n");
                else if (res == 3)
                    fprintf(stderr, "Error: Start Command requires 3 arguments.\n");

            } else if (!strcmp(command, "try")) {
                res = parse_try_command(GSIP, GSport, command_line, PLID, n_trials);
                if (res == 0) {
                    n_trials++;
                    fprintf(stdout, "success\n");
                } else if (res == 1)
                    fprintf(stderr, "Error: Invalid colour.\n");
                else if (res == 3)
                    fprintf(stderr, "Error: Try Command requires 4 arguments.\n");

            } else if (!strcmp(command, "show_trials") || !strcmp(command, "st")) {
                if (!show_trials_c(GSIP, GSport, PLID)) fprintf(stdout, "success\n");
                
            } else if (!strcmp(command, "scoreboard") || !strcmp(command, "sb")) {
                if (!show_sb_c(GSIP, GSport)) fprintf(stdout, "success\n");

            } else if (!strcmp(command, "quit")) {
                if (!quit_c(GSIP, GSport, PLID)) fprintf(stdout, "success\n");

            } else if (!strcmp(command, "exit")) {
                // if (!exit(PLID))

            } else if (!strcmp(command, "debug")) {
                res = parse_debug_command(GSIP, GSport, command_line, PLID);
                if (res == 0) fprintf(stdout, "success\n");
                else if (res == 1)
                    fprintf(stderr, "Error: PLID must be a positive 6 digit number.\n");
                else if (res == 2)
                    fprintf(stderr, "Error: max_playtime must be positive and musn't exceed 600 seconds.\n");
                else if (res == 3)
                    fprintf(stderr, "Error: Debug Command requires 6 arguments.\n");
                else if (res == 4)
                    fprintf(stderr, "Error: Invalid colour.\n");

            } else {
                fprintf(stderr, "Error: invalid command.\n");
            }

        } else fprintf(stderr, "Error: fgets() failed.\n");
    }
}