#include "tcp_client.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int udp_client_start(const char* server_ip, int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("opening datagram socket");
        exit(1);
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server.sin_addr) <= 0) {
        printf("Error converting IP to binary network format");
        close(sockfd);
        return -1;
    }

    //using connect to remember the server address
    if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("Error in UDP connect");
        close(sockfd);
        return -1;
    }

    return sock;
}

int udp_send(int sock, size_t buffer_size, const void* buffer) {

    ssize_t size;

    do {
        size = send(sock, (const char*)buffer, buffer_size ,0);
    } while(size == -1 && errno == EINTR);

    if (size == -1){
        perror("Error sending data");
        return -1;
    }

    if ((size_t)size != buffer_size){
        printf("The whole datagram was not sent");
        return -1;
    }
    return 0;
}