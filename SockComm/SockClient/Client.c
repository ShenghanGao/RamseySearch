#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "Transfer.h"
/*
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
void *(*start_routine) (void *), void *arg);

Compile and link with -pthread.

The pthread_create() function starts a new thread in the calling
process.  The new thread starts execution by invoking
start_routine(); arg is passed as the sole argument of
start_routine().
*/
#include <pthread.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("You should input server IP address only!\n");
		exit(1);
	}
	
	struct broadcast server;
	server.ipAddr = argv[1];
	server.fileName = "DataToServer.txt";
	
	char* file_from_server = "DataFromServer.txt";

	pthread_t sock_recv_thread_id;
	pthread_create(&sock_recv_thread_id, NULL, client_always_listen_to_one_handler, (void*)file_from_server);

	while (1) {
		pthread_t sock_send_thread_id;
		pthread_create(&sock_send_thread_id, NULL, send_to_one_des, (void*)&server);

		int err = 0;
		err = pthread_join(sock_send_thread_id, NULL);
		if (err != 0) {
			printf("sock_thread goes wrong! %s \n", strerror(err));
		}
		sleep(5);
	}
	return 0;
}
