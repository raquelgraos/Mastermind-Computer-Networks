#include "handler.h"
#include "gs_main.h"

bool is_valid_PLID(const char PLID[PLID_SIZE]) {
    if (strlen(PLID) != PLID_SIZE) return false;

    for (int i = 0; i < PLID_SIZE; i++) {
        if (!isdigit(PLID[i])) 
            return false;
    }

    return true;
}

bool is_valid_max_time(const char max_time_str[TIME_SIZE], int len_max_time) {
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

    char OP_CODE[CODE_SIZE] = "RSG";

    char status[4];
    if (n_args != 3) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    char PLID[PLID_SIZE];
    if (args[1] != NULL) 
        strcpy(PLID, args[1]);
    else {
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    //PLID[6] = '\0';

    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    fprintf(stderr, "PLID before: %s\n", PLID);
        
    char max_time[TIME_SIZE];
    if (args[2] != NULL) 
        strcpy(max_time, args[2]);
    else {
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    //max_time[3] = '\0';

    if (!is_valid_max_time(max_time, 3)) {
        fprintf(stderr, "Error: invalid max time.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message))
            return 1;
        else return 0;
    }

    int res = check_ongoing_game(PLID);
    fprintf(stderr, "PLI after: %s\n", PLID);
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

int start_game(const char PLID[PLID_SIZE], const char max_time[TIME_SIZE]) {
    
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

int send_start_message(char OP_CODE[CODE_SIZE], char status[4], char **message) {

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

    char OP_CODE[CODE_SIZE] = "RTR";

    char status[4];
    if (n_args != 7) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message)) //reusing start message since it's the same format
            return 1;
        else return 0;
    }

    char PLID[PLID_SIZE];
    if (args[1] != NULL) 
        strcpy(PLID, args[1]);
    else {
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message)) //reusing start message since it's the same format
            return 1;
        else return 0;
    }

    //PLID[6] = '\0';

    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message)) //reusing start message since it's the same format
            return 1;
        else return 0;
    }
        
    char max_time_str[TIME_SIZE];
    if (args[2] != NULL) 
        strcpy(max_time_str, args[2]);
    else {
        strcpy(status, "ERR");
        if (send_start_message(OP_CODE, status, message)) //reusing start message since it's the same format
            return 1;
        return 0;
    }

    int res_time = check_if_in_time(PLID);
    //int res_trial = check_if_trials(PLID); //TODO
    int res_trial = 0;
    if (res_time == 2 || res_trial == 2) { // if not in time or out of trials
        if (res_time == 2) strcpy(status, "ETM");
        else if (res_trial == 2) strcpy(status, "ENT");

        if (end_game_after_try())
            return 1; // error
        if (send_end_try_message(OP_CODE, status, PLID, message)) // sends secret key
            return 1; //error
        return 0;

    } else if (res_time == 0 && res_trial == 0) {
        //if (try_game())
        //    return 1;
        // if(send_try_message()) // manda aqueles n
        //    return 1; //error
        //else return 0;
    } else if (res_time == 1 || res_time == 1) return 1; //error

    return 0;
}

int end_game_after_try() {
    
    fprintf(stderr, "Ending game!\n");
    return 0;
}

int send_end_try_message(char OP_CODE[CODE_SIZE], char status[4], char PLID[PLID_SIZE], char **message) {

    int fd;
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    if (open_game_file(PLID, &fd, path)) 
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

    char key[KEY_SIZE];
    
    if (sscanf(header, "%*s %*s %4s %*s %*s %*s %*s %*s", key) != 1) {
        fprintf(stderr, "Error: failed to scan header.\n");
        close(fd);
        free(path);
        return 1;
    }

    //key[KEY_SIZE] = '\0';
    //fprintf(stderr, "%s\n", key);

    close(fd);
    int dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open og directory.\n");
        free(path);
        return 1;
    }    
    free(path);

    int status_len = strlen(status);
    *message = (char *) malloc(3 + 1 + strlen(status) + 1 + KEY_SIZE + 3 + 2);
    if (*message == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 1;
    }

    ptr = *message;

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

    fprintf(stderr, "%s", *message);

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

int open_game_file(char PLID[PLID_SIZE], int *fd, char *path) {

    int dir = chdir("server");
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open server directory.\n");
        free(path);
        return 1;
    }

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);

    dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(path);
        return 1;
    }

    *fd = open(filename, O_RDWR);
    if (*fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", filename);
        free(path);
        return 1;
    }
    return 0; 
}

int check_ongoing_game(const char PLID[PLID_SIZE]) {

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

int check_if_in_time(char PLID[PLID_SIZE]) {
    
    int fd;
    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 1;
    }

    if (open_game_file(PLID, &fd, path)) 
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

    //fprintf(stderr, "%s", header);

    char max_time_str[FULLTIME_STR_SIZE];
    
    if (sscanf(header, "%*s %*s %*s %3s %*s %*s %*s %*s", max_time_str) != 1) {
        fprintf(stderr, "Error: failed to scan header.\n");
        close(fd);
        free(path);
        return 1;
    }

    max_time_str[FULLTIME_STR_SIZE] = '\0';

    char *fulltime_str = strrchr(header, ' '); //TODO se eu podia fazer no sscanf? sim, mas nao estava a funcionar e eu estava mad <3

    //fprintf(stderr, "%s %s\n", max_time_str, fulltime_str);

    int max_time = atoi(max_time_str);
    long int fulltime = strtol(fulltime_str, NULL, 10);

    time_t current_time;
    time(&current_time);

    //fprintf(stderr, "%ld - %ld = %ld\n", current_time, fulltime, current_time - fulltime);
    //fprintf(stderr, "%d\n", max_time);
    int res = 0;

    if (current_time - fulltime >= max_time)
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

int assemble_header(char *header, const char PLID[PLID_SIZE], char *mode, const char max_time[TIME_SIZE]) {

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