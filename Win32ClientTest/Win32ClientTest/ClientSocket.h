#pragma once

class CManager;
class CClientSocket
{
	//friend class CManager;
public:
	CClientSocket();
	~CClientSocket();
	bool InitSocket(LPCSTR lpszHost, UINT nPort);
	void setManagerCallBack(CManager *pManager);
	bool IsRunning();
	void StartReceiving();
	static DWORD WINAPI WorkThread(LPVOID lparam);
	int Send(LPBYTE lpData, UINT nSize, _In_reads_bytes_(tolen) const struct sockaddr FAR * to = NULL,
		_In_ int tolen = NULL);
	void Close();
	void OnRead(LPBYTE lpBuffer, DWORD dwIoSize);
	void RunEventLoop();

	HANDLE m_hWorkerThread;
	SOCKET m_Socket;
	HANDLE m_hEvent;
private:
	int SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize);
	int SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize,
		_In_reads_bytes_(tolen) const struct sockaddr FAR * to,
		_In_ int tolen);
	sockaddr_in	ClientAddr;
	bool m_bIsRunning;
	CManager	*m_pManager;
};

