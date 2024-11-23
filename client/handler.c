#include "handler.h"
#include "parser.h"
#include "connections.h"

void start_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded) {
    
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

    //printf("%s", message);

    start_r(GSIP, GSport, message);
}

void start_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return;
    }

    printf("messaged received in handler: %s", msg_received);
    
    int n_args = 2;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, n_args);
    /*for (int i = 0; i < n_args; i++) {
        printf("arg %d: %s\n", i, args[i]);
    }*/

    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Game started successfully.\n");
            //outra coisa eventualmente
        } 
        else if (args[1] != NULL && !strcmp(args[1], "NOK"))
            fprintf(stderr, "There is already an ongoing game.\n");
        // o stor tinha dito para fazermos a verificacao dos args na parte do cliente ou seja isto nunca acontecerá como eu fiz (?)
        else if (args[1] != NULL && !strcmp(args[1], "ERR"))
            fprintf(stderr, "Invalid syntax.\n");
        else 
            fprintf(stderr, "Unknown status: %s.\n", args[1]);

    } else if (res == 1)
        fprintf(stderr, "Error: memory allocation failed.\n");
    else if (res == 2)
        fprintf(stderr, "Error: received invalid message.\n");

    for (int i = 0; i < n_args; i++) {
        if (args[i] != NULL) free(args[i]);
    }
    free(args);

}

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
        memcpy(ptr, args[i], sizeof(char)); // N entendo pq fazias &args[i][0]
        ptr += 1;
        memcpy(ptr, " ", sizeof(char)); 
        ptr += 1;
    }

    char n_trials_str[TRIALS_SIZE + 1];
    sprintf(n_trials_str, "%d", n_trials);
    memcpy(ptr, n_trials_str, strlen(n_trials_str)); // N entendo pq fazias &n_trials_str[0]
    ptr += strlen(n_trials_str);

    memcpy(ptr, "\n", sizeof(char));
    ptr += 1;

    *ptr = '\0'; //ensures null termination

    printf("Message to send: '%s'\n", message); 

    return try_r(GSIP, GSport, message);
}

int try_r(char *GSIP, char *GSport, char *message) {

    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return 1;
    }

    printf("message received in handler: %s", msg_received);
  
    int max_n_args = 9;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, max_n_args);
    for (int i = 0; args[i] != NULL; i++) {
        printf("try arg %d: %s\n", i, args[i]);
    }

    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;
    
    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Valid trial.\n");
            if (args_counter == 5 && atoi(args[3]) == 4) 
                fprintf(stdout, "You won!\n"); //return something
            //if (condicao em que nao aumenta){frees and return !=0}
            for (int i = 0; args[i] != NULL; i++) free(args[i]);
            free(args);
            return 0;
        } 
        else if (args[1] != NULL && !strcmp(args[1], "DUP")) 
            fprintf(stdout, "Repeated guess.\n");
        else if (args[1] != NULL && !strcmp(args[1], "INV"))
            fprintf(stderr, "Não percebo este erro lol\n"); //TODO
        else if (args[1] != NULL && !strcmp(args[1], "NOK"))
            fprintf(stdout, "Trial is out of context.\n");
        else if (args[1] != NULL && !strcmp(args[1], "ENT")) {
            fprintf(stdout, "No more attempts available.\n");
            //reveal secret key and end game
        }
        else if (args[1] != NULL && !strcmp(args[1], "EMT")) {
            fprintf(stdout, "Maximum playtime has been exceeded.\n");
            //reveal secret key and end game
        }
        else if (args[1] != NULL && !strcmp(args[1], "ERR"))
            fprintf(stderr, "Invalid syntax.\n");
        else
            fprintf(stderr, "Unknown status: %s.\n", args[1]);

    } else if (res == 1)
        fprintf(stderr, "Error: memory allocation failed.\n");
    else if (res == 2 || (args_counter != 2 && args_counter != 5 && args_counter != 9))
        fprintf(stderr, "Error: received invalid message.\n");

    for (int i = 0; i < (args[i] != NULL); i++) {
        free(args[i]);
    }
    free(args);
    return 4; //codigo ainda nao usado
}

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

    printf("%s", message);
    //show_trials_r(GSIP, GSport, message);
}

/*void show_trials_r(char *GSIP, char *GSport, char *message) {

}*/

void show_sb_c(char *GSIP, char *GSport) {

    char OP_CODE[CODE_SIZE] = "SSB";

    char message[(CODE_SIZE + 1)*sizeof(char)];
    char *ptr = message;

    memcpy(ptr, OP_CODE, CODE_SIZE*sizeof(char));
    ptr += CODE_SIZE*sizeof(char);

    memcpy(ptr, "\n", sizeof(char));
    ptr += sizeof(char);

    printf("%s", message);
    //show_sb_r(GSIP, GSport, message);
}

/*void show_sb_r(char *GSIP, char *GSport, char *message) {

}*/

void quit_c(char *GSIP, char *GSport, char *PLID) {

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
    quit_r(GSIP, GSport, message);
}

void quit_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return;
    }

    printf("buffer received: %s\n", msg_received);
    
    int max_n_args = 6;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, max_n_args);
    /*for (int i = 0; i < n_args; i++) {
        printf("arg %d: %s\n", i, args[i]);
    }*/

    int args_counter = 0;
    while (args[args_counter] != NULL) args_counter++;

    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Game ended successfully.\n");
            //revelar secret key e sair do jogo
        } 
        else if (args[1] != NULL && !strcmp(args[1], "NOK")) 
            fprintf(stderr, "No ongoing game.\n");
        else if (args[1] != NULL && !strcmp(args[1], "ERR")) 
            fprintf(stderr, "Invalid syntax.\n");
        else fprintf(stderr, "Unknown status: %s.\n", args[1]);

    } else if (res == 1) 
        fprintf(stderr, "Error: memory allocation failed.\n");
    else if (res == 2 || (args_counter != 2 && args_counter != 6))
        fprintf(stderr, "Error: received invalid message.\n");

    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}

//void exit_c(char *GSIP, char *GSport) {}

void debug_c(char *GSIP, char *GSport, char *PLID, char *max_time_padded, char* args[7]){

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
    debug_r(GSIP, GSport, message);
}

void debug_r(char *GSIP, char *GSport, char *message) {
    
    char msg_received[128];
    if (udp_conn(GSIP, GSport, message, msg_received)) {
        fprintf(stderr, "Error: Couldn't connect to UDP.\n");
        return;
    }

    printf("buffer received: %s\n", msg_received);
    
    int n_args = 2;
    char **args = NULL;

    int res = deparse_buffer(msg_received, &args, n_args);
    /*for (int i = 0; i < n_args; i++) {
        printf("arg %d: %s\n", i, args[i]);
    }*/

    if (res == 0) {
        if (args[1] != NULL && !strcmp(args[1], "OK")) {
            fprintf(stdout, "Game started successfully.\n");
            //something
        } 
        else if (args[1] != NULL && !strcmp(args[1], "NOK")) 
            fprintf(stderr, "Already an ongoing game.\n");
        else if (args[1] != NULL && !strcmp(args[1], "ERR")) 
            fprintf(stderr, "Invalid syntax.\n");
        else
            fprintf(stderr, "Unknown status: %s.\n", args[1]);

    } else if (res == 1) 
        fprintf(stderr, "Error: memory allocation failed.\n");
    else if (res == 2)
        fprintf(stderr, "Error: received invalid message.\n");

    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}