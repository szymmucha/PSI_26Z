#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

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

int tcp_client_start(const char* server_ip, int port); // Returns socket fd
int send_all(int serverfd, size_t buffer_size, const void* buffer);

#endif