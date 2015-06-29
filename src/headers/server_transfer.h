#include <stdbool.h>
#ifndef SERVER_TRANSFER_H
#define SERVER_TRANSFER_H

#define RECV_RETURN_BROADCASTNEW 0
#define RECV_RETURN_SEND_GRAPH_DEPTH_FIRST 1
#define RECV_RETURN_SEND_GRAPH_BREADTH_FIRST 2
#define RECV_RETURN_WRITE_GRAPH 3
#define RECV_RETURN_ERROR 4
#define RECV_RETURN_LOG 5
#define BROADCAST_RANDOM_RESTART 1
#define BROADCAST_RANDOM_CONTINUE 2
#define BROADCAST_ORDER	3
#define GRAPH_COLLECT_NO 1000 

extern int SERVER_LISTEN_PORT;

typedef struct broadcast {
	char ipAddr[250];
	int active;
} Broadcast;

void construct_broadcast(Broadcast* bc, const char* ip_addr, int act);

void send_file(int connected_socket, bool search_mode, int send_mode);

void send_log(int connected_socket);

int receive_file(int connected_socket);

void set_port();

void *send_to_one_des(void* _des);

void *send_to_des(void* _des);

void *server_listen_to_clients_handler();

int create_connection(Broadcast*);

void broadcast_graph();

void copy(char*, char*);
void delete_graph(char* send_dir);

extern struct broadcast* broadcast_list[100];
extern int desNum;
extern int recv_count;
extern int send_count;
extern int collect_count;
extern int gsize;
extern int clique_count;
extern char ip_log[14];
#endif
