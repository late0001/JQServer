/*
 * client_test.c
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

#define CMD_SENDUI 		0xfffe
#define CMD_GETREMOTE	0xfffd
#define CMD_HANDSHAKE   0xfffc


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
			//printf("connect_total = %d\nrcv buffer: %s\n", connect_total, recv_buffer+4);
			//connect_total++;
			printf("recv buffer: %s\n", recv_buffer+4);
			
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
	int cur_dptr = 0;
	
	char send_buffer[1024] = {0};//"Hello JQServer";
	char *greets = "Hello JQServer, I'm linux client";

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
		
		cur_dptr = 0;
		*(int *)send_buffer = CMD_HANDSHAKE;
		cur_dptr += 4;
		strncpy(send_buffer+ cur_dptr, greets, strlen(greets));
		cur_dptr += strlen(greets);
		send_buffer[cur_dptr]='\0';
		printf("send handshake: %s\n cur_dptr = %d strlen(send_buffer) = %ld\n", send_buffer+4, cur_dptr, strlen(send_buffer));
		// send
		num = write(socket_fd, (char *)send_buffer, cur_dptr+1);
		if (num <= 0) {
			printf("send error\n");
			return -1;
		}
		
		err = pthread_create(&accep_thread_t, NULL, recv_thread, &socket_fd);
		
		memset(send_buffer, 0 , 1024);
		cur_dptr = 0;
		*(int *)send_buffer = CMD_SENDUI;
		cur_dptr += 4;
		strcpy(send_buffer + cur_dptr, "123456798");
		cur_dptr += 9;
		strcpy(send_buffer + cur_dptr, "WiGeWd");
		cur_dptr += 6;
		send_buffer[cur_dptr]='\0';
		// send
		num = write(socket_fd, (char *)send_buffer, cur_dptr+1);
		if (num <= 0) {
			printf("send error\n");
			return -1;
		}
		while(1) {
			char ch=getchar();
			if(ch == 'q') break;
				
			switch(ch){
			case 'g':
				memset(send_buffer, 0 , 1024);
				cur_dptr = 0;
				//get remote client ip and port
				*(int *)send_buffer = CMD_GETREMOTE;
				cur_dptr += 4;
				strcpy(send_buffer + cur_dptr, "876543210");
				cur_dptr += 9;
				strcpy(send_buffer + cur_dptr, "CiGeWK");
				cur_dptr += 6;
				send_buffer[cur_dptr]='\0';
				num = write(socket_fd, (char *)send_buffer, cur_dptr +1);
				break;
			
			default:
				break;
				
			}
			
		}
		
		
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
	for(index = 0; index < 1; index++)
	{
		temp = connect_to_g_net(port);
	}
	
	
	 
	return 0;
	
}

