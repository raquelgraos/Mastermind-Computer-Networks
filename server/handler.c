#include "handler.h"
#include "gs_main.h"

// MARK: START
int start_s(char **args, char **message, int n_args, char OP_CODE[CODE_SIZE + 1]) {
    char status[4];

    // validate number of arguments
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
    if (res == 0) {
        // ctart a new game
        if (start_game(PLID, max_time, key) == 0) {
            strcpy(status, "OK");
        } else {
            strcpy(status, "ERR");
        }
    } else if (res == 2) {
        strcpy(status, "NOK"); // ongoing game detected
    } else {
        return 1; // error in checking ongoing game
    }

    return send_simple_message(OP_CODE, status, message);
}

int start_game(const char PLID[PLID_SIZE + 1], const char max_time[TIME_SIZE + 1], char key[KEY_SIZE + 1]) {
    char *original_path = getcwd(NULL, 0);
    if (original_path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    // prepare the filename
    char filename[ONGOING_GAME_SIZE + 1];
    snprintf(filename, sizeof(filename), "GAME_%s.txt", PLID);

    // change to GAMES directory
    if (chdir(GAMES_DIR) != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(original_path);
        return 1;
    }

    // open or create the game file
    int fd = open(filename, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", filename);
        free(original_path);
        return 1;
    }

    // assemble the game header
    char header[HEADER_SIZE + 1];
    if (assemble_header(header, PLID, max_time, key)) {
        close(fd);
        free(original_path);
        return 1;
    }
    //fprintf(stderr, "%s", header);


    // write the header to the file
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
int try_s(char **args, char **message, int n_args) {

    char status[4];

    char OP_CODE[CODE_SIZE + 1] = "RTR";

    char PLID[PLID_SIZE + 1];
    if (n_args > 1) {
        if (args[1] != NULL) 
            strcpy(PLID, args[1]);
        else {
            strcpy(status, "ERR");
            return send_simple_message(OP_CODE, status, message);
        }
    } else {
        strcpy(status, "NOK"); // no PLID was sent (trial out of context)
        return send_simple_message(OP_CODE, status, message);
    }

    PLID[PLID_SIZE] = '\0';

    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }

    if (n_args != 7) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }
        
    char given_key[KEY_SIZE + 1];
    for (int i = 2; i < 2 + KEY_SIZE; i++) {
        if (args[i] != NULL) {
            given_key[i-2] = args[i][0];
        }
    }

    given_key[KEY_SIZE] = '\0';

    char nT_str[2];
    if (args[6] != NULL) strcpy(nT_str, args[6]);

    nT_str[1] = '\0';

    int nT = atoi(nT_str);

    if (check_ongoing_game(PLID) == 0) { // no ongoing game to play
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);
    }

    int time_passed = 0;
    int remaining_time;
    int res_time = check_if_in_time(PLID, &time_passed, &remaining_time);

    char secret_key[KEY_SIZE + 1];

    if (res_time == 2) {
        strcpy(status, "ETM");
        if (end_game(time_passed, PLID, secret_key, 'T', 0))
            return 1; // error
        if (send_end_message(OP_CODE, status, secret_key, message)) // sends secret key
            return 1; //error

        return 0;

    } else if (res_time == 0) {
        int nW, nB;
        int res_try = try_game(PLID, given_key, nT, time_passed, &nW, &nB);
        if (res_try == 0) { //won game
            strcpy(status, "OK");
            if (end_game(time_passed, PLID, secret_key, 'W', nT)) //falta implementar o scores
                return 1;
            if (send_try_message(OP_CODE, status, message, nT_str, nW, nB)) //TODO
                return 1;
            else return 0;

        } else if (res_try == 1) {
            return 1; //error
        } else if (res_try == 2) {
            strcpy(status, "OK");
            if (send_try_message(OP_CODE, status, message, nT_str, nW, nB)) //TODO
                return 1;
            else return 0;

        } else if(res_try == 3) {
            strcpy(status, "DUP");
            return send_simple_message(OP_CODE, status, message);

        } else if (res_try == 4) {
            strcpy(status, "INV");
            return send_simple_message(OP_CODE, status, message);
        } else if (res_try == 5) {
            strcpy(status, "ENT");
            if (end_game(time_passed, PLID, secret_key, 'F', 0))
                return 1; // error
            if (send_end_message(OP_CODE, status, secret_key, message)) // sends secret key
                return 1; //error

            return 0; 
        }

    } else if (res_time == 1) return 1; //error

    return 0;
}

int try_game(char PLID[PLID_SIZE + 1], char given_key[KEY_SIZE + 1], int nT, int time_passed, int *nW, int *nB) {
    // we are restoring the original path only when there is no error
    char *path = getcwd(NULL, 0);
    int client_retry = 0;
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    int fd;
    if (open_active_game(PLID, &fd)) { 
        free(path);
        return 1;
    }


    int res = check_repeated_or_invalid(fd, given_key, nT);
    if (res == 1) { // error
        free(path);
        close(fd);
        return 1;
    } else if (res==2){
        int dir = chdir(path);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to open original directory.\n");
            free(path);
            close(fd);
            return 1;
        }
        free(path);
        close(fd);
        return 3; // Status = "DUP"
    } else if (res==3){
        int dir = chdir(path);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to open original directory.\n");
            free(path);
            close(fd);
            return 1;
        }
        free(path);
        close(fd);
        return 4; // Status = "INV"
    } else if (res==4){
        client_retry = 1; // client might not have gotten the message
    }


    char secret_key[KEY_SIZE + 1];
    char playmode[2];

    if (get_header_elements(secret_key, playmode, fd)) {
        free(path);
        close(fd);
        return 1;
    }

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
    if ((*nB) == 4) ret_value = 0; // game won
    else if ((*nB) !=4 && nT == 8)
        ret_value = 5;
    else ret_value = 2;

    fprintf(stderr, "guessed key: %s\n", given_key);
    if (!client_retry){
       char trial_str[40]; //TODO pensar no size disto
        sprintf(trial_str, "T: %s %d %d %d\n", given_key, *nB, *nW, time_passed);

        if (lseek(fd, 0, SEEK_END) == -1) {
            fprintf(stderr, "Error: Failed to seek to the start of the file.\n");
            free(path);
            close(fd);
            return 1;
        }

        ssize_t n = write(fd, trial_str, strlen(trial_str));
        if (n == -1) {
            fprintf(stderr, "Error: Failed to write trial in file.\n");
            free(path);
            close(fd);
            return 1;
        }
        
        close(fd);
    }
    int dir = chdir(path);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to open original directory.\n");
            return 1;
        }
        free(path); 

    return ret_value;
}

int end_game(int time_passed, char PLID[PLID_SIZE + 1], char *key, char mode, int nT) {

    int src;
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    if (open_active_game(PLID, &src)) {
        free(path);
        close(src);
        return 1;
    }

    char playmode[2];

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

    if (lseek(src, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to seek to the start of the file.\n");
        free(path);
        close(src);
        return 1;
    }

    char fdata[BUFSIZ]; // TODO tamanho temporario
    char *ptr_data = fdata;
    ssize_t n = read(src, ptr_data, BUFSIZ - 1); // leave space for null terminator.
    if (n == -1) {
        free(path);
        close(src);
        return 1;
    }

    ssize_t total_bytes_read = n;
    while (n != 0) {
        ptr_data += n;
        n = read(src, ptr_data, BUFSIZ - total_bytes_read - 1); // leave space for null terminator.
        if (n == -1) {
            free(path);
            close(src);
            return 1;
        }
        total_bytes_read += n;
    }

    fdata[total_bytes_read] = '\0';

    //fprintf(stderr, "read: %s\n", fdata);

    close(src);

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);
    unlink(filename); // remove file

    int dir = chdir(PLID);
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

    char new_file[22]; 
    sprintf (new_file, "%4d%02d%02d_%02d%02d%02d_%c.txt",
            end_time->tm_year + 1900, end_time->tm_mon + 1, end_time->tm_mday,
            end_time->tm_hour, end_time->tm_min, end_time->tm_sec, mode);

    int dest = open(new_file, O_CREAT | O_RDWR, 0644);
    if (dest == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", new_file);
        free(path);
        close(dest);
        return 1;
    }

    char *ptr2 = fdata;
    n = write(dest, ptr2, total_bytes_read - 1);
    if (n == -1) {
        fprintf(stderr, "Error: write failed.\n");
        free(path);
        close(dest);
        return 1;
    }
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

    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        return 1;
    }
    free(path);

    if (mode == 'W'){
        //int score = calculate_score();
        fprintf(stderr, "Here!\n");
        int score = 54;
        write_to_scores(score, PLID, key, nT, playmode, end_time);        
    }

    return 0;
}

int write_to_scores(int score, char PLID[PLID_SIZE + 1], char key[KEY_SIZE + 1], int nT, char playmode[2], struct tm *end_time) {

    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    fprintf(stderr, "%s\n", path);
    
    int dir = chdir(SCORES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        free(path);
        return 1;
    }

    char new_file[31]; 
    sprintf(new_file, "%03d_%6s_%02d%02d%04d_%02d%02d%02d.txt",
            score, PLID, 
            end_time->tm_mday, end_time->tm_mon + 1,end_time->tm_year + 1900,
            end_time->tm_hour, end_time->tm_min, end_time->tm_sec);

    int fd = open(new_file, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", new_file);
        free(path);
        close(fd);
        return 1;
    }

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

    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        free(path);
        return 1;
    }

    free(path);
    return 0;
}

// MARK: SHOW_TRIALS
int show_trials_s(char **args, char **message, int n_args) {

    char OP_CODE[CODE_SIZE + 1] = "RST";

    char status[4];

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

    char PLID[PLID_SIZE + 1];
    strncpy(PLID, args[1], PLID_SIZE);

    time_t fulltime;
    char fname[30]; // TODO pensar melhor
    sprintf(fname, "trials_%ld.txt", time(&fulltime));

    char *fdata = NULL;
    if (check_ongoing_game(PLID) == 2) { // active game
        strcpy(status, "ACT");

        char file[strlen(GAMES_DIR) + 1 + PLID_SIZE + 4 + 1];
        sprintf(file, "%s/GAME_%s.txt", GAMES_DIR, PLID);

        size_t fsize = assemble_fdata_st(file, &fdata, 1);
        if (fsize == 1) return 1;

        return send_data_message(OP_CODE, status, fname, fsize, fdata, message);
    } 

    char file[strlen(GAMES_DIR) + 22];
    int res = find_last_game(PLID, file);

    if (res == 0) {
        strcpy(status, "NOK");
        return send_simple_message(OP_CODE, status, message);

    } else {
        strcpy(status, "FIN");
        size_t fsize = assemble_fdata_st(file, &fdata, 0);
        if (fsize == 1) return 1;
        
        return send_data_message(OP_CODE, status, fname, fsize, fdata, message);
    }
}

int assemble_fdata_st(char *file, char **fdata, int act) {

    char keys[8][5];
    char nB[8][2];
    char nW[8][2];

    int count = 0;

    int remaining_time;
    if (act) {
        char PLID[PLID_SIZE + 1];
        char file_cpy[strlen(file)];
        strcpy(file_cpy, file);
        if (sscanf(file_cpy, "server/GAMES/GAME_%6s.txt", PLID) != 1) {
            fprintf(stderr, "Error: sscanf() failed.\n");
            return 1;
        }

        int time_passed;
        if (check_if_in_time(PLID, &time_passed, &remaining_time) == 1) {
            return 1;
        }
    }

    FILE *fd = fopen(file, "r");
    if (fd == NULL) {
        fprintf(stderr, "Error: Failed to open %s file.\n", file);
        return 1;
    }

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

    char *ptr = *fdata;

    size_t fsize = 0;
    for (int i = 0; i < count; i++) {
        int written = sprintf(ptr, "%4s %1s %1s\n", keys[i], nB[i], nW[i]);
        ptr += written;
        fsize += written;
    }

    if (act) {
        int written = sprintf(ptr, "%d\n", remaining_time);
        ptr += written;
        fsize += written;
    }
    *ptr = '\0';
    fprintf(stderr, "fdata:\n%s", *fdata);
    return fsize;
}

int find_last_game(char PLID[PLID_SIZE + 1], char *fname) {
    
    struct dirent **filelist;
    int n_entries, found;
    char dirname[20];

    sprintf(dirname, "%s/%s/", GAMES_DIR, PLID);

    n_entries = scandir(dirname, &filelist, 0, alphasort);

    found = 0;

    if (n_entries <= 0) return 0;
    else {
        while (n_entries--) {
            if (filelist[n_entries]->d_name[0] != '.' && !found) {
                sprintf(fname, "%s%s", dirname, filelist[n_entries]->d_name);
                found = 1;
            }
            free(filelist[n_entries]);
        }
        free(filelist);
    }
    return found;
}

// MARK: SCOREBOARD
int scoreboard_s(char **args, char **message, int n_args) {
    
    char OP_CODE[CODE_SIZE + 1] = "RSS";

    char status[6];

    int scores[10];
    char PLIDs[10][PLID_SIZE + 1];
    char keys[10][KEY_SIZE + 1];
    int nTs[10];
    char modes[10][2];
    
    int res = find_top_scores(scores, PLIDs, keys, nTs, modes);
    if (res == 0) {
        strcpy(status, "EMPTY");
        status[5] = '\0';
        return send_simple_message(OP_CODE, status, message);
    }

    strcpy(status, "OK");
    status[2] = '\0';

    time_t fulltime;
    char fname[30]; // TODO pensar melhor
    sprintf(fname, "scoreboard_%ld.txt", time(&fulltime));

    char *fdata = NULL;
    size_t fsize = assemble_fdata_sb(&fdata, scores, PLIDs, keys, nTs, modes, res);
    if (fsize == 1)
        return 1;
    //fprintf(stderr, "\n%s", fdata);

    //fprintf(stderr, "fsize: %lu bytes\n", fsize);

    return send_data_message(OP_CODE, status, fname, fsize, fdata, message);
}

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
int quit_s(char **args, char **message, int n_args) {
    
    char status[4];

    char OP_CODE[CODE_SIZE + 1] = "RQT";

    char PLID[PLID_SIZE + 1];
    if (n_args > 1) {
        if (args[1] != NULL) 
            strcpy(PLID, args[1]);
        else {
            strcpy(status, "ERR");
            fprintf(stderr, "here plid\n");
            return send_simple_message(OP_CODE, status, message);
        }
    } else {
        strcpy(status, "NOK"); // no PLID was sent (trial out of context)
        return send_simple_message(OP_CODE, status, message);
    }

    PLID[PLID_SIZE] = '\0';

    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }

    if (n_args != 2) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        return send_simple_message(OP_CODE, status, message);
    }

    int res = check_ongoing_game(PLID);
    if (res == 2) { // ongoing game
        int time_passed;
        int remaining_time;
        int res_time = check_if_in_time(PLID, &time_passed, &remaining_time);
        /*if (res_time == 2) { // exceeded time 
            end_game()
        } */ //TODO se o tempo ja tiver terminado mostramos a solução na mesma ou so encerramos o jogo quietly?

        char key[KEY_SIZE + 1];
        if (end_game(time_passed, PLID, key, 'Q', 0)) {
            return 1; // error
        }

        strcpy(status, "OK");
                          
        if (send_end_message(OP_CODE, status, key, message)) {
            return 1;
        } else {
            return 0;
        }

    } else if (res == 0) {
        strcpy(status, "NOK"); // no ongoing game
        return send_simple_message(OP_CODE, status, message);

    } else if (res == 1) return 1; //error

    return 0;
}

// MARK: MESSAGES

int send_simple_message(char OP_CODE[CODE_SIZE + 1], char status[6], char **message) {

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

    //fprintf(stderr, "%s", *message);

    return 0;
}

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

int get_header_elements(char *key, char *playmode, int fd) {

    // ensure the file pointer is at the beginning
    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to rewind the game file.\n");
        return 1;
    }

    char header[HEADER_SIZE + 1];
    char *ptr = header;
    int n = read(fd, ptr, HEADER_SIZE); // leave space for null terminator.
    if (n == -1) {
        fprintf(stderr, "Error: failed to read file.\n");
        return 1;
    }

    ssize_t total_bytes_read = n;
    while (n != 0) {
        ptr += n;
        n = read(fd, ptr, HEADER_SIZE - total_bytes_read); // leave space for null terminator.
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

int open_active_game(char PLID[PLID_SIZE], int *fd) {

    char *path = getcwd(NULL, 0);
        if (path == NULL) {
            fprintf(stderr, "Error: getcwd failed.\n");
            return 1;
        }

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);

    int dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        return 1;
    }

    *fd = open(filename, O_RDWR);
    if (*fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", filename);
        return 1;
    }
    return 0; 
}

int check_ongoing_game(const char PLID[PLID_SIZE + 1]) {

    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    int dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(path);
        return 1;
    }

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);

    int ret_value;
    if (access(filename, F_OK) == 0) ret_value = 2; // ongoing game
    else ret_value = 0;

    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open og directory.\n");
        free(path);
        return 1;
    }    
    free(path);
    return ret_value;
}

int check_if_in_time(char PLID[PLID_SIZE + 1], int *time_passed, int *remaining_time) {
    
    int fd;
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    if (open_active_game(PLID, &fd)) 
        return 1;
    
    char header[HEADER_SIZE + 1];
    char *ptr = header;
    int n = read(fd, ptr, HEADER_SIZE - 1); // leave space for null terminator.
    if (n == -1) {
        fprintf(stderr, "Error: failed to read file.\n");
        free(path);
        close(fd);
        return 1;
    }
    ssize_t total_bytes_read = n;
    while (n != 0) {
        ptr += n;
        n = read(fd, ptr, HEADER_SIZE - total_bytes_read - 1); // leave space for null terminator.
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

    time_t current_time;
    time(&current_time);

    int res = 0;

    *time_passed = current_time - fulltime;
    *remaining_time = max_time - *time_passed;
    if (*remaining_time <= 0)
        res = 2; // max_time exceeded

    close(fd);

    int dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open og directory.\n");
        free(path);
        return 1;
    }    
    free(path);
    return res;
}

int check_repeated_or_invalid(int fd, const char given_key[KEY_SIZE + 1], int nT) {
    char buffer[128];
    char previous_key[KEY_SIZE + 1] = {0};
    int last_trial_number = 0;
    int is_repeated = 0; 

    // rewind the file descriptor to the beginning
    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "Error: Failed to rewind the game file.\n");
        return 1;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            if (strncmp(line, "T:", 2) == 0) { 
                sscanf(line, "T: %4s", previous_key);
                last_trial_number++;
                // check for repeated guess
                if (strcmp(previous_key, given_key) == 0) {
                    is_repeated = 1;
                }
            }
            line = strtok(NULL, "\n"); // move to the next line
        }
    }

    if (bytes_read == -1) {
        fprintf(stderr, "Error: Failed to read the game file.\n");
        return 1;
    }

    if (is_repeated) {
        if (nT == last_trial_number) {
            return 4; // repeated last guess
        }
        return 2; // repeated guess that wasn't the last one
    }

    if (nT != last_trial_number + 1 && !(is_repeated && nT == last_trial_number)) {
        return 3; // invalid trial
    }

    return 0; // valid trial
}

void generate_random_key(char *key) {
    int num_colours = 6;
    for (int i = 0; i < KEY_SIZE; i++) {
        int random_index = rand() % num_colours;
        key[i] = COLOURS[random_index];
    }
    key[KEY_SIZE] = '\0';
}

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
    int fulltime_len = snprintf(fulltime_str, sizeof(fulltime_str), "%ld", (long)fulltime);
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