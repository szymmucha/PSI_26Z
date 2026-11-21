#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdio.h>

typedef struct {
    void* data;
    size_t size;
} message_t;

int tcp_server_start(int port, int listen_queue_size); // Returns socket fd
int recv_all(int clientfd, size_t expected_size, void* out_buffer); // Returns error code, buffer as parameter
int recv_message(int clientfd, message_t* out_message); // Receives length and content of incoming data

#endif