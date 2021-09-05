// Win32ClientTest.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#define SRV_IPADDR "192.168.0.125" // "172.29.11.221" //
#define SRV_PORT	8814 //port
#define COMMANDMAXC 256
#define MAXRETRY	5
//TcpClient.cpp
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <mstcpip.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")
#include <iostream>
#include "proto.h"
#include "ClientSocket.h"
#include "BaseManager.h"

using namespace std;


bool RecvedACK;

//another p2p object
unsigned int p2p_ip_addr;
int p2p_port = -1;

void recv_func(CClientSocket *client, LPBYTE lpBuffer, DWORD dwIoSize)
{
	struct stJQMessage *stHead;

	stHead = (struct stJQMessage *)lpBuffer;
	switch (stHead->iMessageType)
	{
	case CMD_P2PMESSAGE: { //由另一终端发过来的，服务器不会发此消息
		// 接收到P2P的消息
		char *comemessage;
		struct P2PMessage *hsMsg = (struct P2PMessage *)
			((char *)stHead + sizeof(struct stJQMessage));
		comemessage = (char *)stHead + sizeof(struct stJQMessage)
			+ sizeof(struct P2PMessage);
		printf("Recv a Message:%s\n", comemessage);
		struct stJQMessage sendbuf;
		sendbuf.iMessageType = CMD_P2PMESSAGE_ACK;
		sendbuf.iMessageLen = sizeof(sendbuf);
		client->Send((LPBYTE)&sendbuf, sizeof(sendbuf));
		break;

	}
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
		p2p_ip_addr = graMsg->ip_addr;
		p2p_port = graMsg->port;
		break;
	}
	case CMD_P2PSOMEONEWANTTOCALLYOU: {// from server
		struct P2PMessage *p2pMsg = (struct P2PMessage *)
			((char *)stHead + sizeof(struct stJQMessage));
		unsigned int ip = p2pMsg->ipAddr;
		// 接收到打洞命令，向指定的IP地址打洞
		printf("Recv p2someonewanttocallyou data\n");
		sockaddr_in remote;
		remote.sin_addr.S_un.S_addr = htonl(p2pMsg->ipAddr);
		remote.sin_family = AF_INET;
		remote.sin_port = htons(p2pMsg->port);

		// UDP hole punching
		stJQMessage message;
		message.iMessageType = CMD_P2PTRASH;
		message.iMessageLen = sizeof(message);
		client->Send( (LPBYTE)&message, sizeof(message), (sockaddr *)&remote, sizeof(remote));
		break;
	}
	case CMD_P2PMESSAGE_ACK: {
		// 发送消息的应答
		RecvedACK = true;
		break;
	}
	case CMD_P2PTRASH: {
		// 对方发送的打洞消息，忽略掉。
		//do nothing ...
		printf("Recv p2ptrash data\n");
		break;
	}
	case CMD_GETALLUSER_ACK: {// from server
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
			in_addr tmp;
			tmp.S_un.S_addr = htonl(node->client_ip);
			//inet_ntoa( tmp)
			char str[INET_ADDRSTRLEN];
			//PCSTR
			_Null_terminated_ CONST CHAR * ptr = inet_ntop(AF_INET, &tmp, str, sizeof(str));
			printf("%s  %s   %s %d %I64d\n", node->UsrhashId,
				node->passwd,
				ptr,
				node->client_port,
				node->now);
		}
		break;
	}
	default:
		printf("对方说： %s\n", lpBuffer + 8);
		break;
	}




}



CClientSocket *clientSocketInTimer;
//定时事件  
void  CALLBACK TimeProc(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
	char send_buffer[512] = { 0 };
	struct HeartBeatMessage hbMsg;
	hbMsg.iMessageType = CMD_HEARTBEAT;

	CClientSocket *clientSocket = clientSocketInTimer;

	if (clientSocket) {
		clientSocket->Send((LPBYTE)&hbMsg, sizeof(struct HeartBeatMessage));
	}
	else {
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
	clientSocketInTimer = (CClientSocket *)arg;
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

int ConstructSendUIPkt(char *buf, int *pktlen, const char *userId, const char *passwd)
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

/* 这是主要的函数：发送一个消息给某个用户(C)
*流程：直接向某个用户的外网IP发送消息，如果此前没有联系过
*      那么此消息将无法发送，发送端等待超时。
*      超时后，发送端将发送一个请求信息到服务端，
*      要求服务端发送给客户C一个请求，请求C给本机发送打洞消息
*      以上流程将重复MAXRETRY次
*/
bool SendMessageTo(CClientSocket *pClient, char *UserName, char *Message)
{

	char send_buf[1024];
	unsigned int UserIP = p2p_ip_addr;
	unsigned short UserPort = p2p_port;
	if (p2p_port == -1) return false;
	memset(send_buf, 0, sizeof(send_buf));

	struct stJQMessage *stHead = (struct stJQMessage *)send_buf;
	for (int i = 0; i < MAXRETRY; i++)
	{
		RecvedACK = false;

		struct sockaddr_in remote;
		remote.sin_addr.S_un.S_addr = htonl(UserIP);
		remote.sin_family = AF_INET;
		remote.sin_port = htons(UserPort);
		//  -+---------------------+-------------------+-------------+-
		//   |  struct stJQMessage | struct P2PMessage |  pMessage   |
		//  -+---------------------+-------------------+-------------+-
		//
		struct P2PMessage *p2pMsg = (struct P2PMessage *)
			((char *)stHead + sizeof(struct stJQMessage));
		stHead->iMessageType = CMD_P2PMESSAGE;
		char *pMessage = (char *)stHead + sizeof(struct stJQMessage) + sizeof(struct P2PMessage);
		strcpy(pMessage, Message);
		stHead->iMessageLen = sizeof(struct stJQMessage) + sizeof(struct P2PMessage)
			+ strlen(Message) + 1;
		int isend = pClient->Send((LPBYTE)stHead,
			stHead->iMessageLen, (const sockaddr*)&remote, sizeof(remote));

		// 等待接收线程将此标记修改
		for (int j = 0; j < 10; j++)
		{
			if (RecvedACK) {
				printf("received Ack, communication normal\n");
				return true;
			}
			else
				Sleep(300);
		}

		// 没有接收到目标主机的回应，认为目标主机的端口映射没有
		// 打开，那么发送请求信息给服务器，要服务器告诉目标主机
		// 打开映射端口（UDP打洞）
// 		sockaddr_in server;
// 		//server.sin_addr.S_un.S_addr = inet_addr(SRV_IPADDR);
// 		inet_pton(AF_INET, SRV_IPADDR, &server.sin_addr);
// 		server.sin_family = AF_INET;
// 		server.sin_port = htons(SRV_PORT);
		//  -+---------------------+------------------------+--
		//   |  struct stJQMessage | struct P2PTransMessage |  
		//  -+---------------------+------------------------+--
		//
		memset(send_buf, 0, sizeof(send_buf));
		stHead = (struct stJQMessage *)send_buf;
		stHead->iMessageType = CMD_P2PTRANS;
		struct P2PTransMessage *transMsg = (struct P2PTransMessage *)
			((char *)stHead + sizeof(struct stJQMessage));
		strcpy(transMsg->userId, UserName);
		stHead->iMessageLen = sizeof(struct stJQMessage) + sizeof(struct P2PTransMessage);
		pClient->Send((LPBYTE)send_buf, stHead->iMessageLen);
		Sleep(100);// 等待对方先发送信息。
	}
	return false;
}

// 解析命令，暂时只有exit和send命令
// 新增getu命令，获取当前服务器的所有用户
int ParseCommand(char * CommandLine, CClientSocket *pClient)
{

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
		pClient->Close();
		WSACleanup();
		exit(0);
	}
	else if (strcmp(Command, "send") == 0) {
		char sendname[20];
		char message[COMMANDMAXC];
		int i;
		for (i = 5;; i++)
		{
			if (CommandLine[i] != ' ')
				sendname[i - 5] = CommandLine[i];
			else
			{
				sendname[i - 5] = '\0';
				break;
			}
		}
		strcpy(message, &(CommandLine[i + 1]));
		if (SendMessageTo(pClient, "876543210", message))
			printf("Send OK!\n");
		else
			printf("Send Failure!\n");

	}
	else if (strcmp(Command, "getu") == 0) {
		printf("Send command to get remote address\n");
		ConstructGetRemotePkt(send_buffer, &pktlen, "876543210", "CiGeWK");
		pClient->Send( (LPBYTE)send_buffer, pktlen);
	}
	else if (strcmp(Command, "allu") == 0)
	{
		printf("Send command to get All user \n");
		ConstructGetAllUserPkt(send_buffer, &pktlen);
		pClient->Send((LPBYTE) send_buffer, pktlen);
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
	printf("          send\n");
}

int main()
{

	int err = -1;

	//构造函数 初始化Socket库
	CClientSocket socketClient;
	//	SOCKET sockCli = socket(AF_INET, SOCK_DGRAM, 0);//建立套接字
	// 	SOCKADDR_IN addrSrv;
	// 	inet_pton(AF_INET, SRV_IPADDR, &addrSrv.sin_addr);
	// 	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); "49.152.49.84");
	// 	addrSrv.sin_family = AF_INET;
	// 	addrSrv.sin_port = htons(SRV_PORT);

	socketClient.InitSocket(SRV_IPADDR, SRV_PORT);
	/*err = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//连接到目的主机
	if (err == SOCKET_ERROR)
		printf("Socket error\n");
	*/

	//char recvBuf[100];
	char send_buffer[1024] = { 0 };//"Hello JQServer";
	char *greets = "Hello JQServer! I'm windows client";
	int pktlen = 0;
	ConstructHandShakePkt(send_buffer, &pktlen, greets);
	printf("Send handshake!\n");
	socketClient.Send((LPBYTE)send_buffer, pktlen);

	//_beginthread(recv_func, NULL, &recv_param);
	//Sleep(5);
	CBaseManager manager(&socketClient);

	socketClient.StartReceiving();
	//socketClient.setManagerCallBack(&manager);
	//send self information
	ConstructSendUIPkt(send_buffer, &pktlen, "123456798", "WiGeWd");
	//cout << "Send userInfo!\n";
	//cout << "Send len "<< cur_dptr <<endl;
	printf("Send user info\n");
	printf("Send len %d\n", pktlen);
	if (SOCKET_ERROR == socketClient.Send((LPBYTE) send_buffer, pktlen)) {
		printf("Error code: %d\n", WSAGetLastError());
	}
	//printf("sockfd: %ld\n", sockCli);
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
	_beginthreadex(NULL, NULL, heartbeat_proc, &socketClient, 0, NULL);

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
		ParseCommand(Command, &socketClient);
	}

	// 	closesocket(sockClient);//关闭套接字
	// 	WSACleanup();
	return 0;
}

