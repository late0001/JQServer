#pragma once
#include "Manager.h"
class CScreenManager :
	public CManager
{
public:
	CScreenManager(CClientSocket *pClient);
	virtual ~CScreenManager();
};

