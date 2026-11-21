#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdio.h>

typedef struct {
    void* data;
    size_t size;
} message_t;

int tcp_client_start(const char* server_ip, int port); // Returns socket fd
int send_all(int serverfd, size_t buffer_size, const void* buffer);
int send_message(int serverfd, const message_t* message); // Sends size and then content

#endif