#include "handler.h"

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

    printf("%s", message);
    
    return 0; // success
}

int try_c(char *GSIP, char *GSport, char *PLID, char *args[5], int n_trials) {
    
    char OP_CODE[CODE_SIZE] = "TRY";

    char message[(CODE_SIZE + PLID_SIZE + COLOURS_SIZE + TRIALS_SIZE + 7)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, PLID, PLID_SIZE*sizeof(char));
    ptr += PLID_SIZE*sizeof(char);

    memcpy(ptr, " ", sizeof(char));
    ptr += sizeof(char);

    for (int i = 1; i < 5; i++) {
        memcpy(ptr, &args[i][0], sizeof(char));
        ptr += sizeof(char);
        memcpy(ptr, " ", sizeof(char));
        ptr += sizeof(char);
    }

    char n_trials_str[TRIALS_SIZE + 1];
    sprintf(n_trials_str, "%d", n_trials);
    memcpy(ptr, &n_trials_str[0], sizeof(char));
    ptr += sizeof(char);

    memcpy(ptr, "\n", sizeof(char));
    ptr += sizeof(char);

    printf("%s", message);
    return 0; // success
}

int show_trials_c(char *GSIP, char *GSport, char *PLID) {

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

    printf("%s", message);
    
    return 0; // success
}

int show_sb_c(char *GSIP, char *GSport) {

    char OP_CODE[CODE_SIZE] = "SSB";

    char message[(CODE_SIZE + 1)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, "\n", sizeof(char));
    ptr += sizeof(char);

    printf("%s", message);
    return 0; // success
}

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

    printf("%s", message);
    return 0; // success
}

int exit_c(char *GSIP, char *GSport) {

    return 0; // success
}

int debug_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded, char* args[7]){

    char OP_CODE[CODE_SIZE] = "STR";

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

    printf("%s", message);
    return 0; // success
}