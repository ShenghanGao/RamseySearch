#include <stdbool.h>
#ifndef CLIENT_TRANSFER_H
#define CLIENT_TRANSFER_H

#define BUFFER_SIZE 1024
#define PORT 8000

int send_file();

void send_request(char *ip_addr);

void send_check(char* ip_addr);

void receive_file(int connected_socket);

void set_port();

void *client_always_listen_to_one_handler();

int create_connection(char *ip_addr);

extern bool recv_flag;
extern char *ip_addr;

#endif
