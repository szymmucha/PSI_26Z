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

int recv_all(int clientfd, size_t expected_size, void* out_buffer) {
    size_t current_size = 0;

    while (current_size < expected_size) {
        ssize_t size = recv(clientfd, out_buffer + current_size, expected_size - current_size, 0);
        if (size <= 0) {
            return -1;
        }

        current_size += (size_t)size;
    }

    return 0;
}