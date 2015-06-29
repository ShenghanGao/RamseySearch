#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <pthread.h>
#include "server_transfer.h"
#include "search.h"
#include "graph.h"
#include "clique_count.h"

int desNum = 0;
int recv_count = 0;
int send_count = 0;
int collect_count = 0;
int gsize = 107;
int clique_count; // the clique count of the seeds we are currently waiting for
struct broadcast* broadcast_list[100];
int SERVER_LISTEN_PORT = -1;
char ip_log[14] = "ip_status.log";

void broadcast_graph();
void *server_print_handler();

int main(int argc, char* argv[]) {
	char dir_name[250];
	int *g;
	char des_file[250], incoming_ip_addr[20];
	char* existing_ip_addr;
	int p, err = 0;
	struct sockaddr_in serv_addr;	
	
	clique_count = RECURSION_THRESHOLD - 1;
	// upon initialization, put all files into seed_0
	
	// initialize server
	memset(&broadcast_list, 0, sizeof(broadcast_list));
	for (p = 0; p < 100; ++p)
		memset(&broadcast_list[p], 0, sizeof(broadcast_list[p]));
	set_port();


	int connectedSocket, gsize, count, i, exist,
		serv_socket = 0, recv_result;

	int iResult = 0;
	memset(&serv_addr, 0, sizeof(serv_addr));

	//Address family (must be AF_INET)
	serv_addr.sin_family = AF_INET;
	//IP port
	serv_addr.sin_port = htons(SERVER_LISTEN_PORT);
	//IP address
	//The in_addr structure represents an IPv4 Internet address.
	/*If an application does not care what local address is assigned,
	specify the constant value INADDR_ANY for an IPv4 local address.
	*/
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//If no error occurs, socket returns a descriptor referencing the
	//new socket. Otherwise, a value of INVALID_SOCKET is returned, and a specific error code can be retrieved by calling WSAGetLastError.
	serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_socket < 0) {
		perror("Could not create server_listen_to_clients_handler socket!");
		exit(1);
	}

	//A pointer to a sockaddr structure of the local address to assign to the bound socket.
	iResult = bind(serv_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (iResult != 0) {
		perror("Could not bind server_listen_to_clients_handler socket!");
		exit(1);
	}

	//printf("Server starts to listen!\n");
	//If no error occurs, listen returns zero.
	if (listen(serv_socket, 20)) {
		perror("server_listen_to_clients_handler listening failed!");
		exit(1);
	}

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t length = sizeof(client_addr);
	
	pthread_t server_print_thread_id;
	err = pthread_create(&server_print_thread_id, NULL, server_print_handler, NULL);
	if (err != 0) perror("Could not create server_print_thread_id thread!");

	FILE* log;
	while (1) {
		connectedSocket = accept(serv_socket, (struct sockaddr*)&client_addr, &length);
		if (connectedSocket == 0) {
			perror("accept:");
			break;
		}
		log = fopen(ip_log, "a");
		if (log == NULL) perror("Could not open to add!");

		printf("\nThe connection from %s\n", inet_ntop(AF_INET, &(client_addr.sin_addr), incoming_ip_addr, 16));
		//if (log != NULL) fprintf(log, "\nThe connection from %s\n", incoming_ip_addr);
		//printf("incoming_ip_addr = %s\n", incoming_ip_addr);
		exist = 0;
		for (i = 0; i < desNum; ++i) {
			existing_ip_addr = broadcast_list[i]->ipAddr;
			if (strcmp(incoming_ip_addr, existing_ip_addr) == 0) {
				printf("This IP is already in the list!\n");
				//if (log != NULL) fprintf(log, "IP Address: %s is already in the list!\n", incoming_ip_addr);
				//printf("broadcast_list[%d]->ipAddr = %s\n", i, incoming_ip_addr);
				//printf("desNum = %d\n", desNum);
				if (broadcast_list[i]->active == -1) {
					broadcast_list[i]->active = 1;
					if (log != NULL) fprintf(log, "\nIP address: %s rejoins!!!\n", incoming_ip_addr);
				}
				exist = 1;
				break;
			}
		}

		if (exist == 0) {
			Broadcast* broadcast_target = (Broadcast*)malloc(sizeof(Broadcast));
			construct_broadcast(broadcast_target, incoming_ip_addr, 1);

			broadcast_list[desNum] = broadcast_target;

			//printf("broadcast_list[%d] = broadcast_target", desNum);
			
			if (log != NULL) fprintf(log, "New IP Address: %s added.\ndesNum = %d\n", incoming_ip_addr, desNum);

			++desNum;
		}
		fclose(log);
		
		//printf("Trying to receive!\n");
		recv_result = receive_file(connectedSocket);
		if (recv_result == RECV_RETURN_ERROR)
			printf("Return error, receive error\n");
		else if (recv_result == RECV_RETURN_SEND_GRAPH_DEPTH_FIRST) {
			if (collect_count == 0) 
				send_file(connectedSocket, SEARCH_MODE_DEPTH_FIRST, BROADCAST_RANDOM_RESTART);
				//broadcast_graph(SEARCH_MODE_DEPTH_FIRST, BROADCAST_RANDOM_RESTART);
			else
				send_file(connectedSocket, SEARCH_MODE_DEPTH_FIRST, BROADCAST_RANDOM_CONTINUE);
			//	broadcast_graph(SEARCH_MODE_DEPTH_FIRST, BROADCAST_RANDOM_CONTINUE);
		}
		else if (recv_result == RECV_RETURN_SEND_GRAPH_BREADTH_FIRST)
			send_file(connectedSocket, SEARCH_MODE_BREADTH_FIRST, BROADCAST_ORDER);
		else if (recv_result == RECV_RETURN_LOG)
			send_log(connectedSocket);
		else {
			ReadGraph("../../file/server/temp/temp", &g, &gsize);
			count = CliqueCount(g, gsize);
			if (count == 0) {
				sprintf(des_file, "../../file/server/ce_%d/%d", gsize, collect_count);
				copy("../../file/server/temp/temp", des_file);
				sprintf(des_file, "../../file/server/seed_%d/%d", clique_count % 2, collect_count++);
				copy("../../file/server/temp/temp", des_file);
			}
			else if (count <= clique_count){
				sprintf(des_file, "../../file/server/seed_%d/%d", clique_count % 2, collect_count++);
				copy("../../file/server/temp/temp", des_file);
			} // else leave the received file there because it is obsolete

			if (collect_count >= GRAPH_COLLECT_NO) {
				if (clique_count == 0){
					clique_count = RECURSION_THRESHOLD - 1;
					sprintf(dir_name, "../../file/server/ce_%d", ++gsize);
					mkdir(dir_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				}
				else clique_count--;
				// clean seed file
				sprintf(dir_name, "../../file/server/seed_%d", clique_count % 2);
				//printf("cleaning %s\n", dir_name);
				delete_graph(dir_name);
				broadcast_graph();
			}
		}
		//printf("Finish receiving\n");
		close(connectedSocket);
	}
	close(serv_socket);
	return 0;
}

void *server_print_handler() {
	char* filename = "server_print.log";
	FILE* fp;
	while (1) {
		fp = fopen(filename, "w");
		if (fp == NULL) {
			perror("Could not open to write!");
			continue;
		}

		fprintf(fp, "recv_count: %d\n", recv_count);
		fprintf(fp, "send_count: %d\n", send_count);
		fprintf(fp, "clique_count: %d\n", clique_count);
		fprintf(fp, "currently waiting for %d graphs\n", GRAPH_COLLECT_NO - collect_count);
		fprintf(fp, "current maximum counterexample %d\n", gsize);
		fclose(fp);
		sleep(5);
	}
	pthread_exit(0);
}
