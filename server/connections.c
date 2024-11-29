#include "connections.h"
#include "parser.h"

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

    while (1) {
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

                    //fprintf(stderr, "input n: %ld\n",n);
                    input[n] = '\0';

                    fprintf(stderr, "message received: %s\n", input);

                    char *message = NULL;
                    if (parse_input(input, &message)) {
                        fprintf(stdout, "Failed to obtain message to send.\n");
                        close(fd);
                        if (message != NULL) free(message);
                        return 1;
                    }

                    fprintf(stderr, "message to send: %s", message);
                    
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
    int fd,errcode, newfd;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char input[TCP_MAX_BUF_SIZE];

    fd = socket(AF_INET,SOCK_STREAM,0); //TCP socket
    if (fd == -1) return 1; //error

    memset(&hints, 0, sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_STREAM; //TCP socket
    hints.ai_flags=AI_PASSIVE;

    errcode = getaddrinfo(NULL,GSport,&hints,&res);
    if ((errcode)!= 0)/*error*/return 1;

    n=bind(fd,res->ai_addr,res->ai_addrlen);
    if (n == -1) /*error*/ return 1;

    if (listen(fd,5) == -1) /*error*/return 1;

    while (1) {
        addrlen = sizeof(addr);
        if((newfd = accept(fd, (struct sockaddr*) &addr, &addrlen)) == -1)
        /*error*/ return 1;
        n = read(newfd, input, 128);
        if (n == -1)/*error*/return 1;

        write(1,"received: ",10);write(1,input,n);
        n = write(newfd, input, n);
        if (n == -1)/*error*/return 1;

        close(newfd);
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}