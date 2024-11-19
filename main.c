#include "parser.h"

int main(int argc, char *argv[]) {

    char buffer[BUFSIZ];
    char command_line[BUFSIZ];
    char *command;
    int n_trials = 0;
    char PLID[7];
    
    while (1) {
        if (fgets(command_line, sizeof(command_line), stdin) != NULL) {
            strcpy(buffer, command_line);
            command = strtok(buffer, " ");
            int res;

            if (!strcmp(command, "start")) {
                res = parse_start_command(command_line, PLID);
                if (res == 0) printf("success\n");
                else if (res == 1)
                    printf("PLID must be 6 digits.\n");
                else if (res == 2)
                    printf("max_playtime musn't exceed 600 seconds.\n");
                else if (res == 3)
                    printf("Start Command requires 3 arguments.\n");

            } else if (!strcmp(command, "try")) {
                res = parse_try_command(command_line, PLID, n_trials);
                if (res == 0) {
                    n_trials++;
                    printf("success\n");
                } else if (res == 1)
                    printf("Invalid colour.\n");
                else if (res == 3)
                    printf("Try Command requires 4 arguments.\n");

            } else if (!strcmp(command, "show_trials\n") || !strcmp(command, "st\n")) {
                //if (!show_trials(PLID))
                
            } else if (!strcmp(command, "scoreboard\n") || !strcmp(command, "sb\n")) {
                //if (!show_scoreboard())

            } else if (!strcmp(command, "quit\n")) {
                //if (!quit(PLID))

            } else if (!strcmp(command, "exit\n")) {
                //if (!exit(PLID))

            } else if (!strcmp(command, "debug")) {
                res = parse_debug_command(command_line, PLID);
                if (res == 0) printf("success\n");
                else if (res == 1)
                    printf("PLID must be 6 digits.\n");
                else if (res == 2)
                    printf("max_playtime musn't exceed 600 seconds.\n");
                else if (res == 3)
                    printf("Debug Command requires 6 arguments.\n");
                else if (res == 4)
                    printf("Invalid colour.\n");

            } else {
                printf("invalid command");
            }

        } else {
            printf("Error");
        }
    }
}