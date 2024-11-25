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

    printf("message sent: %s\n", message);

    n = sendto(fd, message, msg_len, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/ return 1; //exit(1);
    
    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer_aux, 128, 0, (struct sockaddr*) &addr, &addrlen);
    if (n == -1) /*error*/ return 1; //exit(1);

    char *token = strtok(buffer_aux, "\n"); // acho que isto e desnecessario
    strcat(token, "\n");
    printf("message received: %s\n", token);

    strcpy(buffer, token);
    freeaddrinfo(res);
    close(fd);
    return 0;
}

int tcp_conn(char *GSIP, char *GSport, char *message, char buffer[MAX_BUF_SIZE]) {

    int fd, errcode;
    ssize_t n;
    socklen_t addrlean;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == - 1) /*error*/ return 1; //exit(1);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_STREAM;     // TCP socket

    errcode = getaddrinfo(GSIP, GSport, &hints, &res);
    if (errcode != 0) /*error*/ return 1; //exit(1);

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) /*error*/ return 1; //exit(1);

    int msg_len = strlen(message);
    n = write(fd, message, msg_len);
    if (n == -1) /*error*/ return 1; //exit(1);

    printf("message sent: %s\n", message);

    char *buf_ptr = buffer;
    n = read(fd, buf_ptr, MAX_BUF_SIZE - 1); // leave space for null terminator.
    if (n == -1) /*error*/ return 1;

    ssize_t total_bytes_read = n;
    while (n != 0) {
        buf_ptr += n;
        n = read(fd, buf_ptr, MAX_BUF_SIZE - total_bytes_read - 1); // leave space for null terminator.
        if (n == -1) /*error*/ return 1;
        total_bytes_read += n;
    }

    buffer[total_bytes_read] = '\0';

    printf("message received: %s\n", buffer);
    freeaddrinfo(res);
    close(fd);
    return 0;
}