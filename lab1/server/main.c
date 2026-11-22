#include "tcp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define LISTEN_QUEUE 5

int main(int argc, char* argv[]) {
    int port = argc >= 2 ? atoi(argv[1]) : DEFAULT_PORT;

    int serverfd = tcp_server_start(port, LISTEN_QUEUE);
    if (serverfd == -1) {
        return 1;
    }

    while (1) {
        int clientfd = accept(serverfd, (struct sockaddr*)0, (socklen_t*)0);
        if (clientfd == -1) {
            perror("Error accepting connection");
            continue;
        }

        message_t message;
        memset(&message, 0, sizeof(message));

        if (recv_message(clientfd, &message) == 0) {
            printf("Received message: %.*s (%lu bytes)\n", (int)message.size, (char*)message.data, message.size);
            fflush(stdout); // This is only for the printf to instantly appear, docker quirk
            free(message.data); // Has to be freed
        }

        close(clientfd);
    }

    close(serverfd);
    return 0;
}