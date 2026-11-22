#include "tcp_client.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_IP "172.21.31.2"
#define DEFAULT_PORT 8080

void print_bytes(const uint8_t *buf, size_t n) {
    for (size_t i = 0; i < n; ++i) printf("%02X ", buf[i]);
    printf("\n");
}

static Node *list_to_send(void)
{
    printf("Preparing list to send to server\n");
    fflush(stdout);
    Node *head = NULL, *tail = NULL;

    for (size_t i = 0; i < 10; i++)
    {
        printf("Creating element %zu\n", i);
        fflush(stdout);
        Node *node = malloc(sizeof(Node));
        node->next = NULL;
        node->data.int32_data = rand();
        if (i % 2 == 1)
            node->data.int32_data *= -1;
        node->data.uint16_data = rand() % 65536;

        char *str_buf = malloc(64);
        int n = rand() % 60 + 1;
        memset(str_buf, 'a' + i, n);
        str_buf[n] = '\0';
        node->data.str_data = str_buf;
        node->data.str_len = strlen(node->data.str_data);
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

static void free_list(Node *h) {
    while (h) { Node *n = h->next; free(h->data.str_data); free(h); h = n; }
}

static int send_list(int serverfd, Node *head)
{
    printf("Preparing to send list to server\n");
    fflush(stdout);
    uint32_t count = 0;
    int buffer_size = sizeof(int);
    for (Node *n = head; n; n = n->next, count++)
    {
        buffer_size += 2 + 4 + 2 + n->data.str_len;
    }

    uint8_t *buf = malloc(buffer_size);
    uint8_t *buf_start = buf;
    const tmp_count = htonl(count);
    memcpy(buf, &tmp_count, 4);
    buf += 4;
    for (Node *n = head; n; n = n->next)
    {
        printf("Saving to buffer record: int32=%d uint16=%d str_len=%d str=\"%.*s\"\n",
               n->data.int32_data, n->data.uint16_data, n->data.str_len,
               (int)n->data.str_len, n->data.str_data);
        fflush(stdout);
        const tmp_int32 = htonl(n->data.int32_data);
        const tmp_uint16 = htons(n->data.uint16_data);
        const tmp_str_len = htons(n->data.str_len);
        memcpy(buf, &tmp_int32, 4);
        buf += 4;
        memcpy(buf, &tmp_uint16, 2);
        buf += 2;
        memcpy(buf, &tmp_str_len, 2);
        buf += 2;
        memcpy(buf, n->data.str_data, n->data.str_len);
        buf += n->data.str_len;
    }
    printf("Sending %d records to server. Buffer size: %d \n", count, buffer_size);
    print_bytes(buf_start, buffer_size);
    fflush(stdout);
    int res = send_all(serverfd, buffer_size, buf_start);
    free(buf_start);
    return res;
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));

    const char* server_ip = argc >= 2 ? argv[1] : DEFAULT_IP;
    int port = argc >= 3 ? atoi(argv[2]) : DEFAULT_PORT;

    int serverfd = tcp_client_start(server_ip, port);
    if (serverfd == -1) {
        return 1;
    }

    printf("Connected to server %s:%d\n", server_ip, port);
    fflush(stdout);

    Node *list = list_to_send();
    const int res = send_list(serverfd, list);
    printf("Sent list to server\n");
    fflush(stdout);
    close(serverfd);
    free_list(list);

    return 0;
}

