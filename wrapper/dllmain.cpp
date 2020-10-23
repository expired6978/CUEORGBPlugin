// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmeapi.h>
#include <dsound.h>
#include <tchar.h>
#include <cstdint>
#include <string>

#ifdef CHECK_PLUGIN
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

#include "MinHook.h"

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

uintptr_t FindPatternSimple(uintptr_t StartAddress, uintptr_t MaxSize, const BYTE* ByteMask, const char* Mask)
{
	auto compare = [](const BYTE* pData, const BYTE* bMask, const char* szMask)
	{
		for (; *szMask; ++szMask, ++pData, ++bMask)
		{
			if (*szMask == 'x' && *pData != *bMask)
				return false;
		}

		return *szMask == '\0';
	};

	const size_t maskLen = strlen(Mask);
	for (uintptr_t i = 0; i < MaxSize - maskLen; i++)
	{
		if (compare((BYTE*)(StartAddress + i), ByteMask, Mask))
			return StartAddress + i;
	}

	return 0;
}

#ifdef CHECK_PLUGIN
#define DEFINE_MEMBER_FN_LONG(className, functionName, retnType, ...)		\
	static std::uintptr_t _##functionName##_Address;						\
	typedef retnType (className::* _##functionName##_type)(__VA_ARGS__);	\
																			\
	inline _##functionName##_type * _##functionName##_GetPtr(void)			\
	{																		\
		return (_##functionName##_type *)&_##functionName##_Address;		\
	}

#define CALL_MEMBER_FN(obj, fn)	\
	((*(obj)).*(*((obj)->_##fn##_GetPtr())))

// We just need some memory to hold onto this, we don't actually care how big it is...
class QString
{
public:
	~QString()
	{
		CALL_MEMBER_FN(this, dtor)();
	}

	uintptr_t pad[5];

	DEFINE_MEMBER_FN_LONG(QString, toWCharArray, int, wchar_t*);
	DEFINE_MEMBER_FN_LONG(QString, dtor, void);
};

uintptr_t QString::_toWCharArray_Address = 0;
uintptr_t QString::_dtor_Address = 0;

class QIODevice
{
public:
	virtual ~QIODevice();

	virtual void Unk_01();
	virtual void Unk_02();
	virtual void Unk_03();
	virtual void Unk_04();
	virtual void Unk_05();
	virtual void Unk_06();
	virtual void Unk_07();
	virtual void Unk_08();
	virtual void Unk_09();
	virtual void Unk_0A();
	virtual void Unk_0B();
	virtual void Unk_0C();
	virtual void Unk_0D();
	virtual void Unk_0E();
	virtual void Unk_0F();
	virtual void Unk_10();
	virtual void Unk_11();
	virtual void Unk_12();
	virtual void Unk_13();
	virtual void Unk_14();
	virtual void Unk_15();
	virtual void Unk_16();
	virtual void Unk_17();
	virtual void Unk_18();
	virtual void Unk_19();
	virtual void Unk_1A();
};

class QFileDevice : public QIODevice
{
public:
	virtual ~QFileDevice();

	// Offset 0x6C
	virtual QString fileName() const;
};

class QFile : public QFileDevice
{
public:
	virtual ~QFile();
};
#else
struct QFile;
#endif

typedef bool (*_ValidateDevicePlugin)(QFile* device);
_ValidateDevicePlugin g_validateDevicePluginOriginal = nullptr;
bool ValidateDevicePlugin(QFile* file)
{
#ifdef CHECK_PLUGIN
	QString str = file->fileName();
	wchar_t path[MAX_PATH];
	int length = CALL_MEMBER_FN(&str, toWCharArray)(path);
	path[length] = 0;
	wchar_t* fileName = PathFindFileNameW(path);
	if (_wcsicmp(fileName, L"CUEORGBPlugin.dll") == 0)
	{
		return true;
	}
	return g_validateDevicePluginOriginal(file);
#else
	return true;
#endif
}

DWORD WINAPI CUEHookThread(LPVOID Arg)
{
#ifdef _DEBUG
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif

	HMODULE iCueDllModule = GetModuleHandle(_T("iCUE.dll"));
	uintptr_t begin = reinterpret_cast<uintptr_t>(iCueDllModule);

	const IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(iCueDllModule);
	const IMAGE_NT_HEADERS* ntHeader = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const std::uint8_t*>(dosHeader) + dosHeader->e_lfanew);
	uintptr_t end = begin + ntHeader->OptionalHeader.SizeOfCode;
	
	uintptr_t ValidatePluginSignature = FindPatternSimple(begin, (end - begin), (const BYTE*)"\x50\xE8\x00\x00\x00\x00\x83\xC4\x04\x84\xC0\x75\x63", "xx????xxxxxxx");
	if (ValidatePluginSignature)
	{
		// Read the address for the validate plugin function
		DWORD  oldProtect;
		VirtualProtect((LPVOID)(ValidatePluginSignature + 2), sizeof(std::int32_t), PAGE_EXECUTE_READ, &oldProtect);
		uintptr_t functionAddress = ValidatePluginSignature + *reinterpret_cast<std::int32_t*>(ValidatePluginSignature + 2) + 6;
		VirtualProtect((LPVOID)(ValidatePluginSignature + 2), sizeof(std::int32_t), oldProtect, &oldProtect);

#ifdef CHECK_PLUGIN
		// Read QString manipulation from Qt5Core
		HMODULE qtCoreModule = GetModuleHandle(_T("Qt5Core.dll"));
		QString::_toWCharArray_Address = (uintptr_t)GetProcAddress(qtCoreModule, "?toWCharArray@QString@@QBEHPA_W@Z");
		QString::_dtor_Address = (uintptr_t)GetProcAddress(qtCoreModule, "??1QString@@QAE@XZ");
#endif

		MH_STATUS status = MH_CreateHook((LPVOID)functionAddress, ValidateDevicePlugin, (LPVOID*)&g_validateDevicePluginOriginal);
		if (status == MH_OK)
		{
			MH_EnableHook(MH_ALL_HOOKS);
		}
	}
	
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

			MH_Initialize();

			CUEHookThread(nullptr);
			//CreateThread(nullptr, 0, CUEHookThread, nullptr, 0, nullptr);
			break;

		case DLL_PROCESS_DETACH:
			MH_Uninitialize();
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