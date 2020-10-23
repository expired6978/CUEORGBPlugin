// dllmain.cpp : Defines the entry point for the DLL application.

#include <cstdint>
#include <cstring>
#include <stdlib.h>

#include "sha256/picosha2.h"

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <fstream>
#include <tchar.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

#include "CorsairPluginDeviceManager.h"

#ifdef _DEBUG
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <debugapi.h>

void OutputDebugMessage(const char* lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	char szBuffer[512]; // get rid of this hard-coded buffer
	nBuf = _vsnprintf_s(szBuffer, 511, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	va_end(args);
}
#else
#define OutputDebugMessage(...)
#endif

std::unique_ptr<CorsairPluginDeviceManager> g_deviceManager;

std::wstring GetLocalFile(const std::wstring& relativePath)
{
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW((HINSTANCE)&__ImageBase, path, MAX_PATH);
	PathRemoveFileSpecW(path);
	PathCombineW(path, path, relativePath.c_str());
	return path;
}

std::string GetImageHash(const std::string& relativePath)
{
	size_t reqLength = ::MultiByteToWideChar(CP_UTF8, 0, relativePath.c_str(), (std::int32_t)relativePath.length(), 0, 0);
	std::wstring ret(reqLength, L'\0');
	::MultiByteToWideChar(CP_UTF8, 0, relativePath.c_str(), (std::int32_t)relativePath.length(), &ret[0], (std::int32_t)ret.length());
	std::ifstream f(GetLocalFile(ret.c_str()).c_str(), std::ios::binary);
	std::vector<unsigned char> s(picosha2::k_digest_size);
	picosha2::hash256(f, s.begin(), s.end());
	return picosha2::bytes_to_hex_string(s);
}

CorsairPluginDeviceInfo* CorsairPluginGetDeviceInfo(const char* deviceId)
{
	OutputDebugMessage("CorsairPluginGetDeviceInfo: %s", deviceId);
	if (g_deviceManager)
	{
		return g_deviceManager->GetDeviceInfo(deviceId);
	}

	return nullptr;
}

bool CorsairSetLedsColors(const char* deviceId, std::int32_t size, CorsairLedColor* ledsColors)
{
	if (g_deviceManager)
	{
		return g_deviceManager->SetColor(deviceId, size, ledsColors);
	}
	
	return false;
}

void CorsairSetMode(std::int32_t mode)
{
	// Not sure exactly what modes do yet, direct or corsair link?
	OutputDebugMessage("CorsairSetMode: %d", mode);
}

bool CorsairConfigureKeyEvent(void* unk1, std::int32_t unk2)
{
	OutputDebugMessage("CorsairConfigureKeyEvent: %08X - %d", unk1, unk2);
	return false;
}

bool CorsairSubscribeForEvents(CorsairEventHandler onEvent, void* context)
{
	OutputDebugMessage("CorsairSubscribeForEvents: %08X - %08X", onEvent, context);
	return false;
}

bool CorsairUnsubscribeFromEvents()
{
	OutputDebugMessage("CorsairUnsubscribeFromEvents");
	return false;
}

CorsairPluginDeviceView* CorsairPluginGetDeviceView(const char* deviceId, std::int32_t viewId)
{
	OutputDebugMessage("CorsairPluginGetDeviceView: %s - %d", deviceId, viewId);
	if (g_deviceManager)
	{
		return g_deviceManager->GetDeviceView(deviceId, viewId);
	}

	return nullptr;
}

void CorsairPluginFreeDeviceInfo(CorsairPluginDeviceInfo* deviceInfo)
{
	OutputDebugMessage("CorsairPluginFreeDeviceInfo: %08X", deviceInfo);

	if (deviceInfo->ledPositions)
	{
		if (deviceInfo->ledPositions->ledPosition)
		{
			free(deviceInfo->ledPositions->ledPosition);
		}
		
		free(deviceInfo->ledPositions);
	}
	
	if (deviceInfo->promoImage)
	{
		if (deviceInfo->promoImage->hash)
		{
			free(deviceInfo->promoImage->hash);
		}
		if (deviceInfo->promoImage->path)
		{
			free(deviceInfo->promoImage->path);
		}
		
		free(deviceInfo->promoImage);
	}
	
	if (deviceInfo->thumbnail)
	{
		if (deviceInfo->thumbnail->hash)
		{
			free(deviceInfo->thumbnail->hash);
		}
		
		if (deviceInfo->thumbnail->path)
		{
			free(deviceInfo->thumbnail->path);
		}
		
		free(deviceInfo->thumbnail);
	}
	
	if (deviceInfo->deviceId)
	{
		free(deviceInfo->deviceId);
	}
	
	if (deviceInfo->deviceName)
	{
		free(deviceInfo->deviceName);
	}

	free(deviceInfo);
}

void CorsairPluginFreeDeviceView(CorsairPluginDeviceView* deviceView)
{
	OutputDebugMessage("CorsairPluginFreeDeviceView: %08X", deviceView);

	if (deviceView->ledView)
	{
		if (deviceView->ledView->view)
		{
			for (std::int32_t i = 0; i < deviceView->ledView->numberOfLed; ++i)
			{
				if (deviceView->ledView->view[i].path)
				{
					free(deviceView->ledView->view[i].path);
				}
				if (deviceView->ledView->view[i].text)
				{
					free(deviceView->ledView->view[i].text);
				}
			}

			free(deviceView->ledView->view);
		}
		
		free(deviceView->ledView);
	}
	
	if (deviceView->mask)
	{
		if (deviceView->mask->hash)
		{
			free(deviceView->mask->hash);
		}
		
		if (deviceView->mask->path)
		{
			free(deviceView->mask->path);
		}
		
		free(deviceView->mask);
	}
	
	if (deviceView->view)
	{
		if (deviceView->view->hash)
		{
			free(deviceView->view->hash);
		}

		if (deviceView->view->path)
		{
			free(deviceView->view->path);
		}
		free(deviceView->view);
	}
	

	free(deviceView);
}

void CorsairSubscribeForDeviceConnectionStatusChanges(void* context, _DeviceConnectionStatusChangeCallback deviceStatusCallback)
{
	OutputDebugMessage("CorsairSubscribeForDeviceConnectionStatusChanges: %08X - %08X", context, deviceStatusCallback);

	g_deviceManager = std::make_unique<CorsairPluginDeviceManager>(context, deviceStatusCallback, GetImageHash, GetLocalFile);
	g_deviceManager->Start();
}

void CorsairPluginUnsubscribeFromDeviceStatusChanges()
{
	OutputDebugMessage("CorsairPluginUnsubscribeFromDeviceStatusChanges");

	g_deviceManager.reset();
}

extern "C"
{
void CorsairPluginFreeInstance(CorsairGetInstance* instance)
{
	OutputDebugMessage("CorsairPluginFreeInstance: %08X", instance);
	free(instance);
}

std::int32_t CorsairPluginGetAPIVersion()
{
	OutputDebugMessage("CorsairPluginGetAPIVersion");
	return 0x66;
}

std::int32_t CorsairPluginGetFeatures()
{
	OutputDebugMessage("CorsairPluginGetFeatures");
	return 4;
}

CorsairGetInstance* CorsairPluginGetInstance()
{
	CorsairGetInstance* instance = static_cast<CorsairGetInstance*>(malloc(sizeof(CorsairGetInstance)));
	instance->CorsairPluginGetDeviceInfo = CorsairPluginGetDeviceInfo;
	instance->CorsairPluginSetLedsColors = CorsairSetLedsColors;
	instance->CorsairSubscribeForDeviceConnectionStatusChanges = CorsairSubscribeForDeviceConnectionStatusChanges;
	instance->CorsairPluginUnsubscribeFromDeviceStatusChanges = CorsairPluginUnsubscribeFromDeviceStatusChanges;
	instance->CorsairPluginGetDeviceView = CorsairPluginGetDeviceView;
	instance->CorsairPluginFreeDeviceInfo = CorsairPluginFreeDeviceInfo;
	instance->CorsairPluginFreeDeviceView = CorsairPluginFreeDeviceView;
	instance->CorsairSubscribeForEvents = CorsairSubscribeForEvents;
	instance->CorsairUnsubscribeFromEvents = CorsairUnsubscribeFromEvents;
	instance->CorsairConfigureKeyEvent = CorsairConfigureKeyEvent;
	instance->CorsairSetMode = CorsairSetMode;
	return instance;
}
}