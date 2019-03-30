#pragma once

#include <windows.h>

typedef BOOL(WINAPI *EnumerateFunc) (LPCWSTR lpFileOrPath, void* pUserData);

inline void doFileEnumeration(LPCWSTR lpPath, BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData)
{
	static BOOL s_bUserBreak = FALSE;

	if (s_bUserBreak) return;

	if (lpPath == nullptr) return;

	const int len = lstrlen(lpPath);

	//NotifySys(NRS_DO_EVENTS, 0,0);

	TCHAR path[MAX_PATH];
	lstrcpy(path, lpPath);
	if (lpPath[len - 1] != '\\') lstrcat(path, L"\\");
	lstrcat(path, L"*");

	WIN32_FIND_DATA fd;
	HANDLE hFindFile = FindFirstFile(path, &fd);
	if (hFindFile == INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
		return;
	}

	TCHAR tempPath[MAX_PATH];

	BOOL bFinish = FALSE;
	while (!bFinish)
	{
		lstrcpy(tempPath, lpPath);
		if (lpPath[len - 1] != '\\') lstrcat(tempPath, L"\\");
		lstrcat(tempPath, fd.cFileName);

		const BOOL bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

		//如果是.或..
		if (bIsDirectory
			&& (lstrcmp(fd.cFileName, L".") == 0 || lstrcmp(fd.cFileName, L"..") == 0))
		{
			bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
			continue;
		}

		if (pFunc && bEnumFiles != bIsDirectory)
		{
			const BOOL bUserReture = pFunc(tempPath, pUserData);
			if (bUserReture == FALSE)
			{
				s_bUserBreak = TRUE; ::FindClose(hFindFile);
				return;
			}
		}

		//NotifySys(NRS_DO_EVENTS, 0,0);

		if (bIsDirectory && bRecursion) //是子目录
		{
			doFileEnumeration(tempPath, bRecursion, bEnumFiles, pFunc, pUserData);
		}

		bFinish = (FindNextFile(hFindFile, &fd) == FALSE);
	}

	FindClose(hFindFile);
}
