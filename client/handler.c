#include "handler.h"
#include "parser.h"
#include "connections.h"

// MARK: START

/*
start_c: builds the start game message.

Returns 0 if the start was successful and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - PLID: the player ID
 - max_time_padded: maximum playtime (3 digits)
*/
int start_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded) {
    
    char OP_CODE[CODE_SIZE] = "SNG";

    char message[(CODE_SIZE + PLID_SIZE + TIME_SIZE + 3)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, PLID, PLID_SIZE);
    ptr += PLID_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, max_time_padded, TIME_SIZE);
    ptr += TIME_SIZE;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0'; //ensures null termination

    return start_r(GSIP, GSport, message);
}

/*
start_r: sends the message built in start_c() via UDP and interprets
the reply sent by the GS

Returns 0 if start was successful and 1 otherwise.

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: the message to be sent to the the GS
*/
int start_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }
    
    int n_args = 2;
    char **args = NULL;

    int res = parse_reply(msg_received, &args, n_args);

    int ret_value = 0;

    // if parse_reply() didn't fail
    if (res == 0) {

        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Starting a new game.\nYou are allowed to try 8 times for code with 4 positions, each: R,G,B,Y,O and P");

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
    } else {
        ret_value = 1; // if parse_reply() failed, the command wasn't successful
    }

    // frees the args that were allocated in parse_reply()
    for (int i = 0; i < n_args; i++) {
        if (args[i] != NULL) free(args[i]);
    }

    free(args);
    return ret_value;
}

// MARK: TRY

/*
try_c: builds the try game message.

Returns 0 if the try was successful, 2 if game ended and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - PLID: the player ID
 - args: the input arguments
 - n_trials: current trial number
*/
int try_c(char *GSIP, char *GSport, char *PLID, char *args[5], int n_trials) {
    char OP_CODE[CODE_SIZE] = "TRY";

    char message[(CODE_SIZE + PLID_SIZE + COLOURS_SIZE + TRIALS_SIZE + 7)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, PLID, PLID_SIZE);
    ptr += PLID_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    for (int i = 1; i < 5; i++) {
        memcpy(ptr, args[i], 1);
        ptr += 1;
        memcpy(ptr, " ", 1); 
        ptr += 1;
    }

    char n_trials_str[TRIALS_SIZE + 1];
    sprintf(n_trials_str, "%d", n_trials);
    memcpy(ptr, n_trials_str, strlen(n_trials_str));
    ptr += strlen(n_trials_str);

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0'; //ensures null termination

    return try_r(GSIP, GSport, message);
}

/*
try_r: sends the message built in try_c() via UDP and interprets
the reply sent by the GS

Returns 0 if the try was successful, 2 if game ended and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: the message to be sent to the the GS
*/
int try_r(char *GSIP, char *GSport, char *message) {
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }

    int max_n_args = 9;
    char **args = NULL;

    int ret_value = 0;
    int res = parse_reply(msg_received, &args, max_n_args);

    // if parse_reply() didn't fail
    if (res == 0) {
        int args_counter = 0;
        while (args[args_counter] != NULL) args_counter++;

        if (args[1] != NULL && !strcmp(args[1], "OK")) {

            fprintf(stdout, "nT:%s nB:%s nW:%s\n", args[2], args[3], args[4]);

            // nB = 4, therefore the key has been guessed
            if (args_counter == 5 && atoi(args[3]) == 4) {
                fprintf(stdout, "You won!\n");
                ret_value = 2;
            }

        // duplicate guess
        } else if (args[1] != NULL && !strcmp(args[1], "DUP")) {
            fprintf(stdout, "Repeated guess.\n");
            ret_value = 1;

        // invalid guess
        } else if (args[1] != NULL && !strcmp(args[1], "INV")) {
            fprintf(stderr, "Invalid trial\n");
            ret_value = 1;

        // trial is out of context
        } else if (args[1] != NULL && !strcmp(args[1], "NOK")) {
            fprintf(stdout, "Trial is out of context.\n");
            ret_value = 1;

        // no more attempts available -> game ends, key is revealed
        } else if (args[1] != NULL && !strcmp(args[1], "ENT")) {
            fprintf(stdout, "No more attempts available.\nSolution: ");
            for (int i = 2; args[i] != NULL; i++) fprintf(stdout, "%s ", args[i]);
            fprintf(stdout, "\n");
            ret_value = 2;

        // maximum playtime has been exceeded -> game ends, key is revealed
        } else if (args[1] != NULL && !strcmp(args[1], "ETM")) {
            fprintf(stdout, "Maximum playtime has been exceeded.\nSolution: ");
            for (int i = 2; args[i] != NULL; i++) fprintf(stdout, "%s ", args[i]);
            fprintf(stdout, "\n");
            ret_value = 2;

        // invalid syntax/PLID/colour
        } else if (args[1] != NULL && !strcmp(args[1], "ERR")) {
            fprintf(stderr, "Invalid syntax.\n");
            ret_value = 1;
        
        // default
        } else {
            fprintf(stderr, "Unknown status: %s.\n", args[1]);
            ret_value = 1;
        }
    } else {
        ret_value = 1; // if parse_reply() failed, the command wasn't successful
    }

    // frees the args allocated in deparsed_buffer()
    if (args != NULL) {
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }

    return ret_value;
}

// MARK: SHOW TRIALS

/*
show_trials_c: builds the show trials game message.

Returns 0 if the try was successful, 2 if the game ended and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - PLID: the player ID
*/
int show_trials_c(char *GSIP, char *GSport, char *PLID) {

    char OP_CODE[CODE_SIZE] = "STR";

    char message[(CODE_SIZE + PLID_SIZE + 2)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, PLID, PLID_SIZE);
    ptr += PLID_SIZE;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0'; //ensures null termination

    return show_trials_r(GSIP, GSport, message);
}

/*
show_trials_r: sends the message built in show_trials_c() 
via TCP and interprets the reply sent by the GS

Returns 0 if the show_trials was successful, 2 if the game ended and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: the message to be sent to the the GS
*/
int show_trials_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[MAX_BUF_SIZE];

    if (tcp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to TCP.\n");
        return 1;
    }

    int max_n_args = 5;
    char **args = NULL;

    int res = parse_reply(msg_received, &args, max_n_args);
  
    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;

    int ret_value = 0;

    // if parse_reply() didn't fail
    if (res == 0) {

        // there are no games (active or finished)
        if (args[1] != NULL && !strcmp(args[1], "NOK")) 
            fprintf(stdout, "No games found.\n");

        else if (args[1] != NULL && ((!strcmp(args[1], "ACT")) || (!strcmp(args[1], "FIN")))) {
            
            // there is no ongoing game for the player
            if (!strcmp(args[1], "FIN"))
                ret_value = 2;

            ssize_t fsize;
            fsize = atoi(args[3]);

            // gets current directory
            char *path = getcwd(NULL, 0);
            if (path == NULL) {
                fprintf(stderr, "Error: getcwd failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                return 1;
            }

            // enters the saved files directory
            int dir = chdir(SAVED_DIR);

            // if the directory doesn't yet exist, it's created
            if (dir != 0 && errno == ENOENT) {
                if (mkdir(SAVED_DIR, 0700)) {
                    fprintf(stderr, "Error: mkdir failed.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return 1;
                }
                int dir = chdir(SAVED_DIR);
                if (dir != 0) {
                    fprintf(stderr, "Error: failed to open directory.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return 1;
                }

            } else if (dir != 0) {
                fprintf(stderr, "Error: failed to open directory.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return 1;
            }

            // creates the new show_trials file
            int fd = open(args[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            if (fd == -1) {
                fprintf(stderr, "Error: failed to open %s file.\n", args[2]);
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return 1;
            }

            // writes fdata to file
            char *ptr = args[4];
            ssize_t n = write(fd, ptr, fsize);
            if (n == -1) {
                fprintf(stderr, "Error: write failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return 1;
            }

            // ensures all bytes are written
            ssize_t total_bytes_written = n;
            while (total_bytes_written < fsize) {
                ptr += n;

                n = write(fd, ptr, fsize - total_bytes_written);
                if (n == -1) {
                    fprintf(stderr, "Error: write failed.\n");
                    for (int i = 0; args[i] != NULL; i++) free(args[i]);
                    free(args);
                    free(path);
                    return 1;
                }

                total_bytes_written += n;
            }

            fprintf(stdout, "File saved: %s (%ld bytes).\n", args[2], fsize);
            if (display_file(fd, fsize)) {
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return 1;
            }

            close(fd);

            // changes back to original directory
            dir = chdir(path);
            if (dir != 0) 
                fprintf(stderr, "Error: failed to return to original directory.\n");

            free(path);
        }
    } else {
        ret_value = 1; // if parse_reply() failed, the command wasn't successful
    }

    // frees the args allocated in parse_reply()
    for (int i = 0; args[i] != NULL; i++) free(args[i]);
    free(args);
    return ret_value;
}

// MARK: SCOREBOARD

/*
show_sb_c: builds the scoreboard game message.

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
*/
void show_sb_c(char *GSIP, char *GSport) {

    char OP_CODE[CODE_SIZE] = "SSB";

    char message[(CODE_SIZE + 1)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0'; //ensures null termination

    show_sb_r(GSIP, GSport, message);
}

/*
show_sb_r: sends the message built in show_sb_c() 
via TCP and interprets the reply sent by the GS

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: the message to be sent to the the GS
*/
void show_sb_r(char *GSIP, char *GSport, char *message) {

    char msg_received[MAX_BUF_SIZE];

    if (tcp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to TCP.\n");
        return;
    }

    int max_n_args = 5;
    char **args = NULL;

    int res = parse_reply(msg_received, &args, max_n_args);
  
    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;

    // if parse_reply() didn't fail
    if (res == 0) {

        // there are no won games yet
        if (args[1] != NULL && !strcmp(args[1], "EMPTY")) 
            fprintf(stdout, "No won games found.\n");

        // found won games
        else if (args[1] != NULL && !strcmp(args[1], "OK")) {
            ssize_t fsize;
            fsize = atoi(args[3]);

            // gets current directory
            char *path = getcwd(NULL, 0);
            if (path == NULL) {
                fprintf(stderr, "Error: getcwd failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                return;
            }

            // enters the saved files directory
            int dir = chdir(SAVED_DIR);

            // if the directory doesn't yet exist, it's created
            if (dir != 0 && errno == ENOENT) {
                if (mkdir(SAVED_DIR, 0700)) {
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

            // creates the new scoreboard file
            int fd = open(args[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            if (fd == -1) {
                fprintf(stderr, "Error: failed to open %s file.\n", args[2]);
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }

            // writes fdata to file
            char *ptr = args[4];
            ssize_t n = write(fd, ptr, fsize);
            if (n == -1) {
                fprintf(stderr, "Error: write failed.\n");
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                free(path);
                return;
            }

            // ensures all bytes are written
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

            if (display_file(fd, fsize)) {
                free(path);
                for (int i = 0; args[i] != NULL; i++) free(args[i]);
                free(args);
                return;
            }

            close(fd);

            // changes back to original directory
            dir = chdir(path);
            if (dir != 0)
                fprintf(stderr, "Error: failed to return to original directory.\n");
            free(path);
        }
    }

    // frees the args allocated in parse_reply()
    for (int i = 0; args[i] != NULL; i++) free(args[i]);
    free(args);
}

int display_file(int fd, ssize_t fsize) {
    char buffer[fsize + 1];

    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to rewind the game file.\n");
        return 1;
    }

    // reads the created file
    int n = read(fd, buffer, fsize);
    int total_bytes_read = n;
    if (n == -1) {
        fprintf(stderr, "Error: read failed.\n");
        return 1;
    }

    // ensure all bytes are read
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

/*
quit_c: builds the quit game message.

Returns 0 if the quit was successful and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - PLID: the player ID
*/
int quit_c(char *GSIP, char *GSport, char *PLID) {

    char OP_CODE[CODE_SIZE] = "QUT";

    char message[(CODE_SIZE + PLID_SIZE + 2)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, PLID, PLID_SIZE);
    ptr += PLID_SIZE;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0'; //ensures null termination

    return quit_r(GSIP, GSport, message);
}

/*
quit_r: sends the message built in quit_c() via UDP and interprets
the reply sent by the GS

Returns 0 if the quit was successful and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: the message to be sent to the the GS
*/
int quit_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }
    
    int max_n_args = 6;
    char **args = NULL;

    int res = parse_reply(msg_received, &args, max_n_args);

    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;

    int ret_value = 0;

    // if parse_reply() didn't fail
    if (res == 0) {

        // quit was successful and secret key is shown
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Secret key: %s %s %s %s\n", args[2], args[3], args[4], args[5]);           
            fprintf(stdout, "Game ended successfully.\n");
        }

        // there was no ongoing game for PLID 
        else if (args[1] != NULL && !strcmp(args[1], "NOK")) {
            fprintf(stderr, "No ongoing game.\n");
        
        // invalid syntax
        } else if (args[1] != NULL && !strcmp(args[1], "ERR")) {
            fprintf(stderr, "Invalid syntax.\n");
            ret_value = 1;

        // default
        } else {
            fprintf(stderr, "Unknown status: %s.\n", args[1]);
            ret_value = 1;
        }
    } else {
        ret_value = 1; // if parse_reply() failed, the command wasn't successful
    }

    // frees the args allocated by parse_reply()
    for (int i = 0; args[i] != NULL; i++) free(args[i]);
    free(args);
    return ret_value;
}

// MARK: DEBUG

/*
debug_c: builds the debug game message.

Returns 0 if the debug was successful and 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - PLID: the player ID
 - max_time_padded: maximum playtime (3 digits)
 - args: the input arguments
*/
int debug_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded, char* args[7]){

    char OP_CODE[CODE_SIZE] = "DBG";

    char message[(CODE_SIZE + PLID_SIZE + TIME_SIZE + COLOURS_SIZE + 2)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, PLID, PLID_SIZE);
    ptr += PLID_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, max_time_padded, TIME_SIZE);
    ptr += TIME_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    for (int i = 3; i < 7; i++) {
        memcpy(ptr, &args[i][0], 1);
        ptr += 1;
        if (i < 6) memcpy(ptr, " ", 1);
        else memcpy(ptr, "\n", 1);
        ptr += 1;
    }

    *ptr = '\0'; //ensures null termination

    return debug_r(GSIP, GSport, message);
}

/*
debug_r: sends the message built in debug_c() via UDP and interprets
the reply sent by the GS

Returns 0 if debug was successful and 1 otherwise.

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: the message to be sent to the the GS
*/
int debug_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }

    int n_args = 2;
    char **args = NULL;

    int res = parse_reply(msg_received, &args, n_args);

    int ret_value = 0;

    // if parse_reply() didn't fail
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
    } else {
        ret_value = 1; // if parse_reply() failed, the command wasn't successful
    }

    // frees the args allocated by parse_reply()
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
    return ret_value;
}