#include "handler.h"
#include "gs_main.h"

// MARK: START

/*
start_s: validates the arguments and performs operations
related to starting a game.

Returns 0 if the start was successful and 1 otherwise

Arguments:
 - args: the player message's arguments
 - message: reply message (to be populated)
 - n_args: number of arguments
 - OP_CODE: protocol message code
*/
int start_s(char **args, char **message, int n_args, char OP_CODE[CODE_SIZE + 1]) {
    
    char status[4];

    // Validate number of arguments
    if (n_args != 3 && n_args != 7) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }

    // validate and extract PLID
    if (args[1] == NULL || !is_valid_PLID(args[1])) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }
    char PLID[PLID_SIZE + 1];
    strncpy(PLID, args[1], PLID_SIZE);
    PLID[PLID_SIZE] = '\0';

    // validate and extract max_time
    if (args[2] == NULL || !is_valid_max_time(args[2], TIME_SIZE)) {
        fprintf(stderr, "Error: invalid max time.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }
    char max_time[TIME_SIZE + 1];
    strncpy(max_time, args[2], TIME_SIZE);
    max_time[TIME_SIZE] = '\0';

    // verify and extract key
    char key[KEY_SIZE + 1] = {0}; // Initialize as empty key
    if (n_args == 7) {
        if (args[3] && args[4] && args[5] && args[6]) {
            snprintf(key, KEY_SIZE + 1, "%s%s%s%s", args[3], args[4], args[5], args[6]);
        } else {
            fprintf(stderr, "Error: invalid key in DEBUG.\n");
            strcpy(status, "ERR");
            return send_simple_message(OP_CODE, status, message);
        }
    }

    // check ongoing game
    int res = check_ongoing_game(PLID);
    
    // No ongoing game
    if (res == 0) {
        // Start a new game
        if (start_game(PLID, max_time, key) == 0) {
            strcpy(status, "OK");
        } else {
            strcpy(status, "ERR");
        }

    // Ongoing game detected
    } else if (res == 2) {
        strcpy(status, "NOK");

    // Error in checking ongoing game
    } else {
        return 1;
    }

    // Builds reply message 
    return send_simple_message(OP_CODE, status, message);
}

/*
start_game: starts the game internally.

Returns 0 if the start was successful and 1 otherwise

Arguments:
 - PLID: player ID
 - max_time: maximum playtime (3 digits)
 - key: secret 4-color key
*/
int start_game(const char PLID[PLID_SIZE + 1], const char max_time[TIME_SIZE + 1], char key[KEY_SIZE + 1]) {
    
    // Get the current directory
    char *original_path = getcwd(NULL, 0);
    if (original_path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    // Prepare the filename
    char filename[ONGOING_GAME_SIZE + 1];
    snprintf(filename, sizeof(filename), "GAME_%s.txt", PLID);

    // change to GAMES directory
    if (chdir(GAMES_DIR) != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(original_path);
        return 1;
    }

    // Create the game file
    int fd = open(filename, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", filename);
        free(original_path);
        return 1;
    }

    // Assemble the game header
    char header[HEADER_SIZE + 1];
    if (assemble_header(header, PLID, max_time, key)) {
        close(fd);
        free(original_path);
        return 1;
    }

    // Write the header to the file
    ssize_t len = strlen(header);
    if (write(fd, header, len) != len) {
        fprintf(stderr, "Error: write failed.\n");
        close(fd);
        free(original_path);
        return 1;
    }

    // close the file and return to the original directory
    close(fd);
    if (chdir(original_path) != 0) {
        fprintf(stderr, "Error: failed to return to the original directory.\n");
        free(original_path);
        return 1;
    }

    free(original_path);
    return 0;
}

// MARK: TRY

/*
try_s: validates the arguments and performs operations
related to a key trial.

Returns 0 if the trial was successful and 1 otherwise

Arguments:
 - args: the player message's arguments
 - message: reply message (to be populated)
 - n_args: number of arguments
*/
int try_s(char **args, char **message, int n_args) {

    char status[4];

    char OP_CODE[CODE_SIZE + 1] = "RTR";

    // Extracts PLID
    char PLID[PLID_SIZE + 1];
    if (n_args > 1) {
        if (args[1] != NULL) 
            strcpy(PLID, args[1]);
        else {
            strcpy(status, "ERR");
            return send_simple_message(OP_CODE, status, message);
        }
    // if no PLID was sent (trial out of context)
    } else {
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);
    }

    PLID[PLID_SIZE] = '\0';

    // Validates the PLID
    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }

    // Validates number of arguments
    if (n_args != 7) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }
    
    // Extracts the guessed key
    char given_key[KEY_SIZE + 1];
    for (int i = 2; i < 2 + KEY_SIZE; i++) {
        if (args[i] != NULL) {
            given_key[i-2] = args[i][0];
        }
    }

    given_key[KEY_SIZE] = '\0';

    // Extracts the reported trial number
    char nT_str[2];
    if (args[6] != NULL) strcpy(nT_str, args[6]);

    nT_str[1] = '\0';

    int nT = atoi(nT_str);

    // No ongoing game
    if (check_ongoing_game(PLID) == 0) {
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);
    }

    // Checks if the game's maximum time has been exceeded
    int time_passed = 0;
    int remaining_time;
    int res_time = check_if_in_time(PLID, &time_passed, &remaining_time);

    char secret_key[KEY_SIZE + 1];

    // Maximum time has been surpassed -> game ends
    if (res_time == 2) {
        strcpy(status, "ETM");
        if (end_game(time_passed, PLID, secret_key, 'T', 0))
            return 1; // error
        // Sends secret key
        return send_end_message(OP_CODE, status, secret_key, message);

    // There is still game time left
    } else if (res_time == 0) {
        int nW, nB;

        // performs the trial operation
        int res_try = try_game(PLID, given_key, nT, time_passed, &nW, &nB);

        // Won game
        if (res_try == 0) {
            strcpy(status, "OK");
            if (end_game(time_passed, PLID, secret_key, 'W', nT)) 
                return 1;
            return send_try_message(OP_CODE, status, message, nT_str, nW, nB);

        // Error in trial
        } else if (res_try == 1) {
            return 1;
        
        // Successful trial
        } else if (res_try == 2) {
            strcpy(status, "OK");
            return send_try_message(OP_CODE, status, message, nT_str, nW, nB);

        // Duplicate trial
        } else if(res_try == 3) {
            strcpy(status, "DUP");
            return send_simple_message(OP_CODE, status, message);

        // Invalid trial
        } else if (res_try == 4) {
            strcpy(status, "INV");
            return send_simple_message(OP_CODE, status, message);
        
        // Player has no remaining tries -> game ends
        } else if (res_try == 5) {
            strcpy(status, "ENT");
            if (end_game(time_passed, PLID, secret_key, 'F', 0))
                return 1; // error
            // Sends secret key
            return send_end_message(OP_CODE, status, secret_key, message);
        }

    } else if (res_time == 1) return 1; //error

    return 0;
}

/*
try_game: performs a trial.

Returns:
 - 0: Correct guess (game won)
 - 1: Error
 - 2: Incorrect guess (but successful trial)
 - 3: Duplicate guess
 - 4: Invalid guess
 - 5: No more trials left

Arguments:
 - PLID: player ID
 - given_key: key guessed by player
 - nT: reported trial number
 - time_passed: how much game time has passed
 - nW: number of colors that are in secret key (incorrectly placed) (to be populated)
 - nB: number of correctly placed colors (to be populated)
*/
int try_game(char PLID[PLID_SIZE + 1], char given_key[KEY_SIZE + 1], int nT, int time_passed, int *nW, int *nB) {
    //TODO we are restoring the original path only when there is no error
    
    // Gets current directory
    char *path = getcwd(NULL, 0);
    int client_retry = 0;
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    int fd;

    // Opens player's current game
    if (open_active_game(PLID, &fd)) { 
        free(path);
        return 1;
    }

    // Checks if guess is repeated or invalid
    int res = check_repeated_or_invalid(fd, given_key, nT);
    
    // Error
    if (res == 1) {
        free(path);
        close(fd);
        return 1;
    
    // Duplicate guess
    } else if (res == 2) {
        // Changes back to original directory
        int dir = chdir(path);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to open original directory.\n");
            free(path);
            close(fd);
            return 1;
        }
        free(path);
        close(fd);
        return 3;
    
    // Invalid guess
    } else if (res == 3) {
        // Changes back to original directory
        int dir = chdir(path);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to open original directory.\n");
            free(path);
            close(fd);
            return 1;
        }
        free(path);
        close(fd);
        return 4;
    
    // TODO
    } else if (res == 4){
        client_retry = 1; // client might not have gotten the message
    }

    char secret_key[KEY_SIZE + 1];
    char playmode[2];

    // Gets relevant elements of game file header
    if (get_header_elements(secret_key, playmode, fd)) {
        free(path);
        close(fd);
        return 1;
    }

    // Calculates the correctness of the guessed key
    (*nB) = 0, (*nW) = 0;
    int secret_counts[6] = {0};
    int given_counts[6] = {0};
    for (int i = 0; i < 4; i++) {
        if (secret_key[i] == given_key[i])
            (*nB)++;
        else {
            secret_counts[colour_to_index(secret_key[i])]++;
            given_counts[colour_to_index(given_key[i])]++;
        }
    }

    for (int i = 0; i < 6; i++) {
        (*nW) += (secret_counts[i] < given_counts[i]) ? secret_counts[i] : given_counts[i];
    }

    int ret_value;
    
    // Won game
    if ((*nB) == 4) ret_value = 0;

    // No more trials left
    else if ((*nB) !=4 && nT == 8)
        ret_value = 5;

    // Successful trial (incorrect guess)
    else ret_value = 2;

    if (!client_retry) {
        char trial_str[40]; //TODO pensar no size disto
        sprintf(trial_str, "T: %s %d %d %d\n", given_key, *nB, *nW, time_passed);

        // Goes to the end of the game file
        if (lseek(fd, 0, SEEK_END) == -1) {
            fprintf(stderr, "Error: Failed to seek to the start of the file.\n");
            free(path);
            close(fd);
            return 1;
        }

        // Writes trial information
        ssize_t n = write(fd, trial_str, strlen(trial_str));
        if (n == -1) {
            fprintf(stderr, "Error: Failed to write trial in file.\n");
            free(path);
            close(fd);
            return 1;
        } //TODO
        
        close(fd); //TODO : e se client_retry = 1 ?
    }

    // Changes back to original directory
    int dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        free(path);
        return 1;
    }

    free(path); 
    return ret_value;
}

// MARK: SHOW_TRIALS

/*
show_trials_s: validates the arguments and performs operations
related to showing previous trials.

Returns 0 if the show trials was successful and 1 otherwise

Arguments:
 - args: the player message's arguments
 - message: reply message (to be populated)
 - n_args: number of arguments
*/
int show_trials_s(char **args, char **message, int n_args) {

    char OP_CODE[CODE_SIZE + 1] = "RST";

    char status[4];

    // Validates arguments
    if (n_args == 2) {
        if (args[1] == NULL || !is_valid_PLID(args[1])) {
            fprintf(stderr, "Error: invalid PLID.\n");
            strcpy(status, "NOK");
            return send_simple_message(OP_CODE, status, message);
        }
    } else {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);
    }

    // Extracts PLID
    char PLID[PLID_SIZE + 1];
    strncpy(PLID, args[1], PLID_SIZE);
    PLID[PLID_SIZE] = '\0';

    // Prepares filename
    time_t fulltime;
    char fname[25];
    sprintf(fname, "trials_%ld.txt", time(&fulltime));

    char *fdata = NULL;

    char key[KEY_SIZE + 1];

    // If player has an active game
    if (check_ongoing_game(PLID) == 2) {

        // Checks if game's max time has been exceeded
        int time_passed;
        int remaining_time;
        int res_time = check_if_in_time(PLID, &time_passed, &remaining_time);

        // Exceeded time -> ends game
        if (res_time == 2) {
            if (end_game(time_passed, PLID, key, 'T', 0)) { // WARNING: changed mode from Q to T
                return 1; // error
            }
        
        // Game still has time left
        } else if(res_time == 0){
            strcpy(status, "ACT");

            char file[strlen(GAMES_DIR) + 1 + 4 + 1 + PLID_SIZE + 4 + 1]; // WARNING: changed from this "strlen(GAMES_DIR) + 1 + PLID_SIZE + 4 + 1"
            sprintf(file, "%s/GAME_%s.txt", GAMES_DIR, PLID);

            // Assembles the previous trials (populates fdata)
            size_t fsize = assemble_fdata_st(file, &fdata, 1);
            if (fsize == 1) return 1; // error

            return send_data_message(OP_CODE, status, fname, fsize, fdata, message);
        }
    } 

    // Finds player's last game
    char file[strlen(GAMES_DIR) + 22];
    int res = find_last_game(PLID, file);

    // Player has no games
    if (res == 0) {
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);

    // Shows trials of player's last game
    } else {
        strcpy(status, "FIN");
        size_t fsize = assemble_fdata_st(file, &fdata, 0);
        if (fsize == 1) return 1;
        
        return send_data_message(OP_CODE, status, fname, fsize, fdata, message);
    }
}

/*
assembles_fdata_st: assembles previous trials.

Returns fsize if the assemble was successful and 1 otherwise

Arguments:
 - file: game filename
 - fdata: previous trials (to be populated)
 - act:flag that indicates if player has active game
*/
int assemble_fdata_st(char *file, char **fdata, int act) {

    char keys[8][5];
    char nB[8][2];
    char nW[8][2];

    int count = 0;

    int remaining_time;
    if (act) {
        // Gets PLID
        char PLID[PLID_SIZE + 1];
        char file_cpy[strlen(file)];
        strcpy(file_cpy, file);
        if (sscanf(file_cpy, "server/GAMES/GAME_%6s.txt", PLID) != 1) {
            fprintf(stderr, "Error: sscanf() failed.\n");
            return 1;
        }

        // Gets how much game time has passed
        int time_passed;
        if (check_if_in_time(PLID, &time_passed, &remaining_time)) {
            return 1;
        }
    }

    // Opens game's file
    FILE *fd = fopen(file, "r");
    if (fd == NULL) {
        fprintf(stderr, "Error: Failed to open %s file.\n", file);
        return 1;
    }

    // Gets all trial lines and counts them
    char line[LINE_SIZE_ST + 3];
    while (fgets(line, LINE_SIZE_ST + 2, fd)) {
        if (!strncmp(line, "T:", 2)) {
            if (sscanf(line, "T: %4s %1s %1s %*d\n", keys[count], nB[count], nW[count]) == 3)
                count++;
        }
        
    }

    if (act) 
        *fdata = (char*) malloc((LINE_SIZE_ST + 1) * count + sizeof(int) + 2);
    else 
        *fdata = (char*) malloc((LINE_SIZE_ST + 1) * count);
    
    if (*fdata == NULL) {
        fprintf(stderr, "Error: failed to allocate memory.\n");
        return 1;    
    }

    // populates fdata with previous trial's information and counts the data size
    char *ptr = *fdata;

    size_t fsize = 0;
    for (int i = 0; i < count; i++) {
        int written = sprintf(ptr, "%4s %1s %1s\n", keys[i], nB[i], nW[i]);
        ptr += written;
        fsize += written;
    }

    // if trials are of an active game, also writes to fdata how much game time there is left
    if (act) {
        int written = sprintf(ptr, "%d\n", remaining_time);
        ptr += written;
        fsize += written;
    }
    *ptr = '\0';
    return fsize;
}

/*
find_last_game: if player has no active games, finds their last game

Returns 0 if no games were found or the found game number

Arguments:
 - PLID: player ID
 - fname: filename (to be populated)
*/
int find_last_game(char PLID[PLID_SIZE + 1], char *fname) {
    
    struct dirent **filelist;
    int n_entries, found;
    char dirname[20];

    sprintf(dirname, "%s/%s", GAMES_DIR, PLID);

    n_entries = scandir(dirname, &filelist, 0, alphasort);

    found = 0;

    if (n_entries <= 0) return 0;
    else {
        while (n_entries--) {
            if (filelist[n_entries]->d_name[0] != '.' && !found) {
                sprintf(fname, "%s/%s", dirname, filelist[n_entries]->d_name);
                found = 1;
            }
            free(filelist[n_entries]);
        }
        free(filelist);
    }
    return found;
}

// MARK: SCOREBOARD

/*
scoreboard_s: validates the arguments and performs operations
related to showing scoreboard.

Returns 0 if the show scoreboard was successful and 1 otherwise

Arguments:
 - args: the player message's arguments
 - message: reply message (to be populated)
 - n_args: number of arguments
*/
int scoreboard_s(char **message) {
    
    char OP_CODE[CODE_SIZE + 1] = "RSS";

    char status[6];

    int scores[10];
    char PLIDs[10][PLID_SIZE + 1];
    char keys[10][KEY_SIZE + 1];
    int nTs[10];
    char modes[10][2];
    
    // Finds top 10 scores
    int res = find_top_scores(scores, PLIDs, keys, nTs, modes);

    // No games were founds
    if (res == 0) {
        strcpy(status, "EMPTY");
        status[5] = '\0';
        return send_simple_message(OP_CODE, status, message);
    }

    strcpy(status, "OK");
    status[2] = '\0';

    // Prepares filename
    time_t fulltime;
    char fname[25];
    sprintf(fname, "sb_%ld.txt", time(&fulltime));
    fname[24] = '\0';

    // Assembles the scoreboard
    char *fdata = NULL;
    size_t fsize = assemble_fdata_sb(&fdata, scores, PLIDs, keys, nTs, modes, res);
    if (fsize == 1)
        return 1; // error

    return send_data_message(OP_CODE, status, fname, fsize, fdata, message);
}

/*
assembles_fdata_sb: assembles scoreboard.

Returns fsize if the assemble was successful and 1 otherwise

Arguments:
 - fdata: scoreboard (to be populated)
 - scores: top 10 scores
 - PLIDs: player IDs with top 10 scores
 - keys: keys of top 10 games
 - nTs: number of trials of top 10 games
 - modes: play modes of top 10 games
 - res: games found (max 10)
*/
int assemble_fdata_sb(char **fdata, int scores[10], char PLIDs[10][PLID_SIZE + 1], char keys[10][KEY_SIZE + 1], int nTs[10], char modes[10][2], int res) {

    *fdata = (char*) malloc((LINE_SIZE_SB + 1) * res);
    if (*fdata == NULL) {
        fprintf(stderr, "Error: Failed to allocate fdata memory.\n");
        return 1;
    }

    char *ptr = *fdata;

    size_t fsize = 0;
    for (int i = 0; i < res; i++) {
        int written = sprintf(ptr, "%03d %6s %4s %1d %1s\n", scores[i], PLIDs[i], keys[i], nTs[i], modes[i]);
        ptr += written;
        fsize += written;
    }
    *ptr = '\0';

    return fsize;
}

int find_top_scores(int scores[10], char PLIDs[10][PLID_SIZE + 1], char keys[10][KEY_SIZE + 1], int nTs[10], char modes[10][2]) {
    struct dirent **filelist;
    int n_entries, i_file = 0;
    char fname[300];
    FILE *fp;

    n_entries = scandir(SCORES_DIR, &filelist, 0, alphasort);
    if (n_entries == 0) {
        return 0;
    }

    while (n_entries--) {
        if (filelist[n_entries]->d_name[0] != '.' && i_file < 10) {
            sprintf(fname, "%s/%s", SCORES_DIR, filelist[n_entries]->d_name);
            
            fp = fopen(fname, "r");
            if (fp != NULL) {
                if (fscanf(fp, "%d %s %s %d %s", 
                        &scores[i_file], 
                        PLIDs[i_file], 
                        keys[i_file], 
                        &nTs[i_file], 
                        modes[i_file]) == 5) {
                    ++i_file;
                }
                fclose(fp);
            }
        }
        free(filelist[n_entries]);
    }
    free(filelist);

    return i_file;
}

// MARK: QUIT

/*
quit_s: quits the game internally

Returns 0 if quit was successful and 1 otherwise

Arguments:
 - args: the player message's arguments
 - message: reply message (to be populated)
 - n_args: number of arguments
*/
int quit_s(char **args, char **message, int n_args) {
    
    char status[4];

    char OP_CODE[CODE_SIZE + 1] = "RQT";

    // Extracts PLID
    char PLID[PLID_SIZE + 1];
    if (n_args > 1) {
        if (args[1] != NULL) 
            strcpy(PLID, args[1]);
        else {
            strcpy(status, "ERR");
            return send_simple_message(OP_CODE, status, message);
        }

    // no PLID was sent (trial out of context)
    } else {
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);
    }

    PLID[PLID_SIZE] = '\0';

    // Validates PLID
    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }

    // Validates number of arguments
    if (n_args != 2) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }

    int res = check_ongoing_game(PLID);

    // Ongoing game
    if (res == 2) {
        char key[KEY_SIZE + 1];
        int time_passed;
        int remaining_time;
        int res_time = check_if_in_time(PLID, &time_passed, &remaining_time);

        // Game has exceeded maximum playtime
        if (res_time == 2) { 
            if (end_game(time_passed, PLID, key, 'T', 0)) { //WARNING: changed mode from "Q" to "T"
                return 1; // error
            }
            strcpy(status, "NOK"); 
            return send_simple_message(OP_CODE, status, message);
        }

        // Game in time -> ends game anyway
        if (end_game(time_passed, PLID, key, 'Q', 0)) {
            return 1; // error
        }

        strcpy(status, "OK");
                          
        return send_end_message(OP_CODE, status, key, message);

    // No ongoing game
    } else if (res == 0) {
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);

    } else if (res == 1) return 1; //error

    return 0;
}

// MARK: End Game

/*
end_game: ends game internally

Returns 0 if successul and 1 otherwise

Arguments:
 - time_passed: total game time
 - PLID: player ID
 - key: secret key (to be populated)
 - mode: end mode - [Q]uit; [W]in; [F]ail; [T]imeout
 - nT: number of trials
*/
int end_game(int time_passed, char PLID[PLID_SIZE + 1], char *key, char mode, int nT) {

    int src;

    // Gets current directory
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    // Gets player's current game file
    if (open_active_game(PLID, &src)) {
        free(path);
        close(src);
        return 1;
    }

    char playmode[2];

    // Gets relevant elements from game file header
    if (get_header_elements(key, playmode, src)) {
        free(path);
        close(src);
        return 1;
    }

    playmode[1] = '\0';

    if (lseek(src, 0, SEEK_END) == -1) {
        fprintf(stderr, "Error: Failed to seek to the start of the file.\n");
        free(path);
        close(src);
        return 1;
    }

    // Gets and formats game's end time
    time_t fulltime;
    struct tm *end_time;
    char time_str[DATE_SIZE + 1 + TIME_SIZE + 1];

    time(&fulltime);
    end_time = gmtime(&fulltime);
    sprintf (time_str, "%4d-%02d-%02d %02d:%02d:%02d %d\n",
            end_time->tm_year + 1900, end_time->tm_mon + 1, end_time->tm_mday,
            end_time->tm_hour, end_time->tm_min, end_time->tm_sec, time_passed);

    if (write(src, time_str, strlen(time_str) + 1) == -1) {
        fprintf(stderr, "Error: Failed to write end of game time.\n");
        free(path);
        close(src);
        return 1;
    }

    // Rewinds game file to the beginning
    if (lseek(src, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to seek to the start of the file.\n");
        free(path);
        close(src);
        return 1;
    }

    char fdata[BUFSIZ]; // TODO tamanho temporario

    // Reads game file and saves the data in fdata
    char *ptr_data = fdata;
    ssize_t n = read(src, ptr_data, BUFSIZ - 1); // leave space for null terminator
    if (n == -1) {
        free(path);
        close(src);
        return 1;
    }

    // Ensures all bytes are read
    ssize_t total_bytes_read = n;
    while (n != 0) {
        ptr_data += n;
        n = read(src, ptr_data, BUFSIZ - total_bytes_read - 1); // leave space for null terminator
        if (n == -1) {
            free(path);
            close(src);
            return 1;
        }
        total_bytes_read += n;
    }

    fdata[total_bytes_read] = '\0';

    close(src);

    // Gets active game filename
    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);
    unlink(filename); // removes active game

    // Opens player's (finished) games directory
    int dir = chdir(PLID);
    // Creates it if necessary
    if (ENOENT == errno) {
        if (mkdir(PLID, 0700) == -1) {
            fprintf(stderr, "Error: failed to create %s directory.\n", PLID);
            free(path);
            return 1;
        }
        dir = chdir(PLID);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to enter created %s directory.\n", PLID);
            free(path);
            return 1;
        }
    }

    // Prepares finished game filename
    char new_file[22]; 
    sprintf (new_file, "%4d%02d%02d_%02d%02d%02d_%c.txt",
            end_time->tm_year + 1900, end_time->tm_mon + 1, end_time->tm_mday,
            end_time->tm_hour, end_time->tm_min, end_time->tm_sec, mode);

    // Creates finished game file
    int dest = open(new_file, O_CREAT | O_RDWR, 0644);
    if (dest == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", new_file);
        free(path);
        close(dest);
        return 1;
    }

    // Writes fdata to the new finished game file
    char *ptr2 = fdata;
    n = write(dest, ptr2, total_bytes_read - 1);
    if (n == -1) {
        fprintf(stderr, "Error: write failed.\n");
        free(path);
        close(dest);
        return 1;
    }

    // Ensures all bytes are written
    ssize_t total_bytes_written = n;
    while (total_bytes_written < total_bytes_read - 1) {
        ptr2 += n;

        n = write(dest, ptr2, total_bytes_read - 1 - total_bytes_written);
        if (n == -1) {
            fprintf(stderr, "Error: write failed.\n");
            free(path);
            close(dest);
            return 1;
        }

        total_bytes_written += n;
    }

    close(dest);

    // Goes back to original directory
    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        return 1;
    }

    free(path);

    // If game was won, calculates its score and writes to SCORES directory
    if (mode == 'W'){
        int score = 999 - time_passed - (nT-1)*(399/8); // max score - time passed - (number of trials-1)*((max score- maxtime)/max number of trials)
        return write_to_scores(score, PLID, key, nT, playmode, end_time);        
    }

    return 0;
}

/*
write_to_scores: writes game info to SCORES directory

Returns 0 if successul and 1 otherwise

Arguments:
 - score: game score
 - PLID: player ID
 - key: secret key
 - nT: number of trials
 - playmode: game's playmode - [S]tart; [D]ebug
 - end_time: game's end time
*/
int write_to_scores(int score, char PLID[PLID_SIZE + 1], char key[KEY_SIZE + 1], int nT, char playmode[2], struct tm *end_time) {

    // Gets current directory
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }
    
    // Opens SCORES directory
    int dir = chdir(SCORES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        free(path);
        return 1;
    }

    // Prepares won game filename
    char new_file[31]; 
    sprintf(new_file, "%03d_%6s_%02d%02d%04d_%02d%02d%02d.txt",
            score, PLID, 
            end_time->tm_mday, end_time->tm_mon + 1,end_time->tm_year + 1900,
            end_time->tm_hour, end_time->tm_min, end_time->tm_sec);

    // Opens won game file
    int fd = open(new_file, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", new_file);
        free(path);
        close(fd);
        return 1;
    }

    // Writes to won game file relevant information about game
    char line[21];
    char *ptr = line;
    sprintf(line, "%03d %6s %4s %1d %1s\n", score, PLID, key, nT, playmode);

    ssize_t n = write(fd, ptr, 20);
    if (n == -1) {
        fprintf(stderr, "Error: write failed.\n");
        free(path);
        close(fd);
        return 1;
    }

    // Ensures all bytes are written
    while (n < 20) {
        ptr += n;
        n = write(fd, ptr, 20 - n);
        if (n == -1) {
            fprintf(stderr, "Error: write failed.\n");
            free(path);
            close(fd);
            return 1;
        }
    }

    close (fd);

    // Goes back to original directory
    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        free(path);
        return 1;
    }

    free(path);
    return 0;
}

// MARK: MESSAGES

/*
send_simple_message: populates message that is composed
of only OP_CODE and status

Returns 0 if successful and 1 otherwise

Arguments:
 - OP_CODE: protocol message code
 - status: status of the operation
 - messsage: reply message (to be populated)
*/
int send_simple_message(char OP_CODE[CODE_SIZE + 1], char *status, char **message) {

    int status_len = strlen(status);
    *message = (char *) malloc(3 + 1 + status_len + 2);
    if (*message == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }
    char *ptr = *message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, status, status_len);
    ptr += status_len;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0';

    return 0;
}

/*
send_try_message: populates message that is composed
of OP_CODE, status, trial number, nW and nB

Returns 0 if successful and 1 otherwise

Arguments:
 - OP_CODE: protocol message code
 - status: status of the operation
 - messsage: reply message (to be populated)
 - nT_str: number of trials
 - nW: number of colors that are in secret key (incorrectly placed) (to be populated)
 - nB: number of correctly placed colors (to be populated)
*/
int send_try_message(char OP_CODE[CODE_SIZE + 1], char status[4], char **message, char nT_str[2], int nW, int nB) {

    int status_len = strlen(status);
    *message = (char *) malloc(3 + 1 + strlen(status) + 1 + 3 + 2 + 2);
    if (*message == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }
    char *ptr = *message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, status, status_len);
    ptr += status_len;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, nT_str, 1);
    ptr += 1;

    memcpy(ptr, " ", 1);
    ptr += 1;

    char nB_str[2];
    sprintf(nB_str, "%d", nB);
    memcpy(ptr, nB_str, 1);
    ptr += 1;

    memcpy(ptr, " ", 1);
    ptr += 1;

    char nW_str[2];
    sprintf(nW_str, "%d", nW);
    memcpy(ptr, nW_str, 1);
    ptr += 1;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0';

    return 0;
}

/*
send_end_message: populates message that is composed
of OP_CODE, status and secret key

Returns 0 if successful and 1 otherwise

Arguments:
 - OP_CODE: protocol message code
 - status: status of the operation
 - messsage: reply message (to be populated)
 - key: secret key
*/
int send_end_message(char OP_CODE[CODE_SIZE + 1], char status[4], char key[KEY_SIZE + 1], char **message) {

    int status_len = strlen(status);
    *message = (char *) malloc(3 + 1 + strlen(status) + 1 + KEY_SIZE + 3 + 2);
    if (*message == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }

    char *ptr = *message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, status, status_len);
    ptr += status_len;

    memcpy(ptr, " ", 1);
    ptr += 1;

    for (int i = 0; i < 4; i++) {
        memcpy(ptr, &key[i], sizeof(char));
        ptr += 1;
        if (i < 3) {
            memcpy(ptr, " ", sizeof(char)); 
            ptr += 1;
        }
    }

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0';

    return 0;
}

/*
send_data_message: populates message that is composed
of OP_CODE, status, filename, file size and file data.

Returns 0 if successful and 1 otherwise

Arguments:
 - OP_CODE: protocol message code
 - status: status of the operation
 - fname: filename
 - fsize: file size
 - fdata: file data
 - messsage: reply message (to be populated)
*/
int send_data_message(char OP_CODE[CODE_SIZE + 1], char status[6], char fname[30], size_t fsize, char *fdata, char **message) {

    int status_len = strlen(status);
    int fname_len = strlen(fname);

    char fsize_str[5];
    sprintf(fsize_str, "%lu", fsize);
    int fsize_len = strlen(fsize_str);

    *message = (char *) malloc(CODE_SIZE + 1 + status_len + 1 + fname_len + 1 + fsize_len + fsize + 2);
    if (*message == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }
    char *ptr = *message;

    memcpy(ptr, OP_CODE, CODE_SIZE);
    ptr += CODE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, status, status_len);
    ptr += status_len;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, fname, fname_len);
    ptr += fname_len;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, fsize_str, fsize_len);
    ptr += fsize_len;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, fdata, fsize);
    ptr += fsize;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0';

    // frees allocated memory for fdata
    free(fdata);

    return 0;

}

// MARK: Extras

bool is_valid_PLID(const char PLID[PLID_SIZE + 1]) {
    if (strlen(PLID) != PLID_SIZE) return false;

    for (int i = 0; i < PLID_SIZE; i++) {
        if (!isdigit(PLID[i])) 
            return false;
    }

    return true;
}

bool is_valid_max_time(const char max_time_str[TIME_SIZE + 1], int len_max_time) {
    if (len_max_time > TIME_SIZE) return false;

    for (int i = 0; i < len_max_time; i++) {
        if (!isdigit(max_time_str[i])) return false;
    }

    int max_time = atoi(max_time_str);
    if (max_time <= 0 || max_time > 600) return false;
    else return true;
}

/*
get_header_elements: retrieves the secret key and 
playmode from the game file

Returns 0 if successful and 1 otherwise

Arguments:
 - key: secret key (to be populated)
 - playmode: game's playmode (to be populated)
 - fd: game file descriptor
*/
int get_header_elements(char *key, char *playmode, int fd) {

    // Ensure the file pointer is at the beginning
    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to rewind the game file.\n");
        return 1;
    }

    char header[HEADER_SIZE + 1];
    char *ptr = header;
    int n = read(fd, ptr, HEADER_SIZE); // leave space for null terminator
    if (n == -1) {
        fprintf(stderr, "Error: failed to read file.\n");
        return 1;
    }

    // Ensure all bytes are read
    ssize_t total_bytes_read = n;
    while (n != 0) {
        ptr += n;
        n = read(fd, ptr, HEADER_SIZE - total_bytes_read); // leave space for null terminator
        if (n == -1) {
            fprintf(stderr, "Error: failed to read file.\n");
            return 1;
        }
        total_bytes_read += n;
    }

    header[HEADER_SIZE] = '\0';

    if (sscanf(header, "%*s %1s %4s %*s %*s %*s %*s %*s", playmode, key) != 2) {
        fprintf(stderr, "Error: failed to scan header.\n");
        return 1;
    }

    return 0;
}

/*
open_active_game: opens the player's current game

Returns 0 if game file was successfully opened and
1 otherwise

Arguments:
 - PLID: player ID
 - fd: file descriptor (to be populated)
*/
int open_active_game(char PLID[PLID_SIZE], int *fd) {

    // Prepares the filename
    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);

    // Opens GAMES directory
    int dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        return 1;
    }

    // Opens current game file
    *fd = open(filename, O_RDWR);
    if (*fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", filename);
        return 1;
    }
    return 0; 
}

/*
check_ongoing_game: checks if a given player has an
active game

Returns 0 if there is no active game, 2 if there is
and 1 otherwise

Argument:
 - PLID: player ID
*/
int check_ongoing_game(const char PLID[PLID_SIZE + 1]) {

    // Gets current directory
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    // Opens GAMES directory
    int dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(path);
        return 1;
    }

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);

    int ret_value;

    // Ongoing game
    if (access(filename, F_OK) == 0) {
        ret_value = 2;
    
    } else {
        ret_value = 0;
    }

    // Changes back to original directory
    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open og directory.\n");
        free(path);
        return 1;
    }

    free(path);
    return ret_value;
}

/*
check_if_in_time: checks if game time has not surpassed
maximum playtime

Returns 0 if game is in time, 2 if maximum playtime has
been exceeded and 1 otherwise

Arguments:
 - PLID: player ID
 - time_passed: how much game time has passed (to be populated)
 - remaining_time: how much game time there is left (to be populated)
*/
int check_if_in_time(char PLID[PLID_SIZE + 1], int *time_passed, int *remaining_time) {
    
    int fd;

    // Gets current directory
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    // Opens player's current game
    if (open_active_game(PLID, &fd)) 
        return 1;
    
    // From the game file header, gets the start time
    char header[HEADER_SIZE + 1];
    char *ptr = header;
    int n = read(fd, ptr, HEADER_SIZE - 1); // leave space for null terminator
    if (n == -1) {
        fprintf(stderr, "Error: failed to read file.\n");
        free(path);
        close(fd);
        return 1;
    }

    // Ensures all bytes are read
    ssize_t total_bytes_read = n;
    while (n != 0) {
        ptr += n;
        n = read(fd, ptr, HEADER_SIZE - total_bytes_read - 1); // leave space for null terminator
        if (n == -1) {
            fprintf(stderr, "Error: failed to read file.\n");
            free(path);
            close(fd);
            return 1;
        }
        total_bytes_read += n;
    }

    char *header_safe = strtok(header, "\n");

    char max_time_str[FULLTIME_STR_SIZE];
    
    if (sscanf(header_safe, "%*s %*s %*s %3s %*s %*s %*s %*s", max_time_str) != 1) {
        fprintf(stderr, "Error: failed to scan header.\n");
        close(fd);
        free(path);
        return 1;
    }

    max_time_str[FULLTIME_STR_SIZE] = '\0';

    char *fulltime_str = strrchr(header_safe, ' '); //TODO se eu podia fazer no sscanf? sim, mas nao estava a funcionar e eu estava mad <3

    int max_time = atoi(max_time_str);
    long int fulltime = strtol(fulltime_str, NULL, 10);

    // Gets current time
    time_t current_time;
    time(&current_time);

    int res = 0;

    // Calculates how much time has passed and how much time there is left
    *time_passed = current_time - fulltime;
    *remaining_time = max_time - *time_passed;

    // Maximum playtime exceeded
    if (*remaining_time <= 0)
        res = 2;

    close(fd);

    // Goes back to original directory
    int dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open og directory.\n");
        free(path);
        return 1;
    }

    free(path);
    return res;
}

/*
check_repeated_or_invalid: checks if trial is repeated or invalid

Returns:
 - 0: Valid trial
 - 1: Error
 - 2: Repeated guess that wasn't the last one
 - 3: Invalid trial
 - 4: Repeated last guess

Arguments:
 - fd: game file descriptor
 - give_key: guessed key
 - nT: reported trial number
*/
int check_repeated_or_invalid(int fd, const char given_key[KEY_SIZE + 1], int nT) {
    char buffer[128];
    char previous_key[KEY_SIZE + 1] = {0};
    int last_trial_number = 0;
    int is_repeated = 0; 

    // Rewind the file descriptor to the beginning
    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to rewind the game file.\n");
        return 1;
    }

    // Ensure all bytes are read
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';

        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            // Checks previous trials
            if (strncmp(line, "T:", 2) == 0) { 
                sscanf(line, "T: %4s", previous_key);
                last_trial_number++;
                // Check for repeated guess
                if (strcmp(previous_key, given_key) == 0) {
                    is_repeated = 1;
                }
            }
            line = strtok(NULL, "\n"); // Move to the next line
        }
    }

    if (bytes_read == -1) {
        fprintf(stderr, "Error: Failed to read the game file.\n");
        return 1;
    }

    if (is_repeated) {
        if (nT == last_trial_number) { // TODO check relevance
            return 4;
        }
        return 2;
    }

    // Checks conditions for an invalid trial
    if (nT != last_trial_number + 1 && !(is_repeated && nT == last_trial_number)) {
        return 3;
    }

    return 0;
}

void generate_random_key(char *key) {
    srand(time(NULL));
    int num_colours = 6;
    for (int i = 0; i < KEY_SIZE; i++) {
        int random_index = rand() % num_colours;
        key[i] = COLOURS[random_index];
    }
    key[KEY_SIZE] = '\0';
}

/*
assemble_header: assembles game file header
with relevant information

Returns 0 if successful and 1 otherwise

Arguments:
 - header: game file header (to be populated)
 - PLID: player ID
 - max_time: maximum playtime (3 digits)
 - key: secret key
*/
int assemble_header(char *header, const char PLID[PLID_SIZE + 1], const char max_time[TIME_SIZE + 1], char key[KEY_SIZE + 1]) {

    char mode[2];

    if (key[0] == '\0') {
        generate_random_key(key);
        strcpy(mode, "P");
    } else {
        strcpy(mode, "D");
    }

    time_t fulltime;
    struct tm *current_time;
    char time_str[DATE_SIZE + 1];

    time(&fulltime);
    current_time = gmtime(&fulltime);
    sprintf (time_str, "%4d-%02d-%02d %02d:%02d:%02d",
            current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday,
            current_time->tm_hour, current_time->tm_min, current_time->tm_sec);

    char fulltime_str[FULLTIME_STR_SIZE];
    size_t fulltime_len = snprintf(fulltime_str, sizeof(fulltime_str), "%ld", (long)fulltime);
    if (fulltime_len >= sizeof(fulltime_str)) {
        fprintf(stderr, "Error: fulltime_str is too large.\n");
        return 1;
    }

    char *ptr = header;

    memcpy(ptr, PLID, PLID_SIZE);
    ptr += PLID_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, mode, 1);
    ptr += 1;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, key, KEY_SIZE);
    ptr += KEY_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, max_time, TIME_SIZE);
    ptr += TIME_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, time_str, DATE_SIZE);
    ptr += DATE_SIZE;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, fulltime_str, fulltime_len);
    ptr += fulltime_len;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0';

    return 0;

}

int colour_to_index(char c) {
    switch(c) {
        case 'R': return 0;
        case 'G': return 1;
        case 'B': return 2;
        case 'Y': return 3;
        case 'O': return 4;
        case 'P': return 5;
        default: return -1;
    }
}