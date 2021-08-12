/*
 * epoll_connect.c
 *
 *  Created on: 2015��2��5��
 *      Author: Administrator
 */

#include "log.h"
#include "epoll_connect.h"

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
			snprintf(log_str_buf, LOG_STR_BUF_LEN, "file connection.c Event[%d] mutex init\n", iRet);
			log_s(LOG_LEVEL_INFO, log_str_buf);
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

void init_epoll_connect_by_index(int iEvent, int iConnectFD, char *uiClientIP, int cliPort)
{
	time_t now;

	time(&now);
	event_state_lock(iEvent);
	epoll_connect_client[iEvent].now = now;
	memset(epoll_connect_client[iEvent].client_ip_addr, 0, IP_ADDR_LENGTH);
	memcpy(epoll_connect_client[iEvent].client_ip_addr, uiClientIP, IP_ADDR_LENGTH);
	epoll_connect_client[iEvent].client_port = cliPort;
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

char *get_client_addr_by_index(int index)
{
	if (index >=0 && index < MAX_EVENTS)
	{
		return epoll_connect_client[index].client_ip_addr;
	}
	else
	{
		return "0.0.0.0";
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


