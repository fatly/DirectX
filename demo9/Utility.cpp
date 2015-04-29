#include "Utility.h"
#include <tchar.h>
#include <stdlib.h>

void GetExecuteDirectory(TCHAR * pszPath, int nMaxCount)
{
	TCHAR path[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, path, MAX_PATH);

	size_t len = _tcslen(path);
	for (size_t pos = len - 1; pos >= 0; pos--)
	{
		if (path[pos] == '\\')
		{
			path[pos + 1] = '\0';
			break;
		}
	}

	_tcscpy_s(pszPath, nMaxCount, path);
}

void GetExecuteDirectory(char * pszPath, int nMaxCount)
{
	TCHAR path[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, path, MAX_PATH);
	size_t len = _tcslen(path);
	for (size_t pos = len - 1; pos >= 0; pos--)
	{
		if (path[pos] == '\\')
		{
			path[pos + 1] = '\0';
			break;
		}
	}
#ifdef UNICODE
	wcstombs_s(NULL, pszPath, MAX_PATH, path, MAX_PATH);
#else
	strncpy_s(pszPath, path, len);
#endif
}
