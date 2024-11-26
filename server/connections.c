#include "connections.h"

int udp_connection(char *GSport, int VERBOSE) {
    
    int fd,errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET,SOCK_DGRAM,0); //UDP socket
    if(fd == -1) /*error*/return 1;

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; // IPv4
    hints.ai_socktype=SOCK_DGRAM; // UDP socket
    hints.ai_flags=AI_PASSIVE;

    errcode = getaddrinfo(NULL,GSport,&hints,&res);
    if(errcode != 0) /*error*/ return 1;

    n = bind(fd,res->ai_addr, res->ai_addrlen);
    if(n == -1) /*error*/ return 1;

    while (1) {
        addrlen=sizeof(addr);

        n=recvfrom(fd,buffer,128,0, (struct sockaddr*)&addr,&addrlen);
        if(n == -1)/*error*/return 1;

        write(1,"received: ",10);
        write(1,buffer,n);
        n = sendto(fd,buffer,n,0,(struct sockaddr*)&addr,addrlen);
        if(n == -1)/*error*/return 1;
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}

int tcp_connection(char *GSport, int VERBOSE) {
    int fd,errcode, newfd;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints,*res;
    struct sockaddr_in addr;
    char buffer[128];

    fd = socket(AF_INET,SOCK_STREAM,0); //TCP socket
    if (fd == -1) return 1; //error

    memset(&hints, 0, sizeof hints);
    hints.ai_family=AF_INET; //IPv4
    hints.ai_socktype=SOCK_STREAM; //TCP socket
    hints.ai_flags=AI_PASSIVE;

    errcode = getaddrinfo(NULL,GSport,&hints,&res);
    if((errcode)!= 0)/*error*/return 1;

    n=bind(fd,res->ai_addr,res->ai_addrlen);
    if(n == -1) /*error*/ return 1;

    if(listen(fd,5) == -1)/*error*/return 1;

    while(1) {
        addrlen = sizeof(addr);
        if((newfd = accept(fd, (struct sockaddr*) &addr, &addrlen)) == -1)
        /*error*/ return 1;
        n = read(newfd,buffer,128);
        if(n == -1)/*error*/return 1;

        write(1,"received: ",10);write(1,buffer,n);
        n = write(newfd, buffer, n);
        if(n == -1)/*error*/return 1;

        close(newfd);
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}