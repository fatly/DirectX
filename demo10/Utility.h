#ifndef __CORE_UTILITY_H__
#define __CORE_UTILITY_H__

#include <windows.h>

void GetExecuteDirectory(TCHAR * pszPath, int nMaxCount);
void GetExecuteDirectoryA(char * pszPath, int nMaxCount);
void GetExecuteDirectoryW(wchar_t * pszPath, int nMaxCount);

#endif
