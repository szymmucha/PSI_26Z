#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    int int32_data;
    uint16_t uint16_data;
    uint16_t str_len;
    char *str_data;
} NodeData;

typedef struct {
    NodeData data;
    struct Node *next;
} Node;

int tcp_server_start(int port, int listen_queue_size); // Returns socket fd
int recv_exact(int clientfd, size_t expected_size, void* out_buffer); // Returns error code, buffer as parameter
#endif