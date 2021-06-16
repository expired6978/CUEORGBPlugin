// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmeapi.h>
#include <dsound.h>
#include <tchar.h>
#include <cstdint>
#include <string>

typedef HRESULT(WINAPI* DirectSoundCreateProc)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);
typedef HRESULT(WINAPI* DirectSoundEnumerateAProc)(LPDSENUMCALLBACKA, LPVOID);
typedef HRESULT(WINAPI* DirectSoundEnumerateWProc)(LPDSENUMCALLBACKW, LPVOID);
typedef HRESULT(WINAPI* DllCanUnloadNowProc)();
typedef	HRESULT(WINAPI* DllGetClassObjectProc)(REFCLSID, REFIID, LPVOID*);
typedef HRESULT(WINAPI* DirectSoundCaptureCreateProc)(LPCGUID, LPDIRECTSOUNDCAPTURE*, LPUNKNOWN);
typedef HRESULT(WINAPI* DirectSoundCaptureEnumerateAProc)(LPDSENUMCALLBACKA, LPVOID);
typedef HRESULT(WINAPI* DirectSoundCaptureEnumerateWProc)(LPDSENUMCALLBACKW, LPVOID);
typedef HRESULT(WINAPI* GetDeviceIDProc)(LPCGUID, LPGUID);
typedef HRESULT(WINAPI* DirectSoundFullDuplexCreateProc)(LPCGUID, LPCGUID, LPCDSCBUFFERDESC, LPCDSBUFFERDESC, HWND, DWORD, LPDIRECTSOUNDFULLDUPLEX*, LPDIRECTSOUNDCAPTUREBUFFER8*, LPDIRECTSOUNDBUFFER8*, LPUNKNOWN);
typedef HRESULT(WINAPI* DirectSoundCreate8Proc)(LPCGUID, LPDIRECTSOUND8*, LPUNKNOWN);
typedef HRESULT(WINAPI* DirectSoundCaptureCreate8Proc)(LPCGUID, LPDIRECTSOUNDCAPTURE8*, LPUNKNOWN);

DirectSoundCreateProc m_pDirectSoundCreate;
DirectSoundEnumerateAProc m_pDirectSoundEnumerateA;
DirectSoundEnumerateWProc m_pDirectSoundEnumerateW;
DllCanUnloadNowProc m_pDllCanUnloadNow;
DllGetClassObjectProc m_pDllGetClassObject;
DirectSoundCaptureCreateProc m_pDirectSoundCaptureCreate;
DirectSoundCaptureEnumerateAProc m_pDirectSoundCaptureEnumerateA;
DirectSoundCaptureEnumerateWProc m_pDirectSoundCaptureEnumerateW;
GetDeviceIDProc m_pGetDeviceID;
DirectSoundFullDuplexCreateProc m_pDirectSoundFullDuplexCreate;
DirectSoundCreate8Proc m_pDirectSoundCreate8;
DirectSoundCaptureCreate8Proc m_pDirectSoundCaptureCreate8;

DWORD WINAPI CUEHookThread(LPVOID Arg)
{
#ifdef _DEBUG
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif

	HMODULE winTrustModule = GetModuleHandle(_T("WINTRUST.dll"));
	FARPROC winVerifyTrust = GetProcAddress(winTrustModule, "WinVerifyTrust");

	BYTE bypass[] = { 0x31, 0xC0, 0xC3 };
	// xor eax, eax
	// ret

	DWORD d, ds;
	VirtualProtect((LPVOID)winVerifyTrust, 1, PAGE_EXECUTE_READWRITE, &d);
	memcpy((PBYTE)winVerifyTrust, bypass, sizeof(bypass));
	VirtualProtect((LPVOID)winVerifyTrust, 1, d, &ds);
		
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	static HMODULE dsounddll;

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// Load dll
			TCHAR path[MAX_PATH];
			GetSystemDirectory(path, MAX_PATH);
			_tcscat_s(path, _T("\\dsound.dll"));
			dsounddll = LoadLibrary(path);

			// Get function addresses
			m_pDirectSoundCreate = (DirectSoundCreateProc)GetProcAddress(dsounddll, "DirectSoundCreate");
			m_pDirectSoundEnumerateA = (DirectSoundEnumerateAProc)GetProcAddress(dsounddll, "DirectSoundEnumerateA");
			m_pDirectSoundEnumerateW = (DirectSoundEnumerateWProc)GetProcAddress(dsounddll, "DirectSoundEnumerateW");
			m_pDllCanUnloadNow = (DllCanUnloadNowProc)GetProcAddress(dsounddll, "DllCanUnloadNow");
			m_pDllGetClassObject = (DllGetClassObjectProc)GetProcAddress(dsounddll, "DllGetClassObject");
			m_pDirectSoundCaptureCreate = (DirectSoundCaptureCreateProc)GetProcAddress(dsounddll, "DirectSoundCaptureCreate");
			m_pDirectSoundCaptureEnumerateA = (DirectSoundCaptureEnumerateAProc)GetProcAddress(dsounddll, "DirectSoundCaptureEnumerateA");
			m_pDirectSoundCaptureEnumerateW = (DirectSoundCaptureEnumerateWProc)GetProcAddress(dsounddll, "DirectSoundCaptureEnumerateW");
			m_pGetDeviceID = (GetDeviceIDProc)GetProcAddress(dsounddll, "GetDeviceID");
			m_pDirectSoundFullDuplexCreate = (DirectSoundFullDuplexCreateProc)GetProcAddress(dsounddll, "DirectSoundFullDuplexCreate");
			m_pDirectSoundCreate8 = (DirectSoundCreate8Proc)GetProcAddress(dsounddll, "DirectSoundCreate8");
			m_pDirectSoundCaptureCreate8 = (DirectSoundCaptureCreate8Proc)GetProcAddress(dsounddll, "DirectSoundCaptureCreate8");

			CUEHookThread(nullptr);
			//CreateThread(nullptr, 0, CUEHookThread, nullptr, 0, nullptr);
			break;

		case DLL_PROCESS_DETACH:
			FreeLibrary(dsounddll);
			break;
	}

	return TRUE;
}

HRESULT WINAPI DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
{
	if (!m_pDirectSoundCreate)
	{
		return E_FAIL;
	}

	return m_pDirectSoundCreate(pcGuidDevice, ppDS, pUnkOuter);
}

HRESULT WINAPI DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
	if (!m_pDirectSoundEnumerateA)
	{
		return E_FAIL;
	}

	return m_pDirectSoundEnumerateA(pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
	if (!m_pDirectSoundEnumerateW)
	{
		return E_FAIL;
	}

	return m_pDirectSoundEnumerateW(pDSEnumCallback, pContext);
}

HRESULT WINAPI DllCanUnloadNow()
{
	if (!m_pDllCanUnloadNow)
	{
		return E_FAIL;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	if (!m_pDllGetClassObject)
	{
		return E_FAIL;
	}

	return m_pDllGetClassObject(rclsid, riid, ppv);
}

HRESULT WINAPI DirectSoundCaptureCreate(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE* ppDSC, LPUNKNOWN pUnkOuter)
{
	if (!m_pDirectSoundCaptureCreate)
	{
		return E_FAIL;
	}

	return m_pDirectSoundCaptureCreate(pcGuidDevice, ppDSC, pUnkOuter);
}

HRESULT WINAPI DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
	if (!m_pDirectSoundCaptureEnumerateA)
	{
		return E_FAIL;
	}

	return m_pDirectSoundCaptureEnumerateA(pDSEnumCallback, pContext);
}

HRESULT WINAPI DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback, LPVOID pContext)
{
	if (!m_pDirectSoundCaptureEnumerateW)
	{
		return E_FAIL;
	}

	return m_pDirectSoundCaptureEnumerateW(pDSEnumCallback, pContext);
}

HRESULT WINAPI GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest)
{
	return m_pGetDeviceID(pGuidSrc, pGuidDest);
}

HRESULT WINAPI DirectSoundFullDuplexCreate(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
	DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8* ppDSCBuffer8, LPDIRECTSOUNDBUFFER8* ppDSBuffer8, LPUNKNOWN pUnkOuter)
{
	if (!m_pDirectSoundFullDuplexCreate)
	{
		return E_FAIL;
	}

	return m_pDirectSoundFullDuplexCreate(pcGuidCaptureDevice, pcGuidRenderDevice, pcDSCBufferDesc, pcDSBufferDesc, hWnd, dwLevel, ppDSFD, ppDSCBuffer8, ppDSBuffer8, pUnkOuter);
}

HRESULT WINAPI DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8* ppDS8, LPUNKNOWN pUnkOuter)
{
	if (!m_pDirectSoundCreate8)
	{
		return E_FAIL;
	}

	return m_pDirectSoundCreate8(pcGuidDevice, ppDS8, pUnkOuter);
}

HRESULT WINAPI DirectSoundCaptureCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8* ppDSC8, LPUNKNOWN pUnkOuter)
{
	if (!m_pDirectSoundCaptureCreate8)
	{
		return E_FAIL;
	}

	return m_pDirectSoundCaptureCreate8(pcGuidDevice, ppDSC8, pUnkOuter);
}