/*
 * epoll_connect.h
 *
 *  Created on: 20210820
 *      Author: Administrator
 */

#ifndef EPOLL_CONNECT_H_
#define EPOLL_CONNECT_H_
#include "g_net_global.h"
#define MAX_FDS            10240 //10240
#define MAX_EVENTS         MAX_FDS
#define LISTENQ            4096
#define	IP_ADDR_LENGTH	   20

typedef struct _epoll_connect_struct_
{
	int connect_fd;
	int socket_status; //0--initial,1--Live,2--need send,3--Need Close
	time_t now;
	/*********************/
	/*The Client IP address and PORT : TCP*/
	char client_ip_addr[IP_ADDR_LENGTH];
	int client_port;
	char UsrhashId[10];
	char passwd[7];
	pthread_mutex_t mutex;
} EPOLL_CONNECT;

void init_epoll_connect(void);
void init_epoll_connect_by_index(int iEvent, int iConnectFD, char *uiClientIP, int cliPort);
int get_epoll_connect_free_event_index(void);
int get_matched_event_index_by_fd(int iConnectFD);
void free_event_by_index(int index);
int get_fd_by_event_index(int index);
int update_time_by_index(int index, time_t now);
time_t get_event_connect_time_by_index(int index);
char *get_client_addr_by_index(int index);
int get_client_port_by_index(int index);
EPOLL_CONNECT *get_connect_prv_by_index(int index);
int get_matched_event_index_by_UsrHashId(char *UsrHashId);
int get_all_users(char *buf, int *outlen);
int print_all_users(void);


#endif /* EPOLL_CONNECT_H_ */
