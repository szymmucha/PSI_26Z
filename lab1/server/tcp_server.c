#include "tcp_server.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int tcp_server_start(int port, int listen_queue_size) {
    int sockfd;
    struct sockaddr_in server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error opening socket");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = port;

    if (bind(sockfd, (struct sockaddr*)&server, sizeof server) == -1) {
        perror("Error binding socket");
        return -1;
    }

    listen(sockfd, listen_queue_size);

    return sockfd;
}