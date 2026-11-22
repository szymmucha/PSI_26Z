#include "tcp_client.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_IP "172.21.31.2"
#define DEFAULT_PORT 8080

#define DUMMY_MESSAGE "DUMMY MESSAGE"

int main(int argc, char* argv[]) {
    const char* server_ip = argc >= 2 ? argv[1] : DEFAULT_IP;
    int port = argc >= 3 ? atoi(argv[2]) : DEFAULT_PORT;

    int serverfd = tcp_client_start(server_ip, port);
    if (serverfd == -1) {
        return 1;
    }

    message_t message = {
        .data = DUMMY_MESSAGE,
        .size = strlen(DUMMY_MESSAGE)
    };

    if (send_message(serverfd, &message) == 0) {
        printf("Sent message: %s (%lu bytes)\n", (char*)message.data, message.size);
    }
    else {
        printf("Failed to send message\n");
    }

    close(serverfd);

    return 0;
}