#include <stdio.h>

int tcp_server_start(int port, int listen_queue_size); // Returns socket fd
int recv_all(int clientfd, size_t expected_size, void* out_buffer); // Returns error code, buffer as parameter