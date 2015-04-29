#include "Utility.h"
#include <stdio.h>
#include <tchar.h>
#include <string.h>

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

	_tcsncpy_s(pszPath, nMaxCount, path, len);
}

void GetExecuteDirectoryA(char * pszPath, int nMaxCount)
{
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, path, MAX_PATH);
	size_t len = strlen(path);
	for (size_t pos = len - 1; pos >= 0; pos--)
	{
		if (path[pos] == '\\')
		{
			path[pos + 1] = '\0';
			break;
		}
	}

	strncpy_s(pszPath, nMaxCount, path, len);
}

void GetExecuteDirectoryW(wchar_t * pszPath, int nMaxCount)
{
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, path, MAX_PATH);
	size_t len = wcslen(path);
	for (size_t pos = len - 1; pos >= 0; pos--)
	{
		if (path[pos] == '\\')
		{
			path[pos + 1] = '\0';
			break;
		}
	}

	wcsncpy_s(pszPath, nMaxCount, path, len);
}

char * ReadFile(const char* fileName, int & length)
{
	FILE * fp = NULL;
	fopen_s(&fp, fileName, "rb");
	if (fp == NULL)
	{
		length = -1;
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	size_t fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char * buffer = (char*)malloc(fileSize);
	if (buffer == NULL)
	{
		length = -1;
		fclose(fp);
		return NULL;
	}

	if (fread(buffer, 1, fileSize, fp) != fileSize)
	{
		length = -1;
		fclose(fp);
		free(buffer);
		return NULL;
	}

	length = fileSize;

	fclose(fp);
	return buffer;
}