#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "dirent.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <pthread.h>
#include "server_transfer.h"
#include "clique_count.h"
#include "graph.h"
#include "search.h"
#define BUFFER_SIZE 1024
#define PORT 8000

static int CLIENT_LISTEN_PORT = -1;

void delete_graph(char*);

void construct_broadcast(Broadcast* bc, const char* ip_addr, int act) {
	strcpy(bc->ipAddr, ip_addr);
	bc->active = act;
}

void send_file(int connected_socket, bool search_mode, int send_mode) {
		char buffer[BUFFER_SIZE], filename[250];
		int file_block_length = 0, send_no;
		memset(buffer, 0, sizeof(buffer));
		
		printf("Sending clique_count %d\n", clique_count);	
		if (search_mode == SEARCH_MODE_DEPTH_FIRST)
			buffer[0] = 'd';
		else
			buffer[0] = 'b';
		
		if (send(connected_socket, buffer, BUFFER_SIZE, 0) < 0) {
			perror("Sending failed! error");
			return;
		}
		memset(buffer, 0, sizeof(buffer));
		
		printf("Sending clique_count %d\n", clique_count);	
		sprintf(buffer, "%d", clique_count);
		if (send(connected_socket, buffer, BUFFER_SIZE, 0) < 0) {
			perror("Sending failed! error ");
			return;
		}
		memset(buffer, 0, sizeof(buffer));
		
		printf("Sending graph %d\n", send_count % GRAPH_COLLECT_NO);

		switch(send_mode) {
		case BROADCAST_RANDOM_RESTART:
			send_no = rand() % GRAPH_COLLECT_NO;
			sprintf(filename, "../../file/server/seed_%d/%d", (clique_count + 1) % 2, send_no);
			printf("RESTARTING, sending %d graph from last seed dir\n", send_no);
			break;
		case BROADCAST_RANDOM_CONTINUE:
			send_no = rand() % collect_count;
			sprintf(filename, "../../file/server/seed_%d/%d", clique_count % 2, send_no);
			printf("CONTINUING, sending %d graph from current seed dir\n", send_no);
			break;
		case BROADCAST_ORDER:
			send_no = (send_count * 25 + rand() % 100) % GRAPH_COLLECT_NO;
			printf("NEW, sending %d graph from current seed dir\n", send_no);
			sprintf(filename, "../../file/server/seed_%d/%d", (clique_count + 1) % 2, send_no);
			send_count ++;
			break;
		}
		printf("Reading file %s\n", filename);
		
		FILE * fp = fopen(filename, "r");
		if (fp == NULL) {
			perror("Could not open to read!");
			return;
		}
    else {
      file_block_length = 0;
			//printf("Entering the read while block!\n");
			while (1) {
				file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
				if (file_block_length <= 0) {
					//perror("fread error ");
					break;
				}
				//printf("file_block_length = %d\n", file_block_length);
				if (send(connected_socket, buffer, file_block_length, 0) < 0) {
					perror("Sending file failed! error ");
					break;
				}
				memset(buffer, '0', sizeof(buffer));
			}
		}
		fclose(fp);
		printf("File transmitted!\n");
}

void send_log(int connected_socket) {
	char buffer[BUFFER_SIZE];
	int file_block_length = 0;
	memset(buffer, 0, sizeof(buffer));

	FILE* fp = fopen("server_print.log", "r");
	if (fp == NULL) {
		perror("Could not open log to read!");
		return;
	}
	else {
		file_block_length = 0;
		while (1) {
			file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp);
			if (file_block_length <= 0) {
				perror("fread error ");
				break;
			}
			//printf("file_block_length = %d\n", file_block_length);
			if (send(connected_socket, buffer, file_block_length, 0) < 0) {
				perror("Sending file failed! error ");
				break;
			}
			memset(buffer, '0', sizeof(buffer));
		}
	}
	fclose(fp);
	printf("Log transmitted!\n");
}

int receive_file(int connected_socket) {
	char buffer[BUFFER_SIZE], filename[250];
	int written_length;
	memset(buffer, '0', sizeof(buffer));
	
	int length = recv(connected_socket, buffer, BUFFER_SIZE, 0);
	//printf("buffer %s", buffer);
	if (length < 0) {
		printf("Receiving data failed!\n");
		return RECV_RETURN_ERROR;
	}
	else if (buffer[0] == 'c') {
		if(send_count == GRAPH_COLLECT_NO
			 && collect_count != GRAPH_COLLECT_NO)
			return RECV_RETURN_SEND_GRAPH_DEPTH_FIRST;
		else
			return RECV_RETURN_SEND_GRAPH_BREADTH_FIRST;
	}
	else if (buffer[0] == 'r') {
		recv_count++;
		if (send_count == GRAPH_COLLECT_NO 
				&& collect_count != GRAPH_COLLECT_NO) {
			return RECV_RETURN_SEND_GRAPH_DEPTH_FIRST;
		}
		else
			return RECV_RETURN_SEND_GRAPH_BREADTH_FIRST;
	}
	else if (buffer[0] == 'l') {
			return RECV_RETURN_LOG;
	}
	else {
		sprintf(filename, "../../file/server/temp/temp");
		printf("Receiving filename: %s \n", filename);
		FILE * fp = fopen(filename, "w");
		if (fp == NULL) {
			perror("Could not open to write! fopen error: ");
			return RECV_RETURN_ERROR;
		}
		written_length = fwrite(buffer, sizeof(char), length, fp);
		if (written_length < length) printf("File writing failed!\n");
		memset(buffer, 0, BUFFER_SIZE);
		while (1) {
			length = recv(connected_socket, buffer, BUFFER_SIZE, 0);
 			if(!length) break;
			else if (length < 0) {
				perror("Receiving data failed! recv error ");
				return RECV_RETURN_ERROR;
			}
			
			//printf("buffer: %s\n", buffer);
			written_length = fwrite(buffer, sizeof(char), length, fp);
			//printf("written length: %d\n", written_length);
			if (written_length < length) perror("File writing failed! ");
			memset(buffer, 0, BUFFER_SIZE);
		}
		fclose(fp);
		printf("File receiveing finished!\n");
		return RECV_RETURN_WRITE_GRAPH;
	}
}

void set_port() {
	srand(time(NULL) * 1000);
	SERVER_LISTEN_PORT = PORT;
	CLIENT_LISTEN_PORT = PORT;
}

void *send_to_des(void* _des) {
	struct broadcast** des_list = (struct broadcast **)_des;

	int i;

	for (i = 0; i < desNum; ++i) {
		int is_active = (*(des_list + i))->active;
		if (is_active == -1) continue;

		char* ip_addr = (*(des_list + i))->ipAddr;

		printf("Send to ip: %s\n", ip_addr);
		
		int iResult = 0, client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (client_socket < 0) {
			printf("Could not create send_to_des socket!\n");
			exit(1);
		}

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(CLIENT_LISTEN_PORT);
		
		if (inet_aton(ip_addr, &server_addr.sin_addr) <= 0) {
			printf("IP is not correct!\n");
			exit(1);
		}
		
		socklen_t server_addr_length = sizeof(server_addr);

		//printf("send_to_des tries to connect!\n");
		iResult = connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length);
		if (iResult != 0) {
			printf("send_to_des could not connect! %s has retreated!!!\n", ip_addr);
			(*(des_list + i))->active = -1;
			close(client_socket);
			continue;
		}
		printf("send_to_des connected!\n");

		close(client_socket);
	}
	pthread_exit(0);
}

int create_connection(Broadcast* des) {
	struct sockaddr_in server_addr;
	memset(&server_addr, '0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_LISTEN_PORT);
	if (inet_aton(des->ipAddr, &server_addr.sin_addr) <= 0) {
		printf("Input IP is not correct!\n");
		return -1;
	}

	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0) {
		printf("Could not create send_to_one_des socket!\n");
		return -1;
	}

	socklen_t server_addr_length = sizeof(server_addr);

	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0) {
		printf("send_to_one_des could not connect!\n");
		return -2;
	}
	printf("Connected to the destination!\n");
	return client_socket;
}

void broadcast_graph() {
	printf("broadcast_graph!\n");
	int i, socket;
	collect_count = 0;
	send_count = 0;
	for (i = 0; i < desNum; i++) {
		int is_active = (*(broadcast_list + i))->active;
		if (is_active == -1) continue;

		char* ip_addr = (*(broadcast_list + i))->ipAddr;
		Broadcast* tmp = (Broadcast*)malloc(sizeof(Broadcast));
		construct_broadcast(tmp, ip_addr, 1);
		socket = create_connection(tmp);
		//printf("socket  =  %d\n", socket);
		if(socket != -1 && socket != -2){ 
			send_file(socket, SEARCH_MODE_BREADTH_FIRST, BROADCAST_ORDER);
			close(socket);
		}
		else {
			printf("create_connection failed!\n");
			if (socket == -2) {
				broadcast_list[i]->active = -1;
				FILE* fp = fopen(ip_log, "a");
				if (fp == NULL) {
					perror("Could not open to add!");
					continue;
				}
				fprintf(fp, "\nconnect()<0 ! %s has retreated!!!\n", ip_addr);
				fclose(fp);
			}
		}

/*		printf("ip_addr: %s\n", ip_addr);

		pthread_t sock_send_thread_id;
		err = pthread_create(&sock_send_thread_id, NULL, send_to_one_des, (void*)tmp);
		if (err != 0) perror("Could not create send_to_one_des thread!");

		err = pthread_join(sock_send_thread_id, NULL);
		if (err != 0) {
			printf("sock_thread goes wrong! %s \n", strerror(err));
			perror("sock_thread goes wrong!");
		}*/
		free(tmp);
	}
}

void copy(char* source_file, char* target_file) {
	char ch;
	FILE* source, *target;


	source = fopen(source_file, "r");

	if (source == NULL)
	{
		printf("Press any key to exit...\n");
		return;
	}

	target = fopen(target_file, "w");
	if (target == NULL) {
		perror("Could not open to write! fopen error: ");
		return;
	}

	while ((ch = fgetc(source)) != EOF)
		fputc(ch, target);

	printf("File copied successfully.\n");

	fclose(source);
	fclose(target);
}

void delete_graph(char* send_dir) {
	// These are data types defined in the "dirent" header
	struct dirent *next_file;
	DIR *theFolder;

	char filepath[256];

	theFolder = opendir(send_dir);

	while (next_file = readdir(theFolder))   {
		// build the full path for each file in the folder
		sprintf(filepath, "%s/%s", send_dir, next_file->d_name);
		remove(filepath);
	}
}
