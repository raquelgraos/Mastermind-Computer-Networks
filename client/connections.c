#include "connections.h"

int udp_conn(char *GSIP, char *GSport, char *message, char buffer[128]) {

    int fd,errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer_aux[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (fd == - 1) /*error*/ return 1; //exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP socket

    errcode = getaddrinfo(GSIP, GSport, &hints, &res);
    if (errcode != 0) /*error*/ return 1; //exit(1);

    int msg_len = strlen(message);

    printf("message sent: %s", message);

    n = sendto(fd, message, msg_len, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/ return 1; //exit(1);

    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer_aux, 128, 0, (struct sockaddr*) &addr, &addrlen);
    if (n == -1) /*error*/ return 1; //exit(1);
    
    char *token = strtok(buffer_aux, "\n"); // acho que isto e desnecessario
    strcat(token, "\n");
    printf("message received in udp: %s", token);

    strcpy(buffer, token);
    freeaddrinfo(res);
    close(fd);
    return 0;
}

/*int tcp_conn(char *GSIP, char *GSport, char *message, char *args) {

    int fd, errcode;
    ssize_t n;
    socklen_t addrlean;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (fd == - 1) error return 1; //exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP socket

    errcode = getaddrinfo(GSIP, GSport, &hints, &res);
    if (errcode != 0) error return 1; //exit(1);

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) error return 1; //exit(1);

    int msg_len = strlen(message);
    n = write(fd, message, msg_len);
    if (n == -1) error return 1; //exit(1);

    n = read(fd, buffer, 128);
    if (n == -1) error return 1; //exit(1);

    deparse_buffer(buffer, args);
    freeaddrinfo(res);
    close(fd);
    return 0;
}*/