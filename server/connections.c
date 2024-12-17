#include "connections.h"
#include "parser.h"

volatile sig_atomic_t terminate = 0;

void handle_sigint(int sig) {
    (void)sig;
    terminate = 1;
}

int udp_connection(char *GSport, int VERBOSE) {
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char input[UDP_MAX_BUF_SIZE];
    fd_set readfds, testfds;
    struct timeval timeout;

    fd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (fd == -1) {
        fprintf(stderr, "Error: UDP socket failed.\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, GSport, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "Error: failed to get addrinfo.\n");
        return 1;
    }
    
    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error: failed to bind.\n");
        freeaddrinfo(res);
        close(fd);
        return 1;
    }

    if (res != NULL) freeaddrinfo(res);

    FD_ZERO(&readfds);         // Clear the set
    FD_SET(fd, &readfds);      // Add the socket descriptor to the set

    signal(SIGINT, handle_sigint);

    while (!terminate) {
        testfds = readfds; // Reload mask
        memset((void *)&timeout,0,sizeof(timeout));
        timeout.tv_sec=90; //TODO diminuir isto

        // Monitor the socket for incoming data
        int out_fds = select(FD_SETSIZE, &testfds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) &timeout);

        switch(out_fds) {
            case 0:
                printf("\n ---------------Timeout event-----------------\n");
                break; //what to do ?
            case -1:
                if (terminate) break;
                fprintf(stderr, "Error: select failed.\n");
                close(fd);
                return 1;
            default:
                if (FD_ISSET(fd, &testfds)) { // Check if the socket is ready
                    addrlen = sizeof(addr);

                    n = recvfrom(fd, input, UDP_MAX_BUF_SIZE, 0, (struct sockaddr *) &addr, &addrlen);
                    if (n == -1) {
                        fprintf(stderr, "Error: failed to receive message.\n");
                        close(fd);
                        return 1;
                    }
                    if(VERBOSE){
                        char *ip = inet_ntoa(addr.sin_addr);
                        int port = ntohs(addr.sin_port);
                        fprintf(stdout, "Player IP: %s\n", ip);
                        fprintf(stdout, "Player Port: %d\n", port);

                    }
    
                    input[n] = '\0';

                    char *message = NULL;
                    if (parse_input(input, &message, VERBOSE)) {
                        fprintf(stdout, "Failed to obtain message to send.\n");
                        close(fd);
                        if (message != NULL) free(message);
                        return 1;
                    }
                    
                    int msg_len = strlen(message);
                    n = sendto(fd, message, msg_len, 0, (struct sockaddr *)&addr, addrlen);
                    if (n == -1) {
                        fprintf(stderr, "Error: failed to send message.\n");
                        close(fd);
                        return 1;
                    }

                    free(message);
                }
        }
    }
    close(fd);
    return 0;
}


int tcp_connection(char *GSport, int VERBOSE) {
    int fd, errcode, newfd, ret;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char input[TCP_MAX_BUF_SIZE];
    fd_set readfds, testfds;
    struct timeval timeout;
    pid_t pid;
    act.sa_handler = SIG_IGN;

    if (sigaction(SIGCHLD, &act, NULL) == -1) {
        return 1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (fd == -1) {
        fprintf(stderr, "Error: TCP socket failed.\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;         // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP socket
    hints.ai_flags = AI_PASSIVE;

    errcode = getaddrinfo(NULL, GSport, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "Error: failed to get addrinfo.\n");
        close(fd);
        return 1;
    }

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        fprintf(stderr, "Error: TCP failed to bind.\n");
        freeaddrinfo(res);
        close(fd);
        return 1;
    }

    if (res != NULL) freeaddrinfo(res);

    if (listen(fd, 5) == -1) {
        fprintf(stderr, "Error: TCP failed to listen.\n");
        close(fd);
        return 1;
    }

    FD_ZERO(&readfds);         // Clear the set
    FD_SET(fd, &readfds);      // Add the socket descriptor to the set

    signal(SIGINT, handle_sigint);

    while (!terminate) {
        testfds = readfds; // Reload mask
        memset((void *)&timeout,0,sizeof(timeout));
        timeout.tv_sec=90; //TODO diminuir isto

        // Monitor the socket for incoming data
        int out_fds = select(FD_SETSIZE, &testfds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) &timeout);

        switch(out_fds) {
            case 0:
                printf("\n ---------------Timeout event-----------------\n");
                break; //what to do ?
            case -1:
                if (terminate) break;
                fprintf(stderr, "Error: select failed.\n");
                close(fd);
                return 1;
            default:
                if (FD_ISSET(fd, &testfds)) { // Check if the socket is ready
                    addrlen = sizeof(addr);
                    do newfd = accept(fd, (struct sockaddr*) &addr, &addrlen); // Wait for a connection
                    while (newfd == -1 && errno=EINTR);
                    if (newfd == -1) {
                        close(fd);
                        return 1;
                    }

                    if ((pid = fork()) == -1) {
                        close(fd);
                        return 1;
                    } else if (pid == 0) {
                        close(fd);

                        if (VERBOSE) {
                            char *ip = inet_ntoa(addr.sin_addr);
                            int port = ntohs(addr.sin_port);
                            fprintf(stdout, "Player IP: %s\n", ip);
                            fprintf(stdout, "Player Port: %d\n", port);

                        }

                        n = read(newfd, input, TCP_MAX_BUF_SIZE - 1);
                        if (n == -1) {
                            close(newfd);
                            close(fd);
                            return 1;
                        }
                        input[n] = '\0';
                        
                        char *message = NULL;
                        if (parse_input(input, &message, VERBOSE)) {
                            fprintf(stdout, "Failed to obtain message to send.\n");
                            close(fd);
                            if (message != NULL) free(message);
                            return 1;
                        }
                        
                        int msg_len = strlen(message);
                        n = write(newfd, message, msg_len);
                        int total_bytes_written = n;
                        if (n == -1) {
                            fprintf(stdout, "Error: failed to write.\n");
                            close(newfd);
                            close(fd);
                            if (message != NULL) free(message);
                            return 1;
                        }
                        while (total_bytes_written < msg_len) {
                            n = write(newfd, message, msg_len);
                            if (n == -1) {
                                fprintf(stdout, "Error: failed to write.\n");
                                close(newfd);
                                close(fd);
                                if (message != NULL) free(message);
                                return 1;
                            }
                            total_bytes_written += n;
                        }

                        close(newfd);

                    }
                    do ret = close(newfd);
                    while (ret == -1 && errno == EINTR);
                    if (ret == -1) {
                        close (fd);
                        return 1;
                    }
                }

        }
    
    }

    close(fd);
    return 0;
}