/*
 * epoll_connect.c
 *
 *  Created on: 20180820
 *      Author: Administrator
 */

#include "log.h"
#include "proto.h"
#include "epoll_connect.h"
#include <linux/types.h>
static char log_str_buf[LOG_STR_BUF_LEN];
static EPOLL_CONNECT epoll_connect_client[MAX_EVENTS];



static void event_state_lock(int iEvent)
{
	int iRet;
	
	iRet = pthread_mutex_lock(&epoll_connect_client[iEvent].mutex);
	
	if (iRet != 0)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "Event[%d] mutex lock\n", iEvent);
	}
}

static void event_state_unlock(int iEvent)
{
	int iRet;
	
	iRet = pthread_mutex_unlock(&epoll_connect_client[iEvent].mutex);
	
	if (iRet != 0)
	{
		LOG_INFO(LOG_LEVEL_ERROR, "Event[%d] mutex unlock\n", iEvent);
	}
}


void init_epoll_connect(void)
{
	int iIndex;
	int iRet;

	memset((char*)epoll_connect_client, 0, sizeof(epoll_connect_client));
	for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
	{
		epoll_connect_client[iIndex].connect_fd = -1;
		epoll_connect_client[iIndex].socket_status = 0;
		iRet = pthread_mutex_init(&epoll_connect_client[iIndex].mutex, NULL);
		if (iRet != 0)
		{
			LOG_INFO(LOG_LEVEL_INFO, " Event[%d] mutex init\n", iRet);
		}
	}
}

int get_epoll_connect_free_event_index(void)
{
	int iIndex;

	for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
	{
		if (epoll_connect_client[iIndex].connect_fd == -1)
		{
			return iIndex;
		}
	}
	return (-1);
}


/**
* @param 
* iEvent : epoll_connect_client数组的索引
*/
void init_epoll_connect_by_index(int iEvent, int iConnectFD, struct sockaddr_in *client_addr)
{
	time_t now;
	time(&now);
	event_state_lock(iEvent);
	epoll_connect_client[iEvent].now = now;
	epoll_connect_client[iEvent].client_addr = *client_addr;
	epoll_connect_client[iEvent].client_ip = ntohl(client_addr->sin_addr.s_addr);
	epoll_connect_client[iEvent].client_port = ntohs(client_addr->sin_port);
	epoll_connect_client[iEvent].connect_fd = iConnectFD;
	epoll_connect_client[iEvent].socket_status = 1;
	event_state_unlock(iEvent);
}

int get_matched_event_index_by_fd(int iConnectFD)
{
	int iIndex;

	for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
	{
		if (epoll_connect_client[iIndex].connect_fd == iConnectFD)
		{
			return iIndex;
		}
	}
	return (-1);
}

int get_matched_event_index_by_addr(struct sockaddr_in *cli_addr)
{
	int iIndex;
    unsigned int cli_ip = ntohl(cli_addr->sin_addr.s_addr);
	for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
	{

		if( cli_ip == epoll_connect_client[iIndex].client_ip)
		{
			return iIndex;
		}
	}
	return (-1);
}

int get_matched_event_index_by_ip_addr(unsigned int ip_addr)
{
	int iIndex;

	for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
	{
		if(ip_addr == epoll_connect_client[iIndex].client_ip )
		{
			return iIndex;
		}
	}
	return (-1);
}


int get_all_users(char *buf, int *outlen)
{
	int iIndex;
	int cnt = 0;
	struct GetAllUserAckMessage *userList = (struct GetAllUserAckMessage *)buf;
	struct JQUserNode *userNode = NULL ;
	if(buf == NULL){
		for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
		{
			if (epoll_connect_client[iIndex].connect_fd != -1)
			{
				cnt++;
			}
		}
		*outlen = cnt;
		return 0;
	}
	userNode = userList->node ;
	for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
	{
		if (epoll_connect_client[iIndex].connect_fd != -1)
		{
			userNode[cnt].now = epoll_connect_client[iIndex].now;
			userNode[cnt].client_ip = epoll_connect_client[iIndex].client_ip;
			userNode[cnt].client_port = epoll_connect_client[iIndex].client_port;
			strcpy(userNode[cnt].UsrhashId, epoll_connect_client[iIndex].UsrhashId);
			strcpy(userNode[cnt].passwd, epoll_connect_client[iIndex].passwd);
			cnt++;
			
		}
	}
	return 0;

}

int print_all_users(void)
{
	int iIndex;
	printf("Now IP   Port   UserId  Password\n");

	for (iIndex = 0; iIndex < 10; iIndex++)
	{
		struct in_addr tmp;
        tmp.s_addr = htonl(epoll_connect_client[iIndex].client_ip);
		
		printf("%ld %s %d %s %s\n",
			epoll_connect_client[iIndex].now,
			inet_ntoa(tmp),
			epoll_connect_client[iIndex].client_port,
			epoll_connect_client[iIndex].UsrhashId,
			epoll_connect_client[iIndex].passwd);
	}
	return 0;

}


//更新最近一次读socket时间
int update_time_by_index(int index, time_t now)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		 epoll_connect_client[index].now = now;
	}
	else
	{
		return -1;
	}
	
	return (0);
}

int get_matched_event_index_by_UsrHashId(char *UsrHashId)
{
	int iIndex;

	for (iIndex = 0; iIndex < MAX_EVENTS; iIndex++)
	{
		if (!strncmp(epoll_connect_client[iIndex].UsrhashId, UsrHashId, 9))
		{
			return iIndex;
		}
	}
	return (-1);
}


void free_event_by_index(int index)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		event_state_lock(index);
		epoll_connect_client[index].connect_fd = -1;
		epoll_connect_client[index].socket_status = 0;
		epoll_connect_client[index].client_ip = 0;
		epoll_connect_client[index].client_port = -1;
		bzero(epoll_connect_client[index].UsrhashId, 
			sizeof(epoll_connect_client[index].UsrhashId));
		bzero(epoll_connect_client[index].passwd, 
			sizeof(epoll_connect_client[index].passwd));
		event_state_unlock(index);
	}
}

int get_fd_by_event_index(int index)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		return epoll_connect_client[index].connect_fd;
	}
	else
	{
		return -1;
	}
}

time_t get_event_connect_time_by_index(int index)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		return epoll_connect_client[index].now;
	}
	else
	{
		time_t now;
		time(&now);
		return now;
	}
}

unsigned int get_client_addr_by_index(int index)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		return epoll_connect_client[index].client_ip;
	}
	else
	{
		return 0;
	}
}

int get_client_port_by_index(int index)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		return epoll_connect_client[index].client_port;
	}
	//else
	//{
		return -1;
	//}
}

EPOLL_CONNECT *get_connect_prv_by_index(int index)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		return &epoll_connect_client[index];
	}

	return NULL;

}


