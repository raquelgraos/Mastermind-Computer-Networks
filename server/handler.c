#include "handler.h"
#include "gs_main.h"
#include "../client/parser.h" // nao funciona idk why

bool is_valid_PLID(char *PLID) {
    if (strlen(PLID) != PLID_SIZE) return false;

    for (int i = 0; i < PLID_SIZE; i++)
        if (!isdigit(PLID[i])) return false;
    return true;
}

bool is_valid_max_time(char *max_time_str, int len_max_time) {
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

    char status[4] = "";
    if (n_args != 3) {
        fprintf(stderr, "Error: invalid number of args.\n");
        strcpy(status, "ERR");
    }

    char PLID[7];
    if (args[1] != NULL) strcpy(PLID, args[1]);
    if (!is_valid_PLID(PLID)) {
        fprintf(stderr, "Error: invalid PLID.\n");
        strcpy(status, "ERR");
    }
    
    char max_time[4];
    if (args[2] != NULL) strcpy(max_time, args[2]);

    if (!is_valid_max_time(max_time, 3)) {
        fprintf(stderr, "Error: invalid max time.\n");
        strcpy(status, "ERR");
    }
    
    int res = check_ongoing_game(PLID);
    if (res == 0) {
        if (start_game(PLID, max_time) == 0) // game started successfully
            strcpy(status, "OK");
        else 
            strcpy(status, "ERR");
    } else if (res == 1) {
        strcpy(status, "NOK"); // ongoing game already
    } else if (res == 2) {
        strcpy(status, "ERR");
    }
    
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

int start_game(char *PLID, char *max_time) { //TODO 's' no header
    
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
    fprintf(stderr, "PLID: %s\n", PLID);
    sprintf(filename, "GAME_%s.txt", PLID);

    dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(path);
        return 1;
    }
    
    int fd = open(filename, O_CREAT | O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "Error: failed to open %s file.\n", filename);
        free(path);
        return 1;
    }

    char header[HEADER_SIZE + 1];
    assemble_header(header, PLID, "P", max_time);

    char *ptr = header;    
    ssize_t n = write(fd, ptr, HEADER_SIZE + 1);
    if (n == -1) {
        fprintf(stderr, "Error: write failed.\n");
        free(path);
        return 1;
    }

    ssize_t total_bytes_written = n;
    while (total_bytes_written < HEADER_SIZE + 1) {
        ptr += n;

        n = write(fd, ptr, HEADER_SIZE + 1 - total_bytes_written);
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

/*// MARK: TRY
int try_s(char **args, char **message, int n_args) {

}

// MARK: SHOW_TRIALS
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

int check_ongoing_game(char *PLID) {

    struct dirent **filelist;

    char *path = getcwd(NULL, 0);
    if (path == NULL) {
        fprintf(stderr, "Error: getcwd failed.\n");
        return 2;
    }

    int dir = chdir("server");
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open server directory.\n");
        free(path);
        return 2;
    }

    dir = chdir(GAMES_DIR);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open GAMES directory.\n");
        free(path);
        return 2;
    }

    char filename[ONGOING_GAME_SIZE + 1];
    sprintf(filename, "GAME_%s.txt", PLID);

    int n_entries = scandir(filename, &filelist, 0, alphasort);

    int ret_value;
    if (n_entries == 1) ret_value = 1;
    else ret_value = 0; // no ongoing game

    dir = chdir(path);
    if (dir != 0) {
        fprintf(stderr, "Error: failed to open og directory.\n");
        free(path);
        return 2;
    }    
    free(path);
    return ret_value;
}

void generate_random_key(char *key) {
    int num_colours = 6;
    for (int i = 0; i < KEY_SIZE; i++) {
        int random_index = rand() % num_colours;
        key[i] = COLOURS[random_index];
    }
    key[KEY_SIZE] = '\0';
}

void assemble_header(char *header, char *PLID, char *mode, char *max_time) {
    
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
    ptr += 1;

    memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, time_str, DATE_SIZE);
    ptr += DATE_SIZE;

    memcpy(ptr, "\n", 1);
    ptr += 1;

    /*memcpy(ptr, " ", 1);
    ptr += 1;

    memcpy(ptr, )*/

    //TODO falta o 's'

    *ptr = '\0';

}