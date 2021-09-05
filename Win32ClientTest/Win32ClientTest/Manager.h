#pragma once
#include "ClientSocket.h"
class CManager
{
	friend class CClientSocket;
public:
	CManager(CClientSocket *pClient);
	~CManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

	CClientSocket	*m_pClient;
};

