#include "tcp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>

#define DEFAULT_PORT 8080
#define LISTEN_QUEUE 5
#define BUFFER_SIZE 1024

static void free_list(Node *h) {
    while (h) { Node *n = h->next; free(h->data.str_data); free(h); h = n; }
}

static Node* read_node(int clientfd, uint8_t *buf)
{
    int res_reading = recv_exact(clientfd, 8, buf);
    if (res_reading == -1) {
        return NULL;
    }

    int32_t int32_data;
    uint16_t uint16_data;
    uint16_t str_len;
    memcpy(&int32_data, buf, 4);
    memcpy(&uint16_data, buf + 4, 2);
    memcpy(&str_len, buf + 6, 2);
    int32_data = ntohl(int32_data);
    uint16_data = ntohs(uint16_data);
    str_len = ntohs(str_len);

    if (str_len >= BUFFER_SIZE) {
        fprintf(stderr, "Error: string length %u exceeds buffer size\n", str_len);
        fflush(stderr);
        return NULL;
    }
            
    res_reading = recv_exact(clientfd, str_len, buf);
    if (res_reading == -1) {
        fprintf(stderr, "Error receiving string data from client\n");
        fflush(stderr);
        return NULL;
    }

    char *str_data = malloc(str_len + 1);
    memcpy(str_data, buf, str_len);
    str_data[str_len] = '\0';

    Node *node = malloc(sizeof(Node));
    node->data.int32_data = int32_data;
    node->data.uint16_data = uint16_data;
    node->data.str_len = str_len;
    node->data.str_data = str_data;
    node->next = NULL;
    return node;
}

static Node* read_message(int clientfd, uint8_t *buf)
{
    int res_reading_count = recv_exact(clientfd, 4, buf);
    if (res_reading_count == -1) {
        fprintf(stderr, "Error receiving count from client\n");
        fflush(stderr);
        return NULL;
    }

    uint32_t count;
    memcpy(&count, buf, 4);
    count = ntohl(count);
    printf("Receiving %d records from client\n", count);
    if (count > 1000) {
        fprintf(stderr, "Error: count %d exceeds maximum allowed\n", count);
        fflush(stderr);
        return NULL;
    }

    Node *head = NULL, *tail = NULL;
    
    for (int i = 0; i < count; i++)
    {
        Node *node = read_node(clientfd, buf);
        if (node == NULL) {
            free_list(head);
            return NULL;
        }

        if (head == NULL)
        {
            head = node;
        }
        else
        {
            tail->next = node;
        }
        tail = node;
    }
    return head;
}



int main(int argc, char* argv[]) {
    int port = argc >= 2 ? atoi(argv[1]) : DEFAULT_PORT;

    int serverfd = tcp_server_start(port, LISTEN_QUEUE);
    if (serverfd == -1) {
        return 1;
    }

    uint8_t *buf = malloc(1024);

    while (1) {
        int clientfd = accept(serverfd, (struct sockaddr*)0, (socklen_t*)0);
        if (clientfd == -1) {
            perror("Error accepting connection");
            continue;
        }
        printf("Accepted connection from client\n");
        fflush(stdout);
        Node *node = read_message(clientfd, buf);
        if (node == NULL) {
            fprintf(stderr, "Error reading message from client\n");
            fflush(stderr);
        } else {
            int c = 0;
            for (Node *n = node; n; n = n->next, c++)
            {
                printf("Received record[%d]: int32=%d uint16=%d str_len=%d str=\"%.*s\"\n", c,
                    n->data.int32_data, n->data.uint16_data, n->data.str_len,
                    (int)n->data.str_len, n->data.str_data);
            }
            fflush(stdout);
            free_list(node);
        }

        close(clientfd);
    }

    close(serverfd);
    return 0;
}