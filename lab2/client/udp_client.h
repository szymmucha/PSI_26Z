#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

int udp_client_start(const char* server_ip, int port);
int udp_send(int sock, size_t buffer_size, const void* buffer);

#endif