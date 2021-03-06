#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <pthread.h>
#include "client_transfer.h"
#include "search.h"
static int SERVER_LISTEN_PORT = -1;
static int CLIENT_LISTEN_PORT = -1;

int send_file(char* ip_addr) {
	char buffer[BUFFER_SIZE], filename[250];
	int connected_socket, file_block_length = 0;
	memset(buffer, 0, sizeof(buffer));
	sprintf(filename, "../../file/client/new_graph");
	printf("Trying to transfer file : %s\n", filename);
	FILE * fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("Could not open to read!\n");
		return -1;
	} else {
		connected_socket = create_connection(ip_addr);	
		file_block_length = 0;
		//printf("Entering the read while block!\n");
		while (1) {
			file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
			if(file_block_length == 0) {
				if(feof(fp)) break;
				perror("fread error!\n");
				fclose(fp);
				close(connected_socket);
				return -1;
			} 
			//printf("file_block_length = %d\n", file_block_length);
			//printf("strlen(buffer):%d \n", strlen(buffer));
			//printf("buffer:  %s \n", buffer);
			
			//if (send(connected_socket, buffer, file_block_length, 0) < 0) {
			if(send(connected_socket, buffer, file_block_length, 0) < 0) {
				perror("Sending file failed!\n");
				fclose(fp);
				close(connected_socket);
				return -1;
			}
		}

		memset(buffer, 0, sizeof(buffer));
		fclose(fp);
		close(connected_socket);
		printf("File transmitted!\n\n");
		//nnew_graph_count ++;
	}
	return 0;
}

void send_check(char* ip_addr) {
	int connected_socket = create_connection(ip_addr);
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = 'c';
	if (send(connected_socket, buffer, 1, 0) < 0)
		printf("Sending check signal failed!\n");
	printf("check signal transmitted!\n\n");
	receive_file(connected_socket);
	close(connected_socket);
}

void send_request(char* ip_addr) {
	int connected_socket = create_connection(ip_addr);
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = 'r';
	if (send(connected_socket, buffer, 1, 0) < 0)
		printf("Sending request signal failed!\n");
	printf("request signal transmitted!\n\n");
	receive_file(connected_socket);
	close(connected_socket);
}

void receive_file(int connected_socket) {
	char buffer[BUFFER_SIZE], filename[250];
	int written_length, length;
	memset(buffer, 0, sizeof(buffer));

	printf("Receiving from server %s\n", buffer);
	// receive search mode
	length = recv(connected_socket, buffer, BUFFER_SIZE, 0);
	if(length <= 0) return;
	if(buffer[0] == 'd') {
		search_mode = SEARCH_MODE_DEPTH_FIRST;
		printf("Received DF search instruction!\n");
	}
	else if(buffer[0] == 'b') {
		search_mode = SEARCH_MODE_BREADTH_FIRST;
		printf("Received BF search instruction!\n");
	}
	// receive best_ever
	length = recv(connected_socket, buffer, BUFFER_SIZE, 0);
	if(length <= 0) return;
	best_ever = atoi(buffer);
	printf("Current searching target is %d!\n", best_ever);
		
	// receive file	
	printf("Now starts to receive file.\n");
	sprintf(filename, "../../file/client/old_graph");
	FILE * fp = fopen(filename, "w");
	if (fp == NULL) {
		perror("Could not open to write!\n");
		return;
	}
	
	while (true) {
		length = recv(connected_socket, buffer, BUFFER_SIZE, 0);
		perror("recv error :");
		if (length < 0) {
			printf("Receiving data failed!\n");
			break;
		}
		if (length == 0) break;
		written_length = fwrite(buffer, sizeof(char), length, fp);
		if (written_length < length) printf("File writing failed!\n");
		memset(buffer, 0, BUFFER_SIZE);
	}
	fclose(fp);
	recv_flag = true;
	//new_graph_count = 0;
	printf("File receiveing finished!\n\n");
}

void set_port() {
	srand(time(NULL) * 1000);
	SERVER_LISTEN_PORT = PORT;
	CLIENT_LISTEN_PORT = PORT;
}

void *client_always_listen_to_one_handler() {
	int err = pthread_detach(pthread_self());
	if (err != 0) {
		perror("Could not pthread_detach!");
	}

	int iResult = 0;

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	//Address family (must be AF_INET)
	serv_addr.sin_family = AF_INET;
	//IP port
	serv_addr.sin_port = htons(CLIENT_LISTEN_PORT);
	//IP address
	//The in_addr structure represents an IPv4 Internet address.
	/*If an application does not care what local address is assigned,
	specify the constant value INADDR_ANY for an IPv4 local address.
	*/
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int serv_socket = 0;
	//If no error occurs, socket returns a descriptor referencing the
	//new socket. Otherwise, a value of INVALID_SOCKET is returned, and a specific error code can be retrieved by calling WSAGetLastError.
	serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_socket < 0) {
		printf("Could not create client_always_listen_to_one_handler socket!\n");
		exit(1);
	}

	/*int bind(
	_In_  SOCKET s,
	_In_  const struct sockaddr *name,
	_In_  int namelen
	);
	*/
	//name [in]
	//A pointer to a sockaddr structure of the local address to assign to the bound socket.
	iResult = bind(serv_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (iResult != 0) {
		perror("Could not bind client_always_listen_to_one_handler socket!\n");
		exit(1);
	}

	printf("client_always_listen_to_one_handler starts to listen!\n");
	//If no error occurs, listen returns zero.
	if (listen(serv_socket, 1)) {
		printf("client_always_listen_to_one_handler listening failed!\n");
		exit(1);
	}

	while (1) {
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);

		int connected_socket;
		printf("\nStarts to accept!\n");
		if ((connected_socket = accept(serv_socket, (struct sockaddr*)&client_addr, &length)) == -1) {
			perror("Accepting failed!\n");
			exit(1);
		}
		
		char incoming_ip_addr[20];
		printf("Got connection from %s\n", inet_ntop(AF_INET, &(client_addr.sin_addr), incoming_ip_addr, 16));
		
		receive_file(connected_socket);
		close(connected_socket);
	}
	close(serv_socket);
	pthread_exit(0);
}

int create_connection(char* ip_addr) {
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_LISTEN_PORT);
	if (inet_aton(ip_addr, &server_addr.sin_addr) <= 0) {
		printf("Input IP is not correct!\n");
		exit(1);
	}

	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0) {
		printf("Could not create send_to_one_des socket!\n");
		exit(1);
	}

	socklen_t server_addr_length = sizeof(server_addr);

	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0) {
		printf("send_to_one_des could not connect!\n");
		exit(1);
	}
	printf("Connected to the server!\n");
	return client_socket;
}
