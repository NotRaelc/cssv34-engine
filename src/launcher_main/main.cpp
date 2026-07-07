//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: A redirection tool that allows the DLLs to reside elsewhere.
//
//=====================================================================================//

#if defined( _WIN32 ) && !defined( _X360 )
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <direct.h>
#include <tlhelp32.h>
#endif

#include "steam\steam_api.h"

//-----------------------------------------------------------------------------
// Purpose: Return the directory where this .exe is running from
// Output : char
//-----------------------------------------------------------------------------
char* GetBaseDir(const char* pszBuffer)
{
	static char	basedir[MAX_PATH];
	char szBuffer[MAX_PATH];
	size_t j;
	char* pBuffer = NULL;

	strcpy(szBuffer, pszBuffer);

	pBuffer = strrchr(szBuffer, '\\');
	if (pBuffer)
	{
		*(pBuffer + 1) = '\0';
	}

	strcpy(basedir, szBuffer);

	j = strlen(basedir);
	if (j > 0)
	{
		if ((basedir[j - 1] == '\\') ||
			(basedir[j - 1] == '/'))
		{
			basedir[j - 1] = 0;
		}
	}

	return basedir;
}

DWORD FindProcess(const char* processName) {
	DWORD pid = 0;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProcessSnap) return 0;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return 0;
	}

	do {
		// Compare the process name
		if (processName == pe32.szExeFile) {
			pid = pe32.th32ProcessID;
			break; // Found the process, break the loop
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	
	return pid;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Must add 'bin' to the path....
	char* pPath = getenv("PATH");

	// Use the .EXE name to determine the root directory
	char moduleName[MAX_PATH];
	char szBuffer[4096];
	if (!GetModuleFileName(hInstance, moduleName, MAX_PATH))
	{
		MessageBox(0, "Failed calling GetModuleFileName", "Launcher Error", MB_OK);
		return 0;
	}

	// Get the root directory the .exe is in
	char* pRootDir = GetBaseDir(moduleName);

#ifdef _DEBUG
	int len =
#endif
	_snprintf(szBuffer, sizeof(szBuffer), "PATH=%s\\bin\\;%s", pRootDir, pRootDir, pPath);
	szBuffer[sizeof(szBuffer) - 1] = '\0';
	assert(len < sizeof(szBuffer));
	_putenv(szBuffer);

	// Assemble the full path to our "launcher.dll"
	_snprintf(szBuffer, sizeof(szBuffer), "%s\\bin\\launcher.dll", pRootDir);
	szBuffer[sizeof(szBuffer) - 1] = '\0';

	// STEAM OK ... filesystem not mounted yet
#if defined(_X360)
	HINSTANCE launcher = LoadLibrary(szBuffer);
#else
	HINSTANCE launcher = LoadLibraryEx(szBuffer, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#endif

	if (!launcher)
	{
		char* pszError;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszError, 0, NULL);

		char szBuf[1024];
		_snprintf(szBuf, sizeof(szBuf), "Failed to load the launcher DLL:\n\n%s", pszError);
		szBuf[sizeof(szBuf) - 1] = '\0';
		MessageBox(0, szBuf, "Launcher Error", MB_OK);

		LocalFree(pszError);
		return 0;
	}

	// Loads Steam dlls
	// Note: if you for some reason see C++ Access Violation exceptions with steamclient.dll, that is absolutely normal.
	// It will not crash your game
	HMODULE hSteamDLL = LoadLibrary("steam.dll");
	if (hSteamDLL) {
		// Also, initialze SteamAPI to make everything work from start, not just after connecting to server.
		decltype(SteamAPI_Init)* SteamAPIInit = (decltype(SteamAPI_Init)*)GetProcAddress(LoadLibrary("steam_api.dll"), "SteamAPI_Init");
		if (!SteamAPIInit()) {
			MessageBox(0, "SteamAPI_Init failed or could not be executed.", "Launcher Error", MB_OK);
		}
	}

	typedef int(__cdecl* LauncherMainFn)(
		HINSTANCE,
		HINSTANCE,
		LPSTR,
		int
		);

	LauncherMainFn LauncherMain =
		(LauncherMainFn)GetProcAddress(launcher, "LauncherMain");

	return LauncherMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}


