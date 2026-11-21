#include "tcp_client.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int tcp_client_start(const char* server_ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error opening socket");
        return -1;
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

    if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

int send_all(int serverfd, size_t buffer_size, const void* buffer) {
    size_t current_size = 0;

    while (current_size < buffer_size) {
        ssize_t size = send(serverfd, (const char*)buffer + current_size, buffer_size - current_size, 0);
        if (size == -1) {
            if (errno == EINTR) { // Interrupt - try again
                continue;
            }

            perror("Error sending data");
            return -1;
        }

        current_size += (size_t)size;
    }

    return 0;
}

int send_message(int serverfd, const message_t* message) {
    uint32_t message_size_net = htonl((uint32_t)message->size); // Network order

    if (send_all(serverfd, sizeof(message_size_net), &message_size_net) != 0) {
        return -1;
    }

    if (send_all(serverfd, message->size, message->data) != 0) {
        return -1;
    }

    return 0;
}