#include "stdafx.h"
#include "Manager.h"


CManager::CManager(CClientSocket *pClient)
{
	m_pClient = pClient;
	m_pClient->setManagerCallBack(this);
}

void CManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{

}

CManager::~CManager()
{
}
