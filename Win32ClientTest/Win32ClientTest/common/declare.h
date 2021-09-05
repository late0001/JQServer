#pragma once
#include <afxwin.h>

typedef HANDLE(WINAPI *CreateEventAT)
(
	__in_opt LPSECURITY_ATTRIBUTES lpEventAttributes,
	__in     BOOL bManualReset,
	__in     BOOL bInitialState,
	__in_opt LPCSTR lpName
	);

typedef BOOL
(WINAPI
	*CloseHandleT)(
		__in HANDLE hObject
		);

typedef BOOL
(WINAPI
	*SetEventT)(
		__in HANDLE hEvent
		);

typedef HDESK
(WINAPI
	*OpenInputDesktopT)(
		__in DWORD dwFlags,
		__in BOOL fInherit,
		__in ACCESS_MASK dwDesiredAccess);


typedef HDESK
(WINAPI
	*OpenDesktopAT)(
		__in LPCSTR lpszDesktop,
		__in DWORD dwFlags,
		__in BOOL fInherit,
		__in ACCESS_MASK dwDesiredAccess);

typedef BOOL
(WINAPI
	*CloseDesktopT)(
		__in HDESK hDesktop);

typedef DWORD
(WINAPI
	*GetCurrentThreadIdT)(
		VOID
		);

typedef HDESK
(WINAPI
	*GetThreadDesktopT)(
		__in DWORD dwThreadId);

typedef BOOL
(WINAPI
	*GetUserObjectInformationAT)(
		__in HANDLE hObj,
		__in int nIndex,
		__out_bcount_opt(nLength) PVOID pvInfo,
		__in DWORD nLength,
		__out_opt LPDWORD lpnLengthNeeded);

typedef BOOL
(WINAPI
	*SetThreadDesktopT)(
		__in HDESK hDesktop);

typedef BOOL
(WINAPI
	*ResetEventT)(
		__in HANDLE hEvent
		);

typedef VOID
(WINAPI
	*SleepT)(
		__in DWORD dwMilliseconds
		);

typedef DWORD
(WINAPI
	*WaitForSingleObjectT)(
		__in HANDLE hHandle,
		__in DWORD dwMilliseconds
		);

typedef LONG
(APIENTRY
	*RegOpenKeyAT)(
		__in HKEY hKey,
		__in_opt LPCSTR lpSubKey,
		__out PHKEY phkResult
		);

typedef LONG
(APIENTRY
	*RegQueryValueExAT)(
		__in HKEY hKey,
		__in_opt LPCSTR lpValueName,
		__reserved LPDWORD lpReserved,
		__out_opt LPDWORD lpType,
		__out_bcount_opt(*lpcbData) LPBYTE lpData,
		__inout_opt LPDWORD lpcbData
		);

typedef LONG
(APIENTRY
	*RegCloseKeyT)(
		__in HKEY hKey
		);

typedef LONG
(WINAPI
	*InterlockedExchangeT)(
		__inout LONG volatile *Target,
		__in    LONG Value
		);

typedef HANDLE(WINAPI *CreateThreadT_II)
(
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	DWORD dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags,
	LPDWORD lpThreadId
	);

typedef LPSTR
(WINAPI
	*lstrcpyAT)(
		__out LPSTR lpString1,
		__in  LPCSTR lpString2
		);