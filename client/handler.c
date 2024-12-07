#include "handler.h"
#include "parser.h"
#include "connections.h"

// MARK: START
int start_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded) {
    
    char OP_CODE[CODE_SIZE] = "SNG";

    char message[(CODE_SIZE + PLID_SIZE + TIME_SIZE + 3)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, PLID, PLID_SIZE*sizeof(char));
    ptr += PLID_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, max_time_padded, TIME_SIZE*sizeof(char));
    ptr += TIME_SIZE*sizeof(char);

    memcpy(ptr, "\n", sizeof(char));
    ptr += sizeof(char);

    *ptr = '\0'; //ensures null termination

    return start_r(GSIP, GSport, message); // 0 if success, 1 otherwise
}

int start_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }
    
    int n_args = 2;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, n_args);

    int ret_value = 0;
    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Game started successfully.\n");
        } else if (args[1] != NULL && !strcmp(args[1], "NOK")) {
            fprintf(stderr, "There is already an ongoing game.\n");
            ret_value = 1;
        } else if (args[1] != NULL && !strcmp(args[1], "ERR")) {
            fprintf(stderr, "Invalid syntax.\n");
            ret_value = 1;
        } else {
            fprintf(stderr, "Unknown status: %s.\n", args[1]);
            ret_value = 1;
        }
    }

    for (int i = 0; i < n_args; i++) {
        if (args[i] != NULL) free(args[i]);
    }
    free(args);
    return ret_value;
}

// MARK: TRY
int try_c(char *GSIP, char *GSport, char *PLID, char *args[5], int n_trials) {
    char OP_CODE[CODE_SIZE] = "TRY";

    // sizeof(char) = 1
    char message[(CODE_SIZE + PLID_SIZE + COLOURS_SIZE + TRIALS_SIZE + 7)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", sizeof(char));
    ptr += 1;

    memcpy(ptr, PLID, PLID_SIZE);
    ptr += PLID_SIZE;

    memcpy(ptr, " ", sizeof(char));
    ptr += 1;

    for (int i = 1; i < 5; i++) {
        memcpy(ptr, args[i], sizeof(char));
        ptr += 1;
        memcpy(ptr, " ", sizeof(char)); 
        ptr += 1;
    }

    char n_trials_str[TRIALS_SIZE + 1];
    sprintf(n_trials_str, "%d", n_trials);// change n_trials to 1 to test inv
    memcpy(ptr, n_trials_str, strlen(n_trials_str));
    ptr += strlen(n_trials_str);

    memcpy(ptr, "\n", sizeof(char));
    ptr += 1;

    *ptr = '\0'; //ensures null termination

    //printf("Message to send: '%s'\n", message); 

    return try_r(GSIP, GSport, message); // 0 if success, 2 if game ended, 1 otherwise
}

int try_r(char *GSIP, char *GSport, char *message) {
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }

    int max_n_args = 9;
    char **args = NULL;

    int ret_value = 0;
    int res = deparse_buffer(msg_received, &args, max_n_args);
    if (res == 0) {
        int args_counter = 0;
        while (args[args_counter] != NULL) args_counter++;

        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "nT:%s nB:%s nW:%s\n", args[2], args[3], args[4]);
            if (args_counter == 5 && atoi(args[3]) == 4) {
                fprintf(stdout, "You won!\n");
                ret_value = 2;
            }
        } else if (args[1] != NULL && !strcmp(args[1], "DUP")) {
            fprintf(stdout, "Repeated guess.\n");
            ret_value = 1;
        } else if (args[1] != NULL && !strcmp(args[1], "INV")) {
            fprintf(stderr, "Invalid trial\n"); //TODO
            ret_value = 1;
        } else if (args[1] != NULL && !strcmp(args[1], "NOK")) {
            fprintf(stdout, "Trial is out of context.\n");
            ret_value = 1;
        } else if (args[1] != NULL && !strcmp(args[1], "ENT")) { //reveal secret key and end game
            fprintf(stdout, "No more attempts available.\nSolution: ");
            for (int i = 2; args[i] != NULL; i++) fprintf(stdout, "%s ", args[i]);
            fprintf(stdout, "\n");
            ret_value = 2;
        } else if (args[1] != NULL && !strcmp(args[1], "ETM")) { //reveal secret key and end game
            fprintf(stdout, "Maximum playtime has been exceeded.\nSolution: ");
            for (int i = 2; args[i] != NULL; i++) fprintf(stdout, "%s ", args[i]);
            fprintf(stdout, "\n");
            ret_value = 2;
        } else if (args[1] != NULL && !strcmp(args[1], "ERR")) {
            fprintf(stderr, "Invalid syntax.\n");
            ret_value = 1;
        } else {
            fprintf(stderr, "Unknown status: %s.\n", args[1]);
            ret_value = 1;
        }
    }

    if (args != NULL) {
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }

    return ret_value;
}

// MARK: SHOW TRIALS
void show_trials_c(char *GSIP, char *GSport, char *PLID) {

    char OP_CODE[CODE_SIZE] = "STR";

    char message[(CODE_SIZE + PLID_SIZE + 2)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, PLID, PLID_SIZE*sizeof(char));
    ptr += PLID_SIZE*sizeof(char);

    memcpy(ptr, "\n", sizeof(char));
    ptr += sizeof(char);

    *ptr = '\0'; //ensures null termination

    show_trials_r(GSIP, GSport, message);
}

void show_trials_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[MAX_BUF_SIZE];

    if (tcp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to TCP.\n");
        return;
    }

    int max_n_args = 5;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, max_n_args);
  
    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;

    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "NOK")) 
            fprintf(stdout, "No games found.\n");
        else if (args[1] != NULL && ((!strcmp(args[1], "ACT")) || (!strcmp(args[1], "FIN")))) {
            ssize_t fsize;
            fsize = atoi(args[3]);

            char *path = getcwd(NULL, 0);
            if (path == NULL) {
                fprintf(stderr, "Error: getcwd failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                return;
            }

            int dir = chdir(SAVED_DIR);
            if (dir != 0 && errno == ENOENT) {
                if (mkdir(SAVED_DIR, 0700)) { //confirmar mode
                    fprintf(stderr, "Error: mkdir failed.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return;
                }
                int dir = chdir(SAVED_DIR);
                if (dir != 0) {
                    fprintf(stderr, "Error: failed to open directory.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return;
                }
            } else if (dir != 0) {
                fprintf(stderr, "Error: failed to open directory.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }
            int fd = open(args[2], O_CREAT | O_RDWR); //confirmar modes
            if (fd == -1) {
                fprintf(stderr, "Error: failed to open %s file.\n", args[2]);
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }

            char *ptr = args[4];
            ssize_t n = write(fd, ptr, fsize);
            if (n == -1) {
                fprintf(stderr, "Error: write failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }

            ssize_t total_bytes_written = n;
            while (total_bytes_written < fsize) {
                ptr += n;

                n = write(fd, ptr, fsize - total_bytes_written);
                if (n == -1) {
                    fprintf(stderr, "Error: write failed.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return;
                }

                total_bytes_written += n;
            }
            close(fd);
            fprintf(stdout, "File saved: %s (%ld bytes).\n", args[2], fsize);
            dir = chdir(path);
            if (dir != 0) 
                fprintf(stderr, "Error: failed to return to original directory.\n");
            free(path);
        }
    }

    for (int i = 0; args[i] != NULL; i++) free(args[i]);
    free(args);

}

// MARK: SCOREBOARD
void show_sb_c(char *GSIP, char *GSport) {

    char OP_CODE[CODE_SIZE] = "SSB";

    char message[(CODE_SIZE + 1)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, "\n", sizeof(char));
    ptr += sizeof(char);

    *ptr = '\0'; //ensures null termination

    show_sb_r(GSIP, GSport, message);
}

void show_sb_r(char *GSIP, char *GSport, char *message) {

    char msg_received[MAX_BUF_SIZE];

    if (tcp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to TCP.\n");
        return;
    }

    int max_n_args = 5;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, max_n_args);
  
    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;

    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "EMPTY")) 
            fprintf(stdout, "No won games found.\n");
        else if (args[1] != NULL && !strcmp(args[1], "OK")) {
            ssize_t fsize;
            fsize = atoi(args[3]);

            char *path = getcwd(NULL, 0);
            if (path == NULL) {
                fprintf(stderr, "Error: getcwd failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                return;
            }
            int dir = chdir(SAVED_DIR);
            if (dir != 0 && errno == ENOENT) {
                if (mkdir(SAVED_DIR, 0700)) { //confirmar mode
                    fprintf(stderr, "Error: mkdir failed.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return;
                }
                int dir = chdir(SAVED_DIR);
                if (dir != 0) {
                    fprintf(stderr, "Error: failed to open created directory.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return;
                }
            } else if (dir != 0) {
                fprintf(stderr, "Error: failed to open directory.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }
            int fd = open(args[2], O_CREAT | O_RDWR); //confirmar modes
            if (fd == -1) {
                fprintf(stderr, "Error: failed to open %s file.\n", args[2]);
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }

            char *ptr = args[4];
            ssize_t n = write(fd, ptr, fsize);
            if (n == -1) {
                fprintf(stderr, "Error: write failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }
            ssize_t total_bytes_written = n;
            while (total_bytes_written < fsize) {
                ptr += n;

                n = write(fd, ptr, fsize - total_bytes_written);
                if (n == -1) {
                    fprintf(stderr, "Error: write failed.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return;
                }

                total_bytes_written += n;
            }
            if (display_scores(fd, fsize)) {
                free(path);
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                return;
            }
            close(fd);
            dir = chdir(path);
            if (dir != 0)
                fprintf(stderr, "Error: failed to return to original directory.\n");
            free(path);
        }
    } 

    for (int i = 0; args[i] != NULL; i++) free(args[i]);
    free(args);
}

int display_scores(int fd, ssize_t fsize) {
    char buffer[fsize + 1];

    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to rewind the game file.\n");
        return 1;
    }
    int n = read(fd, buffer, fsize);
    int total_bytes_read = n;
    if (n == -1) {
        fprintf(stderr, "Error: read failed.\n");
        return 1;
    }
    while (total_bytes_read < fsize) {
       int n = read(fd, buffer, fsize);
        if (n == -1) {
            fprintf(stderr, "Error: read failed.\n");
            return 1;
        } 
    }

    buffer[fsize] = '\0';

    fprintf(stdout, "%s", buffer);
    return 0;
}

// MARK: QUIT
int quit_c(char *GSIP, char *GSport, char *PLID) {

    char OP_CODE[CODE_SIZE] = "QUT";

    char message[(CODE_SIZE + PLID_SIZE + 2)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, PLID, PLID_SIZE*sizeof(char));
    ptr += PLID_SIZE*sizeof(char);

    memcpy(ptr, "\n", sizeof(char));
    ptr += sizeof(char);

    *ptr = '\0'; //ensures null termination

    return quit_r(GSIP, GSport, message); //0 if success, 1 otherwise
}

int quit_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }
    
    int max_n_args = 6;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, max_n_args);

    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;

    int ret_value = 0;
    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Secret key: %s %s %s %s\n", args[2], args[3], args[4], args[5]);           
            fprintf(stdout, "Game ended successfully.\n");
        } 
        else if (args[1] != NULL && !strcmp(args[1], "NOK")) {
            fprintf(stderr, "No ongoing game.\n");
            ret_value = 1;
        } else if (args[1] != NULL && !strcmp(args[1], "ERR")) {
            fprintf(stderr, "Invalid syntax.\n");
            ret_value = 1;
        } else {
            fprintf(stderr, "Unknown status: %s.\n", args[1]);
            ret_value = 1;
        }
    }

    for (int i = 0; args[i] != NULL; i++) free(args[i]);
    free(args);
    return ret_value;
}

// MARK: DEBUG
int debug_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded, char* args[7]){

    char OP_CODE[CODE_SIZE] = "DBG";

    char message[(CODE_SIZE + PLID_SIZE + TIME_SIZE + COLOURS_SIZE + 2)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, PLID, PLID_SIZE*sizeof(char));
    ptr += PLID_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, max_time_padded, TIME_SIZE*sizeof(char));
    ptr += TIME_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    for (int i = 3; i < 7; i++) {
        memcpy(ptr, &args[i][0], sizeof(char));
        ptr += sizeof(char);
        if (i < 6) memcpy(ptr, " ", sizeof(char));
        else memcpy(ptr, "\n", sizeof(char));
        ptr += sizeof(char);
    }

    *ptr = '\0'; //ensures null termination

    return debug_r(GSIP, GSport, message); // 0 if success, 1 otherwise
}

int debug_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }

    int n_args = 2;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, n_args);

    int ret_value = 0;
    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Game started successfully.\n");
        } else if (args[1] != NULL && !strcmp(args[1], "NOK")) {
            fprintf(stderr, "Already an ongoing game.\n");
            ret_value = 1;
        } else if (args[1] != NULL && !strcmp(args[1], "ERR")) {
            fprintf(stderr, "Invalid syntax.\n");
            ret_value = 1;
        } else {
            fprintf(stderr, "Unknown status: %s.\n", args[1]);
            ret_value = 1;
        }
    }

    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
    return ret_value;
}