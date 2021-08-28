// Win32ClientTest.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#define SRV_IPADDR "192.168.0.108" // "172.29.11.221" //
#define SRV_PORT 8814 //port
#define COMMANDMAXC 256
//TcpClient.cpp
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <mstcpip.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")
#include <iostream>
#include "proto.h"
using namespace std;

struct recv_parameter {
	SOCKET sockCli;
	SOCKADDR_IN addrSrv;
};

void recv_func(void *arg)
{
	char recvBuf[256];
	struct recv_parameter *recv_param =(struct recv_parameter *)arg;
	SOCKET sockCli = recv_param->sockCli;
	sockaddr_in addrSrv = recv_param->addrSrv;
	socklen_t addrlen;
	struct stJQMessage *stHead;
	int len = 0;
	addrlen = sizeof(addrSrv);
	while (true){
		//while (len = recv(sockClient, recvBuf, 1024, 0) > 0) {
		while (len = recvfrom(sockCli, recvBuf, 1024, 0, (sockaddr *)&addrSrv, &addrlen) > 0) {
			stHead = (struct stJQMessage *)recvBuf;
			switch (stHead->iMessageType)
			{
			case CMD_HANDSHAKE_ACK: {
				struct HandshakeAckMessage *hsMsg = (struct HandshakeAckMessage *)
					((char *)stHead + sizeof(struct stJQMessage));
				printf("[HANDSHAKE ACK] %s\n", hsMsg->info);
				break;
			}
			case CMD_GETREMOTE_ACK: {
				struct GetRemoteAckMessage *graMsg = (struct GetRemoteAckMessage *)
					((char *)stHead + sizeof(struct stJQMessage));
				//if(!graMsg->flag)
					printf("[GETREMOTE ACK] %s\n", graMsg->info);
				break;
			}
			case CMD_GETALLUSER_ACK: {
				int cnt;
				int i;
				struct GetAllUserAckMessage *gauaMsg;
				struct JQUserNode *node;
				gauaMsg = (struct GetAllUserAckMessage *)
					((char *)stHead + sizeof(struct stJQMessage));
				cnt = gauaMsg->cnt;
				printf("user list:\n");
				printf("userId   |   Password  | IP | port | create time\n");
				for (i = 0; i < cnt; i++) {
					node = &gauaMsg->node[i];
					printf("%s  %s   %s %d %I64d\n", node->UsrhashId,
						node->passwd,
						node->client_ip_addr,
						node->client_port,
						node->now);
				}
				break;
			}
			default:
				printf("对方说： %s\n", recvBuf + 8);
				break;
			}
			
		}
		memset(recvBuf, 0, 256);
	}

}

struct recv_parameter recv_param_in_timer;
//定时事件  
void  CALLBACK TimeProc(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
	char send_buffer[512] = { 0 };
	struct HeartBeatMessage hbMsg;
	hbMsg.iMessageType = CMD_HEARTBEAT;
	SOCKET sockCli = recv_param_in_timer.sockCli;
	sockaddr_in addrSrv = recv_param_in_timer.addrSrv;
	if (sockCli) {
		sendto(sockCli, (char *)&hbMsg, sizeof(struct HeartBeatMessage), 0, 
			(struct sockaddr*)&addrSrv, sizeof(addrSrv));
	}else {
		printf("socket fd have lost! \n");
	}
    //printf("thread send heartbeat \n");
}

//线程  //CALLBACK
//unsigned __stdcall heartbeat_proc(void *pvoid)
unsigned  CALLBACK heartbeat_proc(PVOID arg)
{
	//强制系统为线程建立消息队列  
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	recv_param_in_timer = *(struct recv_parameter *)arg;
	//设置定时器  
	SetTimer(NULL, 10, 500, TimeProc);
	//获取并分发消息  
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_TIMER)
		{
			
			TranslateMessage(&msg);    // 翻译消息  
			DispatchMessage(&msg);     // 分发消息  
		}
	}

	KillTimer(NULL, 10);
	printf("Heartbeat thread end here\n");
	return 0;
}

int ConstructHandShakePkt(char *buf, int *pktlen, const char *greets)
{
	int len = 0;
	memset(buf, 0, 512);
	struct stJQMessage *stHead;
	struct HandshakeMessage *msg;
	stHead = (struct stJQMessage *)buf;
	stHead->iMessageType = CMD_HANDSHAKE;
	msg = (struct HandshakeMessage *)(buf + sizeof(struct stJQMessage));
	strncpy(msg->greetings, greets, strlen(greets));
	len = sizeof(stJQMessage) + strlen(greets) + 1;
	stHead->iMessageLen = len;
	*pktlen = len;
	return 0;
}

int ConstructSendUIPkt(char *buf, int *pktlen,const char *userId, const char *passwd)
{
	int len = 0;
	memset(buf, 0, 512);
	struct stJQMessage *stHead;
	struct SendUIMessage *msg;
	stHead = (struct stJQMessage *)buf;
	stHead->iMessageType = CMD_SENDUI;
	msg = (struct SendUIMessage *)(buf + sizeof(struct stJQMessage));
	strcpy(msg->userId, userId);
	strcpy(msg->passwd, passwd);
	len = sizeof(stJQMessage) + sizeof(SendUIMessage);
	stHead->iMessageLen = len;
	*pktlen = len;

	return 0;
}

int ConstructGetRemotePkt(char *buf, int *pktlen, const char *userId, const char *passwd)
{
	int len = 0;
	memset(buf, 0, 512);
	struct stJQMessage *stHead;
	struct GetRemoteMessage *msg;
	stHead = (struct stJQMessage *)buf;
	stHead->iMessageType = CMD_GETREMOTE;
	msg = (struct GetRemoteMessage *)(buf + sizeof(struct stJQMessage));
	strcpy(msg->userId, userId);
	strcpy(msg->passwd, passwd);
	len = sizeof(stJQMessage) + sizeof(GetRemoteMessage);
	stHead->iMessageLen = len;
	*pktlen = len;
	return 0;
}

int ConstructGetAllUserPkt(char *buf, int *pktlen)
{
	int len = 0;
	memset(buf, 0, 512);
	struct stJQMessage *stHead;
	stHead = (struct stJQMessage *)buf;
	stHead->iMessageType = CMD_GETALLUSER;
	len = sizeof(stJQMessage);
	stHead->iMessageLen = len;
	*pktlen = len;
	return 0;
}

// 解析命令，暂时只有exit和send命令
// 新增getu命令，获取当前服务器的所有用户
int ParseCommand(char * CommandLine, struct recv_parameter *recv_param)
{
	SOCKET sockClient = recv_param->sockCli;
	sockaddr_in addrSrv = recv_param->addrSrv;
	char send_buffer[1024] = { 0 };//"Hello JQServer";
	int pktlen = 0;
	if (strlen(CommandLine) < 4) {
		printf("非法输入\n");
		return -1;
	}
	char Command[10];
	strncpy(Command, CommandLine, 4);
	Command[4] = '\0';
	if (strcmp(Command, "exit") == 0) {
		 	closesocket(sockClient);//关闭套接字
		 	WSACleanup();
			exit(0);
	}
	else if (strcmp(Command, "getu") == 0) {
		printf("Send command to get remote address\n");
		ConstructGetRemotePkt(send_buffer, &pktlen, "876543210", "CiGeWK");
		sendto(sockClient, send_buffer, pktlen, 0, (struct sockaddr *)&addrSrv, sizeof(addrSrv));
	}
	else if (strcmp(Command, "allu") == 0)
	{
		printf("Send command to get All user \n");
		ConstructGetAllUserPkt(send_buffer, &pktlen);
		sendto(sockClient, send_buffer, pktlen, 0, (struct sockaddr *)&addrSrv, sizeof(addrSrv));
	}
	return 0;
}

void OutputUsage()
{
	printf("You can input command:\n");
	printf("Command Type:\"send \", \"exit \", \"getu \" ...\n");
	printf("Example : send Username Message\n");
	printf("          exit\n");
	printf("          getu\n");
	printf("          allu\n");
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err = -1;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);//初始化
	if (err != 0)
	{
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}
	SOCKET sockCli = socket(AF_INET, SOCK_DGRAM, 0);//建立套接字
	SOCKADDR_IN addrSrv;
	inet_pton(AF_INET, SRV_IPADDR, &addrSrv.sin_addr);
	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); "49.152.49.84");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(SRV_PORT);
	/*err = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//连接到目的主机
	if (err == SOCKET_ERROR) 
		printf("Socket error\n");
	*/
	struct recv_parameter recv_param;
	recv_param.addrSrv = addrSrv;
	recv_param.sockCli = sockCli;
	//char recvBuf[100];
	char send_buffer[1024] = { 0 };//"Hello JQServer";
	char *greets = "Hello JQServer! I'm windows client";
	int pktlen = 0;
	ConstructHandShakePkt(send_buffer, &pktlen, greets);
	printf("Send handshake!\n");
	sendto(sockCli, send_buffer, pktlen, 0, (struct sockaddr *)&addrSrv, sizeof(addrSrv));

	_beginthread(recv_func, NULL, &recv_param);
	Sleep(5);
	//send self information
	ConstructSendUIPkt(send_buffer, &pktlen, "123456798", "WiGeWd");
	//cout << "Send userInfo!\n";
	//cout << "Send len "<< cur_dptr <<endl;
	printf("Send user info\n");
	printf("Send len %d\n", pktlen);
	if (SOCKET_ERROR == sendto(sockCli, send_buffer, pktlen, 0, (struct sockaddr *)&addrSrv, sizeof(addrSrv))) {
		printf("Error code: %d\n", WSAGetLastError());
	}
	printf("sockfd: %ld\n", sockCli);
	/*
	BOOL bKeepAlive = TRUE;
	int nRet = setsockopt(sockCli, SOL_SOCKET, SO_KEEPALIVE,
		(char*)&bKeepAlive, sizeof(bKeepAlive));
	if (nRet == SOCKET_ERROR)
	{
		printf("setsockopt failed: %d\n", WSAGetLastError());
		return FALSE;
	}
	// set KeepAlive parameter  
	tcp_keepalive alive_in;
	tcp_keepalive alive_out;
	alive_in.keepalivetime = 500;  // 0.5s  
	alive_in.keepaliveinterval = 5000; //5s  
	alive_in.onoff = TRUE;
	unsigned long ulBytesReturn = 0;
	nRet = WSAIoctl(sockCli, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
		&alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);
	if (nRet == SOCKET_ERROR)
	{
		printf("WSAIoctl failed: %d\n", WSAGetLastError());
		return FALSE;
	}*/
	 _beginthreadex(NULL, NULL, heartbeat_proc, &recv_param,0, NULL );

	//while (true)
	//{

	//	recv(sockClient, recvBuf, 1024, 0);
	//	cout << "对方说：";
	//	printf("%s\n", recvBuf+4);
	//	memset(recvBuf, 0, 100);
	//	//cout << "我说：";
	//	//cin >> sendBuf;
	//	//send(sockClient, sendBuf, strlen(sendBuf) + 1, 0);
	//}
	
	OutputUsage();
	for (;;)
	{
		char Command[COMMANDMAXC];
		gets_s(Command);
		ParseCommand(Command, &recv_param);
	}
	
// 	closesocket(sockClient);//关闭套接字
// 	WSACleanup();
	return 0;
}

