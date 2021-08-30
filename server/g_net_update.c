/*
 * g_net_update.c
 *
 *  Created on: 2015Äê3ÔÂ16ÈÕ
 *      Author: j8
 */
/********************** g_net update version 2.0 ***************************/
#include "config_info.h"
#include "log.h"
#include "epoll_connect.h"
#include "udp_thread_pool.h"
#include "g_net_update.h"
#include "database_process.h"
#include "proto.h"

#define true 1
#define false 0
#define bool int

#define MAXBUF 1024
#define SERVER_UDP_PORT 								8814
#define	CONNECT_TO_SQL_SUCCESS							0
#define SERVER_TIMEOUT									60 * 1 //60S

static CONFIG_INFO config_info;
static int epoll_fd = -1; // the epoll fd
static int listen_fd = -1; // the socket fd socket create return
static int listen_udp_fd = -1;
static pthread_t accep_thread_t;
static pthread_t send_thread_t;

static int current_connected_total = 0; // the connections number
static int exit_accept_flag = 0; // is exit the accept
static int exit_flag = 0; // is exit the epoll wait
static int port = 8812;

static void closesocket(int fd);
static void dumpInfo(unsigned char *info, int length);
pthread_mutex_t connect_total_count_mutex = PTHREAD_MUTEX_INITIALIZER;

static void connect_total_count_add(int value)
{
	pthread_mutex_lock(&connect_total_count_mutex);
	current_connected_total +=  value;
	pthread_mutex_unlock(&connect_total_count_mutex);
}

static void connect_total_count_sub(int value)
{
	pthread_mutex_lock(&connect_total_count_mutex);
	current_connected_total -= value;
	pthread_mutex_unlock(&connect_total_count_mutex);
}


static void signal_handler_reboot(int32_t theSignal)
{
	int i;
	int sockfd;
	char log_str_buf[LOG_STR_BUF_LEN];
	signal(SIGPIPE, SIG_IGN);
	if (SIGKILL == theSignal || SIGTERM == theSignal) //we can know when system excute child thread is end
	{
		LOG_INFO(LOG_LEVEL_FATAL,  "receive kill signal exit the server.");
		if (listen_fd != -1)
		{
			closesocket(listen_fd);
			listen_fd = -1;
		}
		exit_flag = 1;
		exit_accept_flag = 1;
		for (i = 0; i < MAX_EVENTS; i++)
		{
			sockfd = get_fd_by_event_index(i);
			if (sockfd != -1)
			{
				closesocket(sockfd);
			}
		}
#if CONNECT_TO_SQL_SUCCESS
	sql_pool_destroy();
#endif
	}
	else if (SIGPIPE == theSignal)
	{
		LOG_INFO(LOG_LEVEL_INFO, "SIGPIPE received.\n");
	}
}

static void closesocket(int fd)
{
	shutdown(fd, SHUT_RDWR);
	close(fd);
}

static int set_non_blocking(int sock)
{
	int opts = -1;
	char log_str_buf[LOG_STR_BUF_LEN];

	opts = fcntl(sock, F_GETFL);
	if (opts < 0)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "fcntl(sock=%d,GETFL).\n", sock);
		return (-1);
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "fcntl(sock=%d,GETFL).\n", sock);
		return (-1);
	}
	return 1;
}

CONFIG_INFO *get_config_info(void)
{
	return &config_info;
}
#if 0
/****************************** accept task ******************************************/
static void *accept_thread(void *arg)
{
	int connect_fd = -1;
	socklen_t clilen = 0;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	struct sockaddr cliaddr;
	struct epoll_event ev;
	int val = 1;
	int err = 0;
	int bufsize = 32 * 1024; //1024*2048;
	int epoll_connect_event_index = -1;
	char log_str_buf[LOG_STR_BUF_LEN] = {0};

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listen_fd) // INVALID_SOCKET
	{
		LOG_INFO(LOG_LEVEL_ERROR, "create socket error.\n");
		return NULL;
	}
	//set socket options
	err = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (0 != err)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "setsockopt SO_REUSEADDR error.\n");
		return NULL;
	}
	err = setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, (char*)(&bufsize), sizeof(int));
	if (0 != err)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "setsockopt SO_RCVBUF error.\n");
		return NULL;
	}

	val = 2; //2 minitues
	err = setsockopt(listen_fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &val, sizeof(val));
	if (0 != err)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "setsockopt TCP_DEFER_ACCEPT error.\n");
		return NULL;
	}

//	set_non_blocking(listen_fd);
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(/*config_info.port*/port); //serveraddr.sin_port  = htons(SERVER_LISTEN_PORT);
	serveraddr.sin_family = AF_INET;
	if (-1 == bind(listen_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) // SOCKET_ERROR
	{
		LOG_INFO(LOG_LEVEL_ERROR, "bind config port %d error.\n", /*config_info.port*/port);		
		printf("bind config port %d error.\n", /*config_info.port*/port);
		return NULL;
	}
	printf("bind config port %d success.\n", /*config_info.port*/port);

	listen(listen_fd, LISTENQ); // 4096
	clilen = sizeof(cliaddr);
	while (!exit_accept_flag)
	{
		if (current_connected_total < MAX_EVENTS) // effective
		{
			if ((connect_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clilen)) < 0)
			{
				if ((errno != EAGAIN) && (errno != ECONNABORTED) && (errno != EPROTO) && (errno != EINTR))
				{
					LOG_INFO(LOG_LEVEL_ERROR,  "accept error %d,%s.\n", errno, strerror(errno));
					
				}
				continue;
			}
		}
		else
		{
			sleep(1);
			LOG_INFO(LOG_LEVEL_INDISPENSABLE, "current accept number achieve to maximum(%d) .\n", MAX_EVENTS);
			continue;
		}
		epoll_connect_event_index = get_epoll_connect_free_event_index();
		// no free epoll event
		if (epoll_connect_event_index == -1) // no the free connect event
		{
			LOG_INFO(LOG_LEVEL_ERROR, "Not find free connect.\n");
			if (connect_fd != -1)
			{
				closesocket(connect_fd);
				connect_fd = -1;
			}
			continue;
		}
		// set connect fd non blocking
		if (set_non_blocking(connect_fd) < 0)
		{
			LOG_INFO(LOG_LEVEL_ERROR, "set non-blocking socket = %d error.\n", connect_fd);
			if (connect_fd != -1)
			{
				closesocket(connect_fd);
				connect_fd = -1;
			}
			continue;
		}
		err = setsockopt(connect_fd, SOL_SOCKET, SO_RCVBUF, (char *)(&bufsize), sizeof(int));
		if (0 != err)
		{
			LOG_INFO(LOG_LEVEL_ERROR, "set socket(%d) setsockopt SO_RCVBUF error.\n", connect_fd);
			if (connect_fd != -1)
			{
				closesocket(connect_fd);
				connect_fd = -1;
			}
			continue;
		}
		init_epoll_connect_by_index(epoll_connect_event_index,
			connect_fd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		// add epoll event
		ev.data.fd = connect_fd;
		ev.events = EPOLLIN | EPOLLET; // set epoll event type
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev) == -1)
		{
			LOG_INFO(LOG_LEVEL_ERROR, "EPOLL_CTL_ADD %d,%s.\n", errno, strerror(errno));
			
			if (connect_fd != -1)
			{
				closesocket(connect_fd);
				connect_fd = -1;
			}
			continue;
		}
		connect_total_count_add(1);
		LOG_INFO(LOG_LEVEL_INDISPENSABLE, 
			"Epoll event[%d] *****connected from %s*****, fd:%d, Total number of clients currently connected = %d.\n", 
			epoll_connect_event_index, inet_ntoa(clientaddr.sin_addr), connect_fd, current_connected_total);
		
	}
	if (-1 != listen_fd) // out the while then close the socket
	{
		closesocket(listen_fd);
		listen_fd = -1;
	}
	return NULL;
}
#endif
static void *udp_thread(void *arg)
{
	int connect_fd = -1;
	socklen_t clilen = 0;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	struct sockaddr cliaddr;
	struct epoll_event ev;
	int val = 1;
	int err = 0;
	int bufsize = 32 * 1024; //1024*2048;
	
	char log_str_buf[LOG_STR_BUF_LEN] = {0};

	listen_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == listen_udp_fd) // INVALID_SOCKET
	{
		LOG_INFO(LOG_LEVEL_ERROR, "create socket error.\n");
		return NULL;
	}
	//set socket options
	err = setsockopt(listen_udp_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (0 != err)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "setsockopt SO_REUSEADDR error.\n");
		return NULL;
	}

	set_non_blocking(listen_udp_fd);
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(/*config_info.port*/SERVER_UDP_PORT); //serveraddr.sin_port  = htons(SERVER_LISTEN_PORT);
	serveraddr.sin_family = AF_INET;
	if (-1 == bind(listen_udp_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) // SOCKET_ERROR
	{
		LOG_INFO(LOG_LEVEL_ERROR, "bind config port %d error.\n", /*config_info.port*/SERVER_UDP_PORT);		
		printf("bind config port %d error.\n", /*config_info.port*/SERVER_UDP_PORT);
		return NULL;
	}
	printf("bind config port %d success.\n", /*config_info.port*/SERVER_UDP_PORT);


	
	//init_epoll_connect_by_index(epoll_connect_event_index,
	//	connect_fd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	// add epoll event
	ev.data.fd = listen_udp_fd;
	ev.events = EPOLLIN | EPOLLET; // set epoll event type
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_udp_fd, &ev) < 0)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "EPOLL_CTL_ADD %d,%s.\n", errno, strerror(errno));
		
		if (listen_udp_fd != -1)
		{
			closesocket(listen_udp_fd);
			listen_udp_fd = -1;
		}
		
	}
		
		

	//if (-1 != listen_udp_fd) // out the while then close the socket
	//{
	//	closesocket(listen_udp_fd);
	//	listen_udp_fd = -1;
	//}
	return NULL;
}


static int create_accept_task(void)
{
	//return pthread_create(&accep_thread_t, NULL, accept_thread, NULL);
	return pthread_create(&accep_thread_t, NULL, udp_thread, NULL);
}
/*************************************************************************/

static int recv_buffer_from_fd(int socket_fd, char *recv_buffer, int *recv_length)
{
	
#define BUFLEN 1024
	int rev = -1;
	int read_continue = 1;
	int total_recv_length = 0, len = 0;
	while (read_continue)
	{
		len = recv(socket_fd, &recv_buffer[total_recv_length], BUFLEN, 0);
		if (len > 0)
		{
			total_recv_length += len;
		}

		if (len < 0) // error
		{
//			if (errno == EAGAIN)
//			{
//				continue_read = 0;
//				break;
//			}
			read_continue = 0;
			rev = -1;
			break;
		}
		else if (len >= 0 && len < BUFLEN) // read over
		{
			read_continue = 0;
			rev = 0;
			*recv_length = total_recv_length;
			break;
		}
		else
		{
			read_continue = 0;
			rev = -1;
			break;
		}
	}
	return rev;
}

static int udp_recv_buffer_from_fd(int socket_fd, char *recv_buffer, int *recv_length, 
	struct sockaddr_in *pclient_addr)
{
	
#define BUFLEN 1024
	int rev = -1;
	int read_continue = 1;
	int total_recv_length = 0, len = 0;
	
    socklen_t cli_len=sizeof(*pclient_addr);
	while (read_continue)
	{
		len = recvfrom(socket_fd, &recv_buffer[total_recv_length], MAXBUF, 0, (struct sockaddr *)pclient_addr, &cli_len);
		//printf("receive from %s\n", inet_ntoa(pclient_addr->sin_addr));
		if (len > 0)
		{
			total_recv_length += len;
		}

		if (len < 0) // error
		{
//			if (errno == EAGAIN)
//			{
//				continue_read = 0;
//				break;
//			}
			read_continue = 0;
			rev = -1;
			break;
		}
		else if (len >= 0 && len < BUFLEN) // read over
		{
			read_continue = 0;
			rev = 0;
			*recv_length = total_recv_length;
			break;
		}
		else
		{
			read_continue = 0;
			rev = -1;
			break;
		}
	}
	return rev;
}


static int send_buffer_to_fd(int socket, unsigned char *pucBuf, int iLen)
{
	int n;
	int nwrite;

	n = iLen;

	while (n > 0)
	{
		nwrite = write(socket, pucBuf + iLen - n, n);
		if (nwrite < n)
		{
			if (nwrite == -1)
			{
				if (errno != EAGAIN)
				{
					LOG_INFO(LOG_LEVEL_ERROR, "Write.\n");
					return 0;
				}
				else
				{
					LOG_INFO(LOG_LEVEL_INFO, "Write EAGAIN.\n");
				}
			}
		}
		n -= nwrite;
	}
	return (iLen - n);
}

static int udp_send_buffer_to_fd(int socket, struct sockaddr_in *client_addr, unsigned char *pucBuf, int iLen)
{
	int n;
	int nwrite;

	n = iLen;
	socklen_t cli_len=sizeof(*client_addr);
	while (n > 0)
	{
		//nwrite = write(socket, pucBuf + iLen - n, n);
		nwrite = sendto(socket, pucBuf + iLen - n, n, 0, (struct sockaddr *)client_addr, cli_len);
		if (nwrite < n)
		{
			if (nwrite == -1)
			{
				if (errno != EAGAIN)
				{
					LOG_INFO(LOG_LEVEL_ERROR, "Write.\n");
					return 0;
				}
				else
				{
					LOG_INFO(LOG_LEVEL_INFO, "Write EAGAIN.\n");
				}
			}
		}
		n -= nwrite;
	}
	return (iLen - n);
}

static void dumpInfo(unsigned char *info, int length)
{
	int index = 0;
	int j = 0;
	char buffer[128] ;
	struct stJQMessage *msgHead = (struct stJQMessage *)info;
	if(msgHead->iMessageType == CMD_HEARTBEAT){//not dump heartbeat packet
		return;
	}
	bzero(buffer, 128*sizeof(char));
	LOG_INFO(LOG_LEVEL_INDISPENSABLE, "dump info length = %d\n", length);
	for (index = 0; index < length; index++, j+=3)
	{
		sprintf(&buffer[j],"%02x ",info[index]);
		if((j+3)/3   == 16){
			buffer[j+3] = 0;
			j = 0;
			LOG_INFO(LOG_LEVEL_INDISPENSABLE, "%s\n", buffer);
		}
	}
	buffer[j]=0;
	LOG_INFO(LOG_LEVEL_INDISPENSABLE, "%s\n", buffer);
	
}


void* respons_stb_info(udp_job_parameter *parameter, int thread_index)
{
	char *recv_buffer;
	char send_buffer[1024] = {0};
	int sockfd = parameter->fd;
	int index = 0, j = 0;
	int cmd;
	int dptr = 0;
	int len = 0;
	recv_buffer = parameter->recv_buffer;
	struct sockaddr_in client_addr = parameter->client_addr;  
	struct stJQMessage *msgHead = (struct stJQMessage *)recv_buffer;

	char *client_ip;
	client_ip = inet_ntoa(client_addr.sin_addr);
	
	if(msgHead->iMessageType!= CMD_HEARTBEAT){
		printf("[sockfd: %d %s] get command: 0x%x \n", sockfd, client_ip, msgHead->iMessageType);
		printf("[sockfd: %d %s] get buffer: %s \n",sockfd, client_ip, recv_buffer +8);
	}
	
	
	// deal with you logic
	if (sockfd != -1)
	{
		//int matched_event_index = get_matched_event_index_by_fd(sockfd);
		int matched_event_index = get_matched_event_index_by_addr(&client_addr);
		//for (index = 0; index < 2; index++, j+=2)
		//{
		//sprintf(&recv_buf[j], "%02x", recv_buffer[index]);
		//}

		switch(msgHead->iMessageType){
			case CMD_HANDSHAKE:
				{
				struct stJQMessage *hdr = (struct stJQMessage *)send_buffer;
				struct HandshakeMessage *hsMsg = (struct HandshakeMessage *)
					(recv_buffer + sizeof(struct stJQMessage));
				printf("get handshake msg : %s\n", hsMsg->greetings);
				memset(send_buffer, 0, 1024);
				//strcpy(send_buffer, "I have get you data");
				struct HandshakeAckMessage *stAck = (struct HandshakeAckMessage *)
				((char *)hdr +sizeof(struct stJQMessage));
				hdr->iMessageType = CMD_HANDSHAKE_ACK;
				struct in_addr tmp;
				tmp.s_addr = htonl(get_client_addr_by_index(matched_event_index));
				snprintf(stAck->info, 256, "addr:%s port:%d ",
					inet_ntoa(tmp),
					get_client_port_by_index(matched_event_index));
				printf("handshake ack: %s\n", stAck->info);
				len = sizeof(struct stJQMessage) + sizeof(struct HandshakeAckMessage);
				hdr->iMessageLen = len;
				udp_send_buffer_to_fd(sockfd, &client_addr, send_buffer, len);
				break;
				}
			case CMD_SENDUI:
				{
				EPOLL_CONNECT *pCon = get_connect_prv_by_index(matched_event_index);
				struct SendUIMessage *sendUIMsg= (struct SendUIMessage *)
					(recv_buffer + sizeof(struct stJQMessage));
				strncpy(pCon->UsrhashId, sendUIMsg->userId , sizeof(sendUIMsg->userId));
				strncpy(pCon->passwd, sendUIMsg->passwd, sizeof(sendUIMsg->passwd));
				printf("[SENDUI] %s  login\n", sendUIMsg->userId);
				}
				break;
			case CMD_GETREMOTE: //get address and port of remote host
				{
				char UsrHashId[10]={0};
				char Pass[7]={0};
				int ev_idx= -1;
				struct stJQMessage *hdr;
				print_all_users();
				memset(send_buffer, 0, 1024);
				struct GetRemoteMessage *grMsg = (struct GetRemoteMessage *)
					(recv_buffer + sizeof(struct stJQMessage));
				hdr = (struct stJQMessage *) send_buffer;
				struct GetRemoteAckMessage *remoteAck = (struct GetRemoteAckMessage *)
						(send_buffer +sizeof(struct stJQMessage));
				strncpy(UsrHashId, grMsg->userId, sizeof(grMsg->userId));
				strncpy(Pass, grMsg->passwd, sizeof(grMsg->passwd));
				ev_idx = get_matched_event_index_by_UsrHashId(UsrHashId);
				if(ev_idx == -1) {
					LOG_INFO(LOG_LEVEL_WARNING, "Not Found client event index,  may be caused by the client being offline\n");
										
					hdr->iMessageType = CMD_GETREMOTE_ACK;
					remoteAck->flag = false;
					snprintf(remoteAck->info, 256, "Not Found client event index,  may be caused by the client being offline");
					len = sizeof(struct stJQMessage) + sizeof(struct GetRemoteAckMessage);
					hdr->iMessageLen = len;
					printf("send remote ack: %s\n", remoteAck->info);
					udp_send_buffer_to_fd(sockfd, &client_addr, send_buffer, len);
					break;
				}
				EPOLL_CONNECT *pCon = get_connect_prv_by_index(ev_idx);
				//ack
				hdr->iMessageType = CMD_GETREMOTE_ACK;
				remoteAck->flag = true;
				remoteAck->ip_addr = pCon->client_ip; 
				remoteAck->port = pCon->client_port;
				struct in_addr tmp;
				tmp.s_addr = htonl(pCon->client_ip);
				snprintf(remoteAck->info, 256, "clientIp:%s clientPort:%d", inet_ntoa(tmp), pCon->client_port);
				len = sizeof(struct stJQMessage) + sizeof(struct GetRemoteAckMessage);
				hdr->iMessageLen = len;
				udp_send_buffer_to_fd(sockfd, &client_addr, send_buffer, len);
				break;
				}
			case CMD_P2PTRANS:
				{
				// A customer wants the server to send a punch message to another customer
				char UsrHashId[10]={0};
				char ip_b[20]={0};
				//char Pass[7]={0};
				int ev_idx= -1;
				struct stJQMessage *hdr;
				print_all_users();
				
				memset(send_buffer, 0, 1024);
				struct P2PTransMessage *p2pTransMsg = (struct P2PTransMessage *)
					(recv_buffer + sizeof(struct stJQMessage));
				strncpy(UsrHashId, p2pTransMsg->userId, sizeof(p2pTransMsg->userId));
				LOG_INFO_SCREEN(LOG_LEVEL_WARNING, "%s wants to p2p %s\n", inet_ntoa(client_addr.sin_addr), UsrHashId);
				ev_idx = get_matched_event_index_by_UsrHashId(UsrHashId);
				if(ev_idx == -1) {
					LOG_INFO_SCREEN(LOG_LEVEL_WARNING, " [CMD_P2PTRANS] Not Found client event index,  may be caused by the client being offline\n");
										
					break;
				}
				EPOLL_CONNECT *pCon = get_connect_prv_by_index(ev_idx);
						
				struct in_addr tmp;
				tmp.s_addr = htonl(pCon->client_ip);
				strcpy(ip_b, inet_ntoa(tmp));
				LOG_INFO_SCREEN(LOG_LEVEL_WARNING, " A [%s:%d] ------> B [%s:%d]\n", 
					inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
					ip_b, pCon->client_port);			
				
                //send address of A to B
                // A :  represents the client currently being processed
				bzero(send_buffer, sizeof(send_buffer));
				hdr = (struct stJQMessage *) send_buffer;
				struct P2PMessage *p2pMsg = (struct P2PMessage *)
						((char *)hdr +sizeof(struct stJQMessage));
				hdr->iMessageType = CMD_P2PSOMEONEWANTTOCALLYOU;
				p2pMsg->ipAddr = ntohl(client_addr.sin_addr.s_addr);
				p2pMsg->port = ntohs(client_addr.sin_port);
				len = sizeof(struct stJQMessage) + sizeof(struct P2PMessage);
				hdr->iMessageLen = len;
				struct sockaddr_in remote;
                remote.sin_family=AF_INET;
                remote.sin_port= htons(pCon->client_port); 
                remote.sin_addr.s_addr = htonl(pCon->client_ip);
				udp_send_buffer_to_fd(sockfd, &remote, send_buffer, len);	
				break;
				}
		
			case CMD_GETALLUSER:
				{
					int cnt = 0;
					printf("[CMD_GETALLUSER] ===>\n");
					get_all_users(NULL, &cnt);
					char *send_buf = malloc(sizeof(struct stJQMessage)+ sizeof(int)
						+sizeof(struct JQUserNode)*cnt) ;
					struct stJQMessage *hdr = (struct stJQMessage *)send_buf;
					struct GetAllUserAckMessage *gauAck = (struct GetAllUserAckMessage *)
						((char *) hdr + sizeof(struct stJQMessage));
					hdr->iMessageType = CMD_GETALLUSER_ACK;
					
					gauAck->cnt = cnt;
					
					get_all_users((char *)gauAck, &cnt);
					len = sizeof(struct stJQMessage) + sizeof(int) + 
						sizeof(struct JQUserNode)*cnt;
					hdr->iMessageLen = len;
					udp_send_buffer_to_fd(sockfd, &client_addr, send_buf, len);
					printf("[CMD_GETALLUSER] send cnt %d\n", cnt);
					free(send_buf);
					break;
				}
			default:
				break;
		}
		
#if 0
		connect_total_count_sub(1);
		LOG_INFO(LOG_LEVEL_INDISPENSABLE,
			"send data to stb over then close the socket(%d), *****client addr(%s)***** , thread_index = %d.\n", 
			sockfd, get_client_addr_by_index(matched_event_index), thread_index);
		free_event_by_index(matched_event_index);
		closesocket(sockfd);
		sockfd = -1;
#endif
	}else{
		printf("2222 sockfd = %d\n", sockfd);
	}

}
#if 0
void recycle_timeout_confd(thpool_t* thpool,time_t now, struct epoll_event *ev)
{
	int index = 0;
	int connect_socket_fd_temp = -1;
	int delete_pool_job_number = 0;
	char log_str_buf[LOG_STR_BUF_LEN];
	for (index = 0; index < MAX_EVENTS; index++)
	{
		connect_socket_fd_temp = get_fd_by_event_index(index);
		if (connect_socket_fd_temp != -1)
		{
			if ((now - get_event_connect_time_by_index(index)) > SERVER_TIMEOUT)
			{
				printf("Epoll event[%d] timeout closed and fd= %d.\n", index, connect_socket_fd_temp);
				LOG_INFO(LOG_LEVEL_INDISPENSABLE, "Epoll event[%d] timeout closed and fd= %d.\n", index, connect_socket_fd_temp);
				
				free_event_by_index(index);
				if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connect_socket_fd_temp, ev) == -1)
				{
					 LOG_INFO(LOG_LEVEL_ERROR, "EPOLL_CTL_DEL %d,%s.\n", errno, strerror(errno));
					
				}
				connect_total_count_sub(1);
				closesocket(connect_socket_fd_temp);
				connect_socket_fd_temp = -1;
			}
		}
	}
	// delete the pool job time out job
	delete_pool_job_number = delete_timeout_job(thpool, SERVER_TIMEOUT);
	connect_total_count_sub(delete_pool_job_number);
	
	LOG_INFO(LOG_LEVEL_INDISPENSABLE, "pool queque delete job number = %d.\n", delete_pool_job_number);

}
#endif

void recycle_timeout_confd(udp_thpool_t* thpool,time_t now)
{
	int index = 0;
	int connect_socket_fd_temp = -1;
	int delete_pool_job_number = 0;
	char log_str_buf[LOG_STR_BUF_LEN];
	for (index = 0; index < MAX_EVENTS; index++)
	{
		connect_socket_fd_temp = get_fd_by_event_index(index);
		if (connect_socket_fd_temp != -1)
		{
			if ((now - get_event_connect_time_by_index(index)) > SERVER_TIMEOUT)
			{
				struct in_addr tmp;
				tmp.s_addr = htonl(get_client_addr_by_index(index));
				LOG_INFO_SCREEN(LOG_LEVEL_INDISPENSABLE,
					"Epoll event[%d] timeout will be removed and fd: %d.\n"
					" client addr: %s\n",
					index, connect_socket_fd_temp,
					inet_ntoa(tmp));
				
				free_event_by_index(index);
				//if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connect_socket_fd_temp, ev) == -1)
				//{
				//	 LOG_INFO(LOG_LEVEL_ERROR, "EPOLL_CTL_DEL %d,%s.\n", errno, strerror(errno));
				//	
				//}
				connect_total_count_sub(1);
				//closesocket(connect_socket_fd_temp);
				connect_socket_fd_temp = -1;
			}
		}
	}
	// delete the pool job time out job
	delete_pool_job_number = delete_timeout_job(thpool, SERVER_TIMEOUT);
	connect_total_count_sub(delete_pool_job_number);
	
	LOG_INFO(LOG_LEVEL_INDISPENSABLE, "pool queque delete job number = %d.\n", delete_pool_job_number);

}

/*******************************************************************/
#define LOGI(lvl, fmt, arg...)\
printf("[%s %s %d]" #fmt,  __FILE__, __FUNCTION__, __LINE__, ##arg)
int main(int argc, char *argv[])
{
	char log_file_name[128] = {0};
	char log_str_buf[LOG_STR_BUF_LEN];
	struct rlimit rl;
	udp_thpool_t *thpool = NULL;
	time_t now;
	time_t prevTime, eventTime = 0;
	struct epoll_event ev, events[MAX_EVENTS];
	int epoll_events_number = 0;
	int index = 0;
	int connect_socket_fd_temp = -1;
	//char npc = "j8kk";

	//printf("[%s %s %d]" npc,  __FILE__, __FUNCTION__, __LINE__);
	//return 0;
	if (2 != argc)
	{
		printf("please input the port\n");
		return 0;
	}
	port = atoi(argv[1]);
	// read config info
	if (read_config_info(&config_info) != 0)
	{
		printf("[%s %s %d] read_config_info fail.\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	print_config_info(config_info);

	signal(SIGCHLD, SIG_IGN); // Ignore the child to the end of the signal, preventing the zombie process(2015.7.17)

	// init log
	sprintf(log_file_name, "log_%d.txt", port);
	// for distinguish between different ports
	set_log_file_name(log_file_name);
	if (log_init() != 0)
	{
		printf("init log error\n");
		return -1;
	}
	log_set_level(config_info.log_level);

	// create fork
	pid_t pidfd = fork();
	if (pidfd < 0)
	{
		LOG_INFO(LOG_LEVEL_FATAL,  "fork failed! errorcode = %d[%s].\n", errno, strerror(errno));
		log_close();
		return (-1);
	}
	if (pidfd != 0)
	{
		LOG_INFO(LOG_LEVEL_INFO, "parent fork over.\n");
		exit(0);
	}
	setsid();
	LOG_INFO(LOG_LEVEL_INFO, "children fork start.\n");

	//set max number of open files(also for tcp connections)
	rl.rlim_cur = MAX_EVENTS;
	rl.rlim_max = MAX_EVENTS;
	setrlimit(RLIMIT_NOFILE, &rl);
	getrlimit(RLIMIT_NOFILE, &rl);
	fprintf(stderr, "cur:%d\n", (int)rl.rlim_cur);
	fprintf(stderr, "max:%d\n", (int)rl.rlim_max);
	LOG_INFO(LOG_LEVEL_INFO, "information about rlimit cur:%d max:%d.\n", (int)rl.rlim_cur, (int)rl.rlim_max);
	
	sleep(1);
	signal(SIGKILL, signal_handler_reboot); // register the signal
	signal(SIGTERM, signal_handler_reboot);
	signal(SIGPIPE, signal_handler_reboot);

	init_epoll_connect();
//	g_net_init_mysql(); // connect to the database
#if CONNECT_TO_SQL_SUCCESS
	if (0 != sql_pool_create(THREAD_POLL_SIZE))
	{
		LOG_INFO(LOG_LEVEL_FATAL, "mysql error.\n");
		log_close();
		return -1;
	}
#endif
	epoll_fd = epoll_create(MAX_FDS); // 1024
	if (0 >= epoll_fd)
	{
		LOG_INFO(LOG_LEVEL_FATAL, "epoll_create error.\n");
		log_close();
		return -1;
	}

	// create thread pool
	thpool = udp_thpool_init(THREAD_POLL_SIZE);
	if (NULL == thpool)
	{
		LOG_INFO(LOG_LEVEL_FATAL, "create thread pool error.\n");
		log_close();
		return -1;
	}
	LOG_INFO(LOG_LEVEL_INDISPENSABLE, "thpool_init success.\n");

	// accept the socket connect
	create_accept_task();

	time(&prevTime);
	eventTime = prevTime;
	while (!exit_flag)
	{
		time(&now);
		if (abs(now - eventTime) >= SERVER_TIMEOUT) //SERVER_TIMEOUT second detect one time delete the time out event
		{
			printf("60s detect one time =====>\n");
			eventTime = now;
			recycle_timeout_confd(thpool, now);
		}
		epoll_events_number = epoll_wait(epoll_fd, events, MAX_EVENTS, 2000); //2seconds
		for (index = 0; index < epoll_events_number; ++index) // deal with the event
		{
			connect_socket_fd_temp = events[index].data.fd; // get the socket fd
			// delete epoll event
			#if 0
			if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[index].data.fd, &ev) == -1)
			{
				LOG_INFO(LOG_LEVEL_ERROR, "EPOLL_CTL_DEL %d,%s.\n", errno, strerror(errno));
				events[index].data.fd = -1;
			}
			#endif
			if (events[index].events & EPOLLIN) //have read event
			{
				int event_index = -1;
				int recv_length = 0;
				unsigned char recv_buffer[BUFFER_SIZE]={0};
				struct sockaddr_in client_addr;

				/*if (connect_socket_fd_temp < 0)
				{
					connect_total_count_sub(1);
					LOG_INFO(LOG_LEVEL_ERROR, "Event[%d] read invalid handle.\n", index);
					continue;
				}*/
				
				// receive the buffer from the socket fd
				if (0 == udp_recv_buffer_from_fd(connect_socket_fd_temp, recv_buffer, &recv_length, &client_addr))
				{
					//LOG_INFO(LOG_LEVEL_INDISPENSABLE, "recv_length = %d, current fd = %d, current job queue job number = %d.\n",
					//	recv_length, connect_socket_fd_temp, get_jobqueue_number(thpool));
					//event_index = get_matched_event_index_by_fd(connect_socket_fd_temp);
					event_index = get_matched_event_index_by_addr(&client_addr);
	
					LOG_INFO(LOG_LEVEL_ERROR, "Epoll get Event[%d] fd = %d.\n", event_index, connect_socket_fd_temp);
					
					// no the event
					if (event_index < 0)
					{
						
						LOG_INFO(LOG_LEVEL_ERROR, "not find matched fd = %d.\n", connect_socket_fd_temp);
						//init_epoll_connect_by_index(int iEvent,int iConnectFD,char * uiClientIP,int cliPort)
						event_index = get_epoll_connect_free_event_index();
						LOG_INFO(LOG_LEVEL_ERROR, "alloc event_index = %d.\n", event_index);
						// no free epoll event
						if (event_index == -1) // no the free connect event
						{
							LOG_INFO(LOG_LEVEL_ERROR, "Not find free connect.\n");
							if (connect_socket_fd_temp != -1)
							{
								closesocket(connect_socket_fd_temp);
								connect_socket_fd_temp = -1;
							}
							continue;
						}
						init_epoll_connect_by_index(event_index, connect_socket_fd_temp, 
							&client_addr);
						//free_event_by_index(event_index);
						/*
						if (connect_socket_fd_temp != -1)
						{
							closesocket(connect_socket_fd_temp);
							connect_socket_fd_temp = -1;
						}
						continue;*/
					}
					dumpInfo((unsigned char *)recv_buffer, recv_length);
					// receive buffer success then add the thread pool
					udp_thpool_add_work(thpool, (void*)respons_stb_info, event_index, connect_socket_fd_temp, client_addr, recv_buffer);
				}
				else
				{
					// receive buffer error
					connect_total_count_sub(1);
					LOG_INFO(LOG_LEVEL_INDISPENSABLE, "Epoll event[%d] not read data, and the socket fd = %d.\n", event_index, connect_socket_fd_temp);
					free_event_by_index(event_index);
					if (connect_socket_fd_temp != -1)
					{
						closesocket(connect_socket_fd_temp);
						connect_socket_fd_temp = -1;
					}
				}
			}
//			else if (events[index].events & EPOLLOUT) //have read event. Will not reach here
//			{
//				//;
//			}
			else
			{
				LOG_INFO(LOG_LEVEL_ERROR, "Unknown error! event.data.fd = %d.\n", events[index].data.fd);
				connect_total_count_sub(1);
				if (connect_socket_fd_temp < 0)
				{
					LOG_INFO(LOG_LEVEL_ERROR, "EPOLLOUT.\n");
					continue;
				}
				//close the socket
				free_event_by_index(get_matched_event_index_by_fd(connect_socket_fd_temp));
				if (connect_socket_fd_temp != -1)
				{
					closesocket(connect_socket_fd_temp);
					connect_socket_fd_temp = -1;
				}
			}
		}
	}
	log_close();
	if (listen_udp_fd != -1)
	{
		closesocket(listen_udp_fd);
		listen_udp_fd = -1;
	}

#if CONNECT_TO_SQL_SUCCESS
	sql_pool_destroy();
#endif
	exit_accept_flag = 1;
	udp_thpool_destroy(thpool);
	printf("[%s %s %d]Done...\n", __FILE__, __FUNCTION__, __LINE__);
	return 1;
}


