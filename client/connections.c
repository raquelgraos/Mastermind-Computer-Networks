#include "connections.h"

/*
udp_conn: establishes a UDP connection to the GS, sends a message
and retrieves a reply.

Returns 0 if successful, 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: message to be sent to the GS
 - buffer: reply message from the GS
 */
int udp_conn(char *GSIP, char *GSport, char *message, char buffer[128]) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer_aux[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        fprintf(stderr,"Socket creation failed\n");
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          
    hints.ai_socktype = SOCK_DGRAM;     

    errcode = getaddrinfo(GSIP, GSport, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(fd);
        return 1;
    }

    struct timeval timeout;
    timeout.tv_sec = 5; 
    timeout.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "setsockopt failed\n");
        freeaddrinfo(res);
        close(fd);
        return 1;
    }

    int retries = 3; 
    int received = 0;

    for (int attempt = 1; attempt <= retries; ++attempt) {

        n = sendto(fd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);
        if (n == -1) {
            fprintf(stderr, "Send failed\n");
            continue;
        }

        addrlen = sizeof(addr);
        n = recvfrom(fd, buffer_aux, sizeof(buffer_aux) - 1, 0, (struct sockaddr *)&addr, &addrlen);
        if (n < 0) {
            fprintf(stdout, "Couldn't receive message from server. Retrying...\n");
        } else {
            buffer_aux[n] = '\0';
            received = 1;
            break;
        }
    }

    if (!received) {
        fprintf(stdout,"Failed to receive message after %d attempts.\n", retries);
        freeaddrinfo(res);
        close(fd);
        return 1;
    }

    strncpy(buffer, buffer_aux, 128);
    buffer[127] = '\0';  

    freeaddrinfo(res);
    close(fd);
    return 0;
}

/*
tcp_conn: establishes a UDP connection to the GS, sends a message
and retrieves a reply.

Returns 0 if successful, 1 otherwise

Arguments:
 - GSIP: IP address of the machine where the GS runs
 - GSport: port where the GS accepts requests
 - message: message to be sent to the GS
 - buffer: reply message from the GS
 */
int tcp_conn(char *GSIP, char *GSport, char *message, char buffer[MAX_BUF_SIZE]) {

    int fd, errcode;
    ssize_t n;
    socklen_t addrlean;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == - 1) return 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_STREAM;     // TCP socket

    errcode = getaddrinfo(GSIP, GSport, &hints, &res);
    if (errcode != 0) return 1;

    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) return 1;

    int msg_len = strlen(message);
    n = write(fd, message, msg_len);
    if (n == -1) return 1;

    // ensures all bytes are written
    ssize_t total_bytes_written = n;
    while (total_bytes_written < msg_len) {
        n = write(fd, message, msg_len - total_bytes_written);
        if (n == -1) return 1;
        total_bytes_written += n;
    }

    char *buf_ptr = buffer;
    n = read(fd, buf_ptr, MAX_BUF_SIZE - 1); // leave space for null terminator
    if (n == -1) return 1;

    // ensures all bytes are read
    ssize_t total_bytes_read = n;
    while (n != 0) {
        buf_ptr += n;
        n = read(fd, buf_ptr, MAX_BUF_SIZE - total_bytes_read - 1); // leave space for null terminator
        if (n == -1) return 1;
        total_bytes_read += n;
    }

    buffer[total_bytes_read] = '\0';

    freeaddrinfo(res);
    close(fd);
    return 0;
}