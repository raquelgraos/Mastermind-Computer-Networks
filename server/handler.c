#include "handler.h"
#include "gs_main.h"

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

// MARK: START
int start_s(char **args, char **message, int n_args) {

    char OP_CODE[CODE_SIZE + 1] = "RSG";

    char status[4];
    if (n_args != 3) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    char PLID[PLID_SIZE + 1];
    if (args[1] != NULL) 
        strcpy(PLID, args[1]);
    else {
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    PLID[PLID_SIZE] = '\0';

    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    //fprintf(stderr, "PLID before: %s\n", PLID);
        
    char max_time[TIME_SIZE + 1];
    if (args[2] != NULL) 
        strcpy(max_time, args[2]);
    else {
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    max_time[TIME_SIZE] = '\0';

    if (!is_valid_max_time(max_time, 3)) {
        fprintf(stderr, "Error: invalid max time.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    int res = check_ongoing_game(PLID);
    //fprintf(stderr, "PLI after: %s\n", PLID);
    if (res == 0) {
        if (start_game(PLID, max_time) == 0) { // game started successfully
            strcpy(status, "OK");
            if (send_start_message(OP_CODE, status, message))
                return 1;
            else return 0;
        } else {
            strcpy(status, "ERR");
            if (send_start_message(OP_CODE, status, message))
                return 1;
            else return 0;
        }
    } else if (res == 2) {
        strcpy(status, "NOK"); // ongoing game already
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    } else if (res == 1) {
        return 1; //error
    }

    return 0;
}

int start_game(const char PLID[PLID_SIZE + 1], const char max_time[TIME_SIZE + 1]) {
    
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    int dir = chdir("server");
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open server directory.\n");
        free(path);
        return 1;
    }

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);
    filename[ONGOING_GAME_SIZE] = '\0';

    dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(path);
        return 1;
    }
    
    int fd = open(filename, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", filename);
        free(path);
        return 1;
    }

    char header[HEADER_SIZE + 1];
    if (assemble_header(header, PLID, "P", max_time)) {
        free(path);
        return 1;
    }
    //fprintf(stderr, "%s", header);

    char *ptr = header;
    ssize_t len = strlen(header);    
    ssize_t n = write(fd, ptr, len);
    if (n == -1) {
        fprintf(stderr, "Error: write failed.\n");
        free(path);
        return 1;
    }

    ssize_t total_bytes_written = n;
    while (total_bytes_written < len) {
        ptr += n;

        n = write(fd, ptr, len - total_bytes_written);
        if (n == -1) {
            fprintf(stderr, "Error: write failed.\n");
            free(path);
            return 1;
        }
        total_bytes_written += n;
    }

    close(fd);
    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        return 1;
    }
    free(path);
    return 0;
}

int send_start_message(char OP_CODE[CODE_SIZE + 1], char status[4], char **message) {

    int status_len = strlen(status);
    *message = (char *) malloc(3 + 1 + strlen(status) + 2);
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
            if (send_start_message(OP_CODE, status, message))
                return 1;
            else return 0;
        }
    } else {
        strcpy(status, "NOK"); // no PLID was sent (trial out of context)
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    PLID[PLID_SIZE] = '\0';

    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    if (n_args != 7) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
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
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    int time_passed = 0;
    int res_time = check_if_in_time(PLID, &time_passed);
    fprintf(stderr, "Time passed: %d\n", time_passed);
    //int res_trial = check_if_trials(PLID); //TODO -> devolve trials
    int res_trial = 0;
    char secret_key[KEY_SIZE + 1];

    if (res_time == 2) {
        strcpy(status, "ETM");
        if (end_game_after_try(time_passed, PLID, secret_key, 'T'))
            return 1; // error
        if (send_end_try_message(OP_CODE, status, secret_key, message)) // sends secret key
            return 1; //error

        return 0;

    } else if (res_time > 1) {
        strcpy(status, "ENT");
        if (end_game_after_try(time_passed, PLID, secret_key, 'F'))
            return 1; // error
        if (send_end_try_message(OP_CODE, status, secret_key, message)) // sends secret key
            return 1; //error

        return 0;

    } else if (res_time == 0 && res_trial == 0) { // in time and with trials left
        int nW, nB;
        int res_try = try_game(PLID, given_key, nT, res_trial, time_passed, &nW, &nB);
        if (res_try == 0) { //won game
            strcpy(status, "OK");
            if (end_game_after_try(time_passed, PLID, secret_key, 'W')) //falta implementar o scores
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
            if (send_start_message(OP_CODE, status, message))
                return 1;
            else return 0;

        } else if (res_try == 4) {
            strcpy(status, "INV");
            if (send_start_message(OP_CODE, status, message))
                return 1;
            else return 0;
        }
    } else if (res_time == 1 || res_time == 1) return 1; //error

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
    ptr += 1;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    *ptr = '\0';

    return 0;
}

int try_game(char PLID[PLID_SIZE + 1], char given_key[KEY_SIZE + 1], int nT, int res_trial, int time_passed, int *nW, int *nB) {

    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    int fd;
    if (open_active_game(PLID, &fd)) {
        free(path);
        return 1;
    }

    //int res_repeat = check_repeated_guess();
    int res_repeat = 2;
    if (res_repeat == 1) { //error
        free(path);
        close(fd);
        return 1;
    } else if (res_repeat == 0) { //repeated guess
        int dir = chdir(path);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to open original directory.\n");
            free(path);
            close(fd);
            return 1;
        }
        free(path);
        close(fd);
        return 3; // status = "DUP"
    }

    /*if (trial != res_trial) { // unexpected trial
        dir = chdir(path);
        if (dir != 0) {
            fprintf(stderr, "Error: failed to open original directory.\n");
            free(path);
            close(fd);
            return 1;
        }
        free(path);
        close(fd);
        return 4; // status = "INV"
    }*/

    char secret_key[KEY_SIZE + 1];
    if (get_secret_key(secret_key, fd)) {
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
    else ret_value = 2;

    fprintf(stderr, "given key: %s\n", given_key);

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

    int dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open original directory.\n");
        return 1;
    }
    free(path);

    return ret_value;
}

/*int check_repeated_guess() { //1 if error, 0 if repeated guess, 2 if new guess

}*/

int end_game_after_try(int time_passed, char PLID[PLID_SIZE + 1], char *key, char mode) {

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

    if (get_secret_key(key, src)) {
        free(path);
        close(src);
        return 1;
    }

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
    
    return 0;
}

int send_end_try_message(char OP_CODE[CODE_SIZE + 1], char status[4], char key[KEY_SIZE + 1], char **message) {

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

/*// MARK: SHOW_TRIALS
int show_trials_s(char **args, char **message, int n_args) {

}

// MARK: SCOREBOARD
int scoreboard_s(char **args, char **message, int n_args) {

}

// MARK: QUIT
int quit_s(char **args, char **message, int n_args) {

}

// MARK: DEBUG
int debug_s(char **args, char **message, int n_args) {

}*/

int get_secret_key(char *key, int fd) {

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
    
    if (sscanf(header, "%*s %*s %4s %*s %*s %*s %*s %*s", key) != 1) {
        fprintf(stderr, "Error: failed to scan header.\n");
        return 1;
    }

    return 0;
}

int open_active_game(char PLID[PLID_SIZE], int *fd) {

    int dir = chdir("server");
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open server directory.\n");
        return 1;
    }

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);

    dir = chdir(GAMES_DIR);
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

    int dir = chdir("server");
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open server directory.\n");
        free(path);
        return 1;
    }

    dir = chdir(GAMES_DIR);
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

int check_if_in_time(char PLID[PLID_SIZE + 1], int *time_passed) {
    
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
    if (*time_passed >= max_time)
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

void generate_random_key(char *key) {
    int num_colours = 6;
    for (int i = 0; i < KEY_SIZE; i++) {
        int random_index = rand() % num_colours;
        key[i] = COLOURS[random_index];
    }
    key[KEY_SIZE] = '\0';
}

int assemble_header(char *header, const char PLID[PLID_SIZE + 1], char *mode, const char max_time[TIME_SIZE + 1]) {

    char key[KEY_SIZE + 1];
    generate_random_key(key);

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