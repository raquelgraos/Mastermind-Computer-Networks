#include "gs_main.h"

void print_usage(const char *prog_name) {
    fprintf(stdout, "Usage: %s [-p GSport] [-v]\n", prog_name);
    fprintf(stdout, "\t-p GSport : Port of the game server (default: 58000+GN, where GN is the group number)\n");
    fprintf(stdout, "\t-v : program operates in verbose mode\n");
}

int main(int argc, char *argv[]) {

    char *GSport = DEFAULT_PORT;

    int VERBOSE = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-p") && i + 1 < argc && strcmp(argv[i + 1], "-v")) {
            GSport = argv[++i];
        } else if (!strcmp(argv[i], "-v")) {
            VERBOSE = 1;
        } else {
            fprintf(stderr, "Error: Invalid argument '%s'.\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    pid_t udp;
    pid_t tcp;

    udp = fork();
    if (udp == -1) {
        fprintf(stderr, "Error: fork failed.\n");
        return 1;
    } else if (udp == 0) {
        udp_connection(GSport, VERBOSE);
        return 0;
    }

    tcp = fork();
    if (tcp == -1) {
        fprintf(stderr, "Error: fork failed.\n");
        return 1;
    } else if (tcp == 0) {
        tcp_connection(GSport, VERBOSE);
        return 0;
    }

    int status;
    waitpid(udp, &status, 0);
    waitpid(tcp, &status, 0);
    fprintf(stdout, "Terminating...\n");
    return 0;
}