// Win32ClientTest.cpp : �������̨Ӧ�ó������ڵ㡣
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
using namespace std;

struct recv_parameter {
	SOCKET sockCli;
	SOCKADDR_IN addrSrv;
};
bool RecvedACK;

//another p2p object
unsigned int p2p_ip_addr;
int p2p_port = -1;

void recv_func(void *arg)
{
	char recvBuf[1024];
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
			case CMD_P2PMESSAGE:{
				// ���յ�P2P����Ϣ
				char *comemessage;
				struct P2PMessage *hsMsg = (struct P2PMessage *)
					((char *)stHead + sizeof(struct stJQMessage));
				comemessage = (char *)stHead + sizeof(struct stJQMessage)
					+ sizeof(struct P2PMessage);
				printf("Recv a Message:%s\n", comemessage);
				struct stJQMessage sendbuf;
				sendbuf.iMessageType = CMD_P2PMESSAGE_ACK;
				sendbuf.iMessageLen = sizeof(sendbuf);
				sendto(sockCli, (const char*)&sendbuf,
				sizeof(sendbuf), 0, (const sockaddr*)&addrSrv, sizeof(addrSrv));		
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
			case CMD_P2PSOMEONEWANTTOCALLYOU: {
				struct P2PMessage *p2pMsg = (struct P2PMessage *)
					((char *)stHead + sizeof(struct stJQMessage));
				unsigned int ip = p2pMsg->ipAddr;
				// ���յ��������ָ����IP��ַ��
				printf("Recv p2someonewanttocallyou data\n");
				sockaddr_in remote;
				remote.sin_addr.S_un.S_addr = htonl(p2pMsg->ipAddr);
				remote.sin_family = AF_INET;
				remote.sin_port = htons(p2pMsg->port);

				// UDP hole punching
				stJQMessage message;
				message.iMessageType = CMD_P2PTRASH;
				message.iMessageLen = sizeof(message);
				sendto(sockCli, (const char *)&message, sizeof(message),
					0, (const sockaddr*)&remote, sizeof(remote));
				break;
			}
			case CMD_P2PMESSAGE_ACK:{
				// ������Ϣ��Ӧ��
				RecvedACK = true;
				break;
			}
			case CMD_P2PTRASH: {
				// �Է����͵Ĵ���Ϣ�����Ե���
				//do nothing ...
				printf("Recv p2ptrash data\n");
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
				printf("�Է�˵�� %s\n", recvBuf + 8);
				break;
			}
			
		}
		memset(recvBuf, 0, 1024);
	}

}

struct recv_parameter recv_param_in_timer;
//��ʱ�¼�  
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

//�߳�  //CALLBACK
//unsigned __stdcall heartbeat_proc(void *pvoid)
unsigned  CALLBACK heartbeat_proc(PVOID arg)
{
	//ǿ��ϵͳΪ�߳̽�����Ϣ����  
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	recv_param_in_timer = *(struct recv_parameter *)arg;
	//���ö�ʱ��  
	SetTimer(NULL, 10, 500, TimeProc);
	//��ȡ���ַ���Ϣ  
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_TIMER)
		{
			
			TranslateMessage(&msg);    // ������Ϣ  
			DispatchMessage(&msg);     // �ַ���Ϣ  
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

/* ������Ҫ�ĺ���������һ����Ϣ��ĳ���û�(C)
*���̣�ֱ����ĳ���û�������IP������Ϣ�������ǰû����ϵ��
*      ��ô����Ϣ���޷����ͣ����Ͷ˵ȴ���ʱ��
*      ��ʱ�󣬷��Ͷ˽�����һ��������Ϣ������ˣ�
*      Ҫ�����˷��͸��ͻ�Cһ����������C���������ʹ���Ϣ
*      �������̽��ظ�MAXRETRY��
*/
bool SendMessageTo(SOCKET socket, char *UserName, char *Message)
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
		int isend = sendto(socket, (const char *)stHead,
			stHead->iMessageLen, 0, (const sockaddr*)&remote, sizeof(remote));

		// �ȴ������߳̽��˱���޸�
		for (int j = 0; j < 10; j++)
		{
			if (RecvedACK) {
				printf("received Ack, communication normal\n");
				return true;
			}
			else
				Sleep(300);
		}

		// û�н��յ�Ŀ�������Ļ�Ӧ����ΪĿ�������Ķ˿�ӳ��û��
		// �򿪣���ô����������Ϣ����������Ҫ����������Ŀ������
		// ��ӳ��˿ڣ�UDP�򶴣�
		sockaddr_in server;
		//server.sin_addr.S_un.S_addr = inet_addr(SRV_IPADDR);
		inet_pton(AF_INET, SRV_IPADDR, &server.sin_addr);
		server.sin_family = AF_INET;
		server.sin_port = htons(SRV_PORT);
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
		sendto(socket, send_buf, stHead->iMessageLen,
			0, (const sockaddr*)&server, sizeof(server));
		Sleep(100);// �ȴ��Է��ȷ�����Ϣ��
	}
	return false;
}

// ���������ʱֻ��exit��send����
// ����getu�����ȡ��ǰ�������������û�
int ParseCommand(char * CommandLine, struct recv_parameter *recv_param)
{
	SOCKET sockClient = recv_param->sockCli;
	sockaddr_in addrSrv = recv_param->addrSrv;
	char send_buffer[1024] = { 0 };//"Hello JQServer";
	int pktlen = 0;
	if (strlen(CommandLine) < 4) {
		printf("�Ƿ�����\n");
		return -1;
	}
	char Command[10];
	strncpy(Command, CommandLine, 4);
	Command[4] = '\0';
	if (strcmp(Command, "exit") == 0) {
		 	closesocket(sockClient);//�ر��׽���
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
				sendname[i-5] = CommandLine[i];
			else
			{
				sendname[i-5] = '\0';
				break;
			}
		}
		strcpy(message, &(CommandLine[i + 1]));
		if (SendMessageTo(sockClient, "876543210", message))
			printf("Send OK!\n");
		else
			printf("Send Failure!\n");

	}else if (strcmp(Command, "getu") == 0) {
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
	printf("          send\n");
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err = -1;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);//��ʼ��
	if (err != 0)
	{
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}
	SOCKET sockCli = socket(AF_INET, SOCK_DGRAM, 0);//�����׽���
	SOCKADDR_IN addrSrv;
	inet_pton(AF_INET, SRV_IPADDR, &addrSrv.sin_addr);
	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); "49.152.49.84");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(SRV_PORT);
	/*err = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//���ӵ�Ŀ������
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
	//	cout << "�Է�˵��";
	//	printf("%s\n", recvBuf+4);
	//	memset(recvBuf, 0, 100);
	//	//cout << "��˵��";
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
	
// 	closesocket(sockClient);//�ر��׽���
// 	WSACleanup();
	return 0;
}

