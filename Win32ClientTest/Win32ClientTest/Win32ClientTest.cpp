// Win32ClientTest.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "stdafx.h"
#define SRV_IPADDR "172.29.11.221" //"192.168.0.109"
#define CMD_SENDUI 		0xfffe
#define CMD_GETREMOTE	0xfffd
#define CMD_HANDSHAKE   0xfffc
#define CMD_HEARTBEAT   0xfff1 
//TcpClient.cpp
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <mstcpip.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")
#include <iostream>
using namespace std;
void recv_func(void *arg)
{
	char recvBuf[256];
	SOCKET sockClient = *(SOCKET *)arg;
	int len = 0;
	while (true){
		while (len = recv(sockClient, recvBuf, 1024, 0) > 0) {
			
			printf("�Է�˵�� %s\n", recvBuf + 4);
		}
		memset(recvBuf, 0, 256);
	}

}

SOCKET sockSrvInTimer = NULL;
//��ʱ�¼�  
void  CALLBACK TimeProc(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
	char send_buffer[512] = { 0 };
	int cur_dptr = 0;
	*(int *)send_buffer = CMD_HEARTBEAT;
	cur_dptr += 4;
	send_buffer[cur_dptr] = '\0';
	if(sockSrvInTimer)
		send(sockSrvInTimer, send_buffer, cur_dptr + 1, 0);
	printf("thread send heartbeat \n");
}

//�߳�  //CALLBACK
//unsigned __stdcall heartbeat_proc(void *pvoid)
unsigned  CALLBACK heartbeat_proc(PVOID arg)
{
	//ǿ��ϵͳΪ�߳̽�����Ϣ����  
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	sockSrvInTimer = *(SOCKET *)arg;
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
	int cur_dptr = 0;
	memset(buf, 0, 512);
	*(int *)buf = CMD_HANDSHAKE;
	strncpy(buf + 4, greets, strlen(greets));
	cur_dptr += 4 + strlen(greets);
	buf[cur_dptr] = '\0';

	return 0;
}

int ConstructSendUIPkt(char *buf, int *pktlen,const char *userId, const char *passwd)
{
	int cur_dptr = 0;
	memset(buf, 0, 512);
	*(int *)buf = CMD_SENDUI;
	cur_dptr += 4;
	strncpy(buf + cur_dptr, userId, 9);
	cur_dptr += 9;
	strncpy(buf + cur_dptr, passwd, 6);
	cur_dptr += 6;
	buf[cur_dptr] = '\0';
	*pktlen = cur_dptr + 1;

	return 0;
}

int ConstructGetRemotePkt(char *buf, int *pktlen, const char *userId, const char *passwd)
{
	int cur_dptr = 0;
	memset(buf, 0, 512);
	*(int *)buf = CMD_GETREMOTE;
	cur_dptr += 4;
	strncpy(buf + cur_dptr, userId, 9);
	cur_dptr += 9;
	strncpy(buf + cur_dptr, passwd, 6);
	cur_dptr += 6;
	buf[cur_dptr] = '\0';
	*pktlen = cur_dptr + 1;
	return 0;
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
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);//�����׽���
	SOCKADDR_IN addrSrv;
	inet_pton(AF_INET, SRV_IPADDR, &addrSrv.sin_addr);
	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); "49.152.49.84");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(8812);
	err = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//���ӵ�Ŀ������
	if (err == SOCKET_ERROR) 
		printf("Socket error\n");
	char recvBuf[100];
	char send_buffer[1024] = { 0 };//"Hello JQServer";
	char *greets = "Hello JQServer! I'm windows client";
	int pktlen = 0;
	ConstructHandShakePkt(send_buffer, &pktlen, greets);
	printf("Send handshake!\n");
	send(sockClient, send_buffer, pktlen, 0);

	_beginthread(recv_func, NULL, &sockClient);
	Sleep(5);
	//fixme
	ConstructSendUIPkt(send_buffer, &pktlen, "876543210", "CiGeWK");
	//cout << "Send userInfo!\n";
	//cout << "Send len "<< cur_dptr <<endl;
	printf("Send user info\n");
	printf("Send len %d\n", pktlen);
	if (SOCKET_ERROR == send(sockClient, send_buffer, pktlen, 0)) {
		printf("Error code: %d", WSAGetLastError());
	}
	printf("sockfd: %ld\n", sockClient);

	BOOL bKeepAlive = TRUE;
	int nRet = setsockopt(sockClient, SOL_SOCKET, SO_KEEPALIVE,
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
	nRet = WSAIoctl(sockClient, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in),
		&alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);
	if (nRet == SOCKET_ERROR)
	{
		printf("WSAIoctl failed: %d\n", WSAGetLastError());
		return FALSE;
	}
	 _beginthreadex(NULL, NULL, heartbeat_proc, &sockClient,0, NULL );

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
	while (1) {
		char ch = getchar();
		if (ch == 'q') break;
		switch (ch) {	
		case 'g':
			printf("Send command to get remote address\n");
			ConstructGetRemotePkt(send_buffer, &pktlen, "123456798", "WiGeWd");
			send(sockClient, send_buffer, pktlen, 0);
			break;
		default:
				break;

		}
	}
	
	closesocket(sockClient);//�ر��׽���
	WSACleanup();
	return 0;
}

