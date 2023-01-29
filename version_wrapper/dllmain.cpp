// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <cstdint>
#include <string>

#include <winver.h>

// Function Name     : GetFileVersionInfoA
// Ordinal           : 1 (0x1)
namespace P { BOOL(WINAPI* GetFileVersionInfoA)(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData); }
extern "C"    BOOL WINAPI  GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
	return  P::GetFileVersionInfoA(lptstrFilename, dwHandle, dwLen, lpData);
}



// Function Name     : GetFileVersionInfoByHandle
// Ordinal           : 2 (0x2)
namespace P { int(WINAPI* GetFileVersionInfoByHandle)(int hMem, LPCWSTR lpFileName, int v2, int v3); }
extern "C"    int WINAPI  GetFileVersionInfoByHandle(int hMem, LPCWSTR lpFileName, int v2, int v3) {
	return P::GetFileVersionInfoByHandle(hMem, lpFileName, v2, v3);
}


// Function Name     : GetFileVersionInfoExA
// Ordinal           : 3 (0x3)
namespace P { BOOL(WINAPI* GetFileVersionInfoExA)(DWORD dwFlags, LPCSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData); }
extern "C"    BOOL WINAPI  GetFileVersionInfoExA(DWORD dwFlags, LPCSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
	return  P::GetFileVersionInfoExA(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
}


// Function Name     : GetFileVersionInfoExW
// Ordinal           : 4 (0x4)
namespace P { BOOL(WINAPI* GetFileVersionInfoExW)(DWORD dwFlags, LPCWSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData); }
extern "C"    BOOL WINAPI  GetFileVersionInfoExW(DWORD dwFlags, LPCWSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
	return  P::GetFileVersionInfoExW(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
}


// Function Name     : GetFileVersionInfoSizeA
// Ordinal           : 5 (0x5)
namespace P { DWORD(WINAPI* GetFileVersionInfoSizeA)(LPCSTR lptstrFilename, LPDWORD lpdwHandle); }
extern "C"    DWORD WINAPI  GetFileVersionInfoSizeA(LPCSTR lptstrFilename, LPDWORD lpdwHandle) {
	return  P::GetFileVersionInfoSizeA(lptstrFilename, lpdwHandle);
}


// Function Name     : GetFileVersionInfoSizeExA
// Ordinal           : 6 (0x6)
namespace P { DWORD(WINAPI* GetFileVersionInfoSizeExA)(DWORD dwFlags, LPCSTR lpwstrFilename, LPDWORD lpdwHandle); }
extern "C"    DWORD WINAPI  GetFileVersionInfoSizeExA(DWORD dwFlags, LPCSTR lpwstrFilename, LPDWORD lpdwHandle) {
	return   P::GetFileVersionInfoSizeExA(dwFlags, lpwstrFilename, lpdwHandle);
}


// Function Name     : GetFileVersionInfoSizeExW
// Ordinal           : 7 (0x7)
#undef F
#define F GetFileVersionInfoSizeExW
namespace P { DWORD(WINAPI* GetFileVersionInfoSizeExW)(DWORD dwFlags, LPCWSTR lpwstrFilename, LPDWORD lpdwHandle); }
extern "C"    DWORD  WINAPI  GetFileVersionInfoSizeExW(DWORD dwFlags, LPCWSTR lpwstrFilename, LPDWORD lpdwHandle) {
	return    P::GetFileVersionInfoSizeExW(dwFlags, lpwstrFilename, lpdwHandle);
}


// Function Name     : GetFileVersionInfoSizeW
// Ordinal           : 8 (0x8)
namespace P { DWORD(WINAPI* GetFileVersionInfoSizeW)(LPCWSTR lptstrFilename, LPDWORD lpdwHandle); }
extern "C"    DWORD WINAPI   GetFileVersionInfoSizeW(LPCWSTR lptstrFilename, LPDWORD lpdwHandle) {
	return    P::GetFileVersionInfoSizeW(lptstrFilename, lpdwHandle);
}


// Function Name     : GetFileVersionInfoW
// Ordinal           : 9 (0x9)
namespace P { BOOL(WINAPI* GetFileVersionInfoW)(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData); }
extern "C"    BOOL  WINAPI  GetFileVersionInfoW(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
	return   P::GetFileVersionInfoW(lptstrFilename, dwHandle, dwLen, lpData);
}


// Function Name     : VerFindFileA
// Ordinal           : 10 (0xa)
namespace P { DWORD(WINAPI* VerFindFileA)(DWORD uFlags, LPCSTR szFileName, LPCSTR szWinDir, LPCSTR szAppDir, LPSTR szCurDir, PUINT lpuCurDirLen, LPSTR szDestDir, PUINT lpuDestDirLen); }
extern "C"    DWORD  WINAPI  VerFindFileA(DWORD uFlags, LPCSTR szFileName, LPCSTR szWinDir, LPCSTR szAppDir, LPSTR szCurDir, PUINT lpuCurDirLen, LPSTR szDestDir, PUINT lpuDestDirLen) {
	return    P::VerFindFileA(uFlags, szFileName, szWinDir, szAppDir, szCurDir, lpuCurDirLen, szDestDir, lpuDestDirLen);
}


// Function Name     : VerFindFileW
// Ordinal           : 11 (0xb)
namespace P { DWORD(WINAPI* VerFindFileW)(DWORD uFlags, LPCWSTR szFileName, LPCWSTR szWinDir, LPCWSTR szAppDir, LPWSTR szCurDir, PUINT lpuCurDirLen, LPWSTR szDestDir, PUINT lpuDestDirLen); }
extern "C"    DWORD  WINAPI  VerFindFileW(DWORD uFlags, LPCWSTR szFileName, LPCWSTR szWinDir, LPCWSTR szAppDir, LPWSTR szCurDir, PUINT lpuCurDirLen, LPWSTR szDestDir, PUINT lpuDestDirLen) {
	return    P::VerFindFileW(uFlags, szFileName, szWinDir, szAppDir, szCurDir, lpuCurDirLen, szDestDir, lpuDestDirLen);
}


// Function Name     : VerInstallFileA
// Ordinal           : 12 (0xc)
namespace P { DWORD(WINAPI* VerInstallFileA)(DWORD uFlags, LPCSTR szSrcFileName, LPCSTR szDestFileName, LPCSTR szSrcDir, LPCSTR szDestDir, LPCSTR szCurDir, LPSTR szTmpFile, PUINT lpuTmpFileLen); }
extern "C"    DWORD  WINAPI  VerInstallFileA(DWORD uFlags, LPCSTR szSrcFileName, LPCSTR szDestFileName, LPCSTR szSrcDir, LPCSTR szDestDir, LPCSTR szCurDir, LPSTR szTmpFile, PUINT lpuTmpFileLen) {
	return    P::VerInstallFileA(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, lpuTmpFileLen);
}


// Function Name     : VerInstallFileW
// Ordinal           : 13 (0xd)
namespace P { DWORD(WINAPI* VerInstallFileW)(DWORD uFlags, LPCWSTR szSrcFileName, LPCWSTR szDestFileName, LPCWSTR szSrcDir, LPCWSTR szDestDir, LPCWSTR szCurDir, LPWSTR szTmpFile, PUINT lpuTmpFileLen); }
extern "C"    DWORD  WINAPI  VerInstallFileW(DWORD uFlags, LPCWSTR szSrcFileName, LPCWSTR szDestFileName, LPCWSTR szSrcDir, LPCWSTR szDestDir, LPCWSTR szCurDir, LPWSTR szTmpFile, PUINT lpuTmpFileLen) {
	return    P::VerInstallFileW(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, lpuTmpFileLen);
}


// Function Name     : VerLanguageNameA
// Ordinal           : 14 (0xe)
namespace P { DWORD(WINAPI* VerLanguageNameA)(DWORD wLang, LPSTR szLang, DWORD cchLang); }
extern "C"    DWORD  WINAPI  VerLanguageNameA(DWORD wLang, LPSTR szLang, DWORD cchLang) {
	return    P::VerLanguageNameA(wLang, szLang, cchLang);
}


// Function Name     : VerLanguageNameW
// Ordinal           : 15 (0xf)
namespace P { DWORD(WINAPI* VerLanguageNameW)(DWORD wLang, LPWSTR szLang, DWORD cchLang); }
extern "C"    DWORD  WINAPI  VerLanguageNameW(DWORD wLang, LPWSTR szLang, DWORD cchLang) {
	return    P::VerLanguageNameW(wLang, szLang, cchLang);
}


// Function Name     : VerQueryValueA
// Ordinal           : 16 (0x10)
namespace P { BOOL(WINAPI* VerQueryValueA)(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen); }
extern "C"    BOOL  WINAPI  VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen) {
	return   P::VerQueryValueA(pBlock, lpSubBlock, lplpBuffer, puLen);
}


// Function Name     : VerQueryValueW
// Ordinal           : 17 (0x11)
namespace P { BOOL(WINAPI* VerQueryValueW)(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen); }
extern "C"    BOOL  WINAPI  VerQueryValueW(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID * lplpBuffer, PUINT puLen) {
	return   P::VerQueryValueW(pBlock, lpSubBlock, lplpBuffer, puLen);
}

template<typename T>
void setup(T*& funcPtr, HMODULE library, const char* funcName) {
	if (funcPtr != nullptr) {
		return;
	}
	funcPtr = reinterpret_cast<T*>(GetProcAddress(library, funcName));
}

#define ASSIGN_PROC(Name, library) \
	setup(P::##Name, library, #Name);


DWORD WINAPI CUEHookThread(LPVOID Arg)
{
#ifdef _DEBUG
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif

	HMODULE winTrustModule = GetModuleHandle(_T("WINTRUST.dll"));
	if (winTrustModule) {
		FARPROC winVerifyTrust = GetProcAddress(winTrustModule, "WinVerifyTrust");
		if (winVerifyTrust) {
			BYTE bypass[] = { 0x31, 0xC0, 0xC3 };
			// xor eax, eax
			// ret

			DWORD d, ds;
			VirtualProtect((LPVOID)winVerifyTrust, 1, PAGE_EXECUTE_READWRITE, &d);
			memcpy((PBYTE)winVerifyTrust, bypass, sizeof(bypass));
			VirtualProtect((LPVOID)winVerifyTrust, 1, d, &ds);
		}
	}
		
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static HMODULE versiondll;

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// Load dll
			TCHAR path[MAX_PATH];
			GetSystemDirectory(path, MAX_PATH);
			_tcscat_s(path, _T("\\version.dll"));
			versiondll = LoadLibrary(path);

			ASSIGN_PROC(GetFileVersionInfoA, versiondll);
			ASSIGN_PROC(GetFileVersionInfoByHandle, versiondll)
			ASSIGN_PROC(GetFileVersionInfoExA, versiondll)
			ASSIGN_PROC(GetFileVersionInfoExW, versiondll)
			ASSIGN_PROC(GetFileVersionInfoSizeA, versiondll)
			ASSIGN_PROC(GetFileVersionInfoSizeExA, versiondll)
			ASSIGN_PROC(GetFileVersionInfoSizeExW, versiondll)
			ASSIGN_PROC(GetFileVersionInfoSizeW, versiondll)
			ASSIGN_PROC(GetFileVersionInfoW, versiondll)
			ASSIGN_PROC(VerFindFileA, versiondll)
			ASSIGN_PROC(VerFindFileW, versiondll)
			ASSIGN_PROC(VerInstallFileA, versiondll)
			ASSIGN_PROC(VerInstallFileW, versiondll)
			ASSIGN_PROC(VerLanguageNameA, versiondll)
			ASSIGN_PROC(VerLanguageNameW, versiondll)
			ASSIGN_PROC(VerQueryValueA, versiondll)
			ASSIGN_PROC(VerQueryValueW, versiondll)

			CUEHookThread(nullptr);
			//CreateThread(nullptr, 0, CUEHookThread, nullptr, 0, nullptr);
			break;

		case DLL_PROCESS_DETACH:
			FreeLibrary(versiondll);
			break;
	}

	return TRUE;
}
