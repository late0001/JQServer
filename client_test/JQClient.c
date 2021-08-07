/*
 * stb_client_test.c
 *
 *  Created on: 2015Äê3ÔÂ16ÈÕ
 *      Author: Administrator
 */
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include "g_net.h"

// socket info
#define G_NET_UPDATE_SERVER_ADDR					"127.0.0.1"
//#define G_NET_UPDATE_SERVER_ADDR					"192.168.0.109" 

#define MAX_SN_LEN									8
#define TRY_CONNECT_TIMES							1

#define PORT_NUMBER									1

static int port  = 8812;

static pthread_t accep_thread_t;
static int connect_total = 0;

typedef int BOOL;
#ifndef FALSE
#define	FALSE						(0)
#endif
#ifndef	TRUE
#define	TRUE						(!FALSE)
#endif


static void *recv_thread(void *arg)
{
	int socket_fd;
	int num;
	char recv_buffer[2048] = {0};
	socket_fd = *(int*)arg;
	while(1)
	{
		//------------------------------
		// recv
		num = recv(socket_fd, recv_buffer, 2048, 0);
		if(num == -1) break;
		if (num > 0) // success
		{
			printf("connect_total = %d\nrcv buffer: %s\n", connect_total, recv_buffer);
			connect_total++;
			
		}
	}
	printf("exit recv_thread \n");
	return NULL;
}


/**
 * return value:0: recv data error, 1: connect error, 2: success
 */
int connect_to_g_net(int port)
{
	int return_value = 0;
	int socket_fd, err, num, loc;
	struct sockaddr_in server_addr;
	int recv_len;
	
	char send_buffer[1024] = "Hello JQServer";

	// create socket
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	// set server data info
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(G_NET_UPDATE_SERVER_ADDR);
	// connect
	err = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(err == 0)
	{
		// send
		num = write(socket_fd, (char *)send_buffer, strlen(send_buffer));
		if (num <= 0) {
			printf("send error\n");
			return -1;
		}
		err = pthread_create(&accep_thread_t, NULL, recv_thread, &socket_fd);
		
	}
	else
	{
		printf("connect error\n");
		return_value = 1;
	}

	return return_value;
}

static void *con_thread(void *arg)
{
	int loop_index , index;
	for (loop_index = 0; loop_index < 200; loop_index++)
	{
		// simulation connect 3 times
		for(index = 0; index < TRY_CONNECT_TIMES; index++)
		{
			connect_to_g_net(port);
		}
	}
	return NULL;
}




int main()
{
	int index = 0, temp;
	int port_index = 0;
	
	

	// simulation connect 3 times
	for(index = 0; index < 10; index++)
	{
		temp = connect_to_g_net(port);
	}
	
	while(1) {
		char ch=getchar();
		if(ch== 'q')
			break;
	}
	 
	return 0;
	
}

