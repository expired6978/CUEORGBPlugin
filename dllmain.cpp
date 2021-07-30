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

#ifdef _DEBUG
using CorsairGetInstance = CorsairGetInstance_v67;
#else
using CorsairGetInstance = CorsairGetInstance_v66;
#endif

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

bool CorsairGetPluginPropertyInfo(CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyDataType& dataType, std::int32_t& flags)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairGetPluginPropertyInfo: Index: %d, PropertyId: %d", index, propertyId);

	switch (propertyId)
	{
	case CorsairDevicePropertyId::CPDPI_PropertiesList:
	{
		dataType = CorsairPropertyDataType::CPDT_IntegerArray;
		flags = CorsairPropertyFlags::CPF_CanRead | CorsairPropertyFlags::CPF_Indexed;
		return true;
	}
	break;
	default:
		break;
	}

	return false;
}

bool CorsairGetDevicePropertyInfo(const char* deviceId, CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyDataType& dataType, std::int32_t& flags)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairGetDevicePropertyInfo: Device: %s, Index: %d, PropertyId: %d", deviceId, index, propertyId);
	switch (propertyId)
	{
	case CorsairDevicePropertyId::CPDPI_PropertiesList:
	{
		dataType = CorsairPropertyDataType::CPDT_IntegerArray;
		flags = CorsairPropertyFlags::CPF_CanRead;
		return true;
	}
	break;
	case CPDPI_SensorsCount:
	{
		dataType = CorsairPropertyDataType::CPDT_Integer;
		flags = CorsairPropertyFlags::CPF_CanRead;
		return true;
	}
	break;
	case CPDPI_SensorType:
	case CPDPI_SensorValue:
	{
		dataType = CorsairPropertyDataType::CPDT_Integer;
		flags = CorsairPropertyFlags::CPF_CanRead | CorsairPropertyFlags::CPF_Indexed;
		return true;
	}
	break;
	case CPDPI_SensorName:
	{
		dataType = CorsairPropertyDataType::CPDT_String;
		flags = CorsairPropertyFlags::CPF_CanRead | CorsairPropertyFlags::CPF_Indexed;
		return true;
	}
	break;
	default:
		break;
	}
	return false;
}

CorsairPropertyData* CorsairReadDevicePropertyData(const char* deviceId, CorsairDevicePropertyId propertyId, std::int32_t index)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairReadDevicePropertyData: Device: %s, Index: %d, PropertyId: %d", deviceId, index, propertyId);

	switch (propertyId)
	{
	case CorsairDevicePropertyId::CPDPI_PropertiesList:
	{
		CorsairPropertyData* propertyData = static_cast<CorsairPropertyData*>(malloc(sizeof(CorsairPropertyData)));
		uint32_t* props = new uint32_t[4];
		props[0] = CPDPI_SensorsCount;
		props[1] = CPDPI_SensorType;
		props[2] = CPDPI_SensorValue;
		props[3] = CPDPI_SensorName;
		propertyData->data = props;
		propertyData->count = 4;
		return propertyData;
	}
	break;
	case CPDPI_SensorsCount:
	{
		CorsairPropertyData* propertyData = static_cast<CorsairPropertyData*>(malloc(sizeof(CorsairPropertyData)));
		propertyData->data = (void*)1;
		propertyData->count = 0;
		return propertyData;
	}
	break;
	case CPDPI_SensorType:
	{
		CorsairPropertyData* propertyData = static_cast<CorsairPropertyData*>(malloc(sizeof(CorsairPropertyData)));
		propertyData->data = (void*)CST_Temperature;
		propertyData->count = 0;
		return propertyData;
	}
	break;
	case CPDPI_SensorValue:
	{
		CorsairPropertyData* propertyData = static_cast<CorsairPropertyData*>(malloc(sizeof(CorsairPropertyData)));
		propertyData->data = (void*)6000;
		propertyData->count = 0;
		return propertyData;
	}
	break;
	case CPDPI_SensorName:
	{
		CorsairPropertyData* propertyData = static_cast<CorsairPropertyData*>(malloc(sizeof(CorsairPropertyData)));
		propertyData->data = _strdup("TEST");
		propertyData->count = 0;
		return propertyData;
	}
	break;
	default:
		break;
	}

	return nullptr;
}

CorsairPropertyData* CorsairReadPluginPropertyData(CorsairDevicePropertyId propertyId, std::int32_t index)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairReadPluginPropertyData: Index: %d, PropertyId: %d", index, propertyId);
	switch (propertyId)
	{
	case CorsairDevicePropertyId::CPDPI_PropertiesList:
	{
		CorsairPropertyData* propertyData = static_cast<CorsairPropertyData*>(malloc(sizeof(CorsairGetInstance)));
		uint32_t* props = new uint32_t[4];
		props[0] = CPDPI_SensorsCount;
		props[1] = CPDPI_SensorType;
		props[2] = CPDPI_SensorName;
		props[3] = CPDPI_SensorValue;
		propertyData->data = props;
		propertyData->count = 4;
		return propertyData;
	}
	break;
	default:
		break;
	}

	return nullptr;
}

bool CorsairWriteDevicePropertyData(const char* deviceId, CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyData& data)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairWriteDevicePropertyData: Device: %s, Index: %d, PropertyId: %d", deviceId, index, propertyId);
	return false;
}

bool CorsairWritePluginPropertyData(CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyData& data)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairWritePluginPropertyData: Index: %d, PropertyId: %d", index, propertyId);
	return false;
}

void CorsairFreePropertyData(CorsairPropertyDataType propertyId, CorsairPropertyData* data)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	switch (propertyId)
	{
	case CorsairPropertyDataType::CPDT_BoolArray:
		delete[] static_cast<bool*>(data->data);
		break;
	case CorsairPropertyDataType::CPDT_IntegerArray:
		delete[] static_cast<uint32_t*>(data->data);
		break;
	case CorsairPropertyDataType::CPDT_DoubleArray:
		delete[] static_cast<double*>(data->data);
		break;
	case CorsairPropertyDataType::CPDT_StringArray:
		for (int32_t i = 0; i < data->count; ++i)
		{
			free(reinterpret_cast<char**>(data->data)[i]);
		}
		delete[] static_cast<char**>(data->data);
		break;
	case CorsairPropertyDataType::CPDT_String:
		free(data->data);
		break;
	default:
		break;
	}

	free(data);

	OutputDebugMessage("CorsairFreePropertyData: PropertyId: %d", propertyId);
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
#ifdef _DEBUG
	return 0x67;
#else
	return 0x66;
#endif
}

std::int32_t CorsairPluginGetFeatures()
{
	OutputDebugMessage("CorsairPluginGetFeatures");
	std::int32_t features = CorsairSupportedFeatures::CPSF_DetachedMode;
#ifdef _DEBUG
	features |= CorsairSupportedFeatures::CPSF_DeviceProperties;
#endif
	return features;
}

struct QString
{
	uintptr_t pad[5];
};

struct QMetaEnum
{
	uintptr_t pad[10];
};

struct QMessageLogger
{
	uintptr_t pad[50];
};

struct QDebug
{
	uintptr_t pad[50];
};
struct QMetaObject
{
	uintptr_t pad[50];
};
struct QLoggingCategory
{
	uintptr_t pad[50];
};

CorsairGetInstance* CorsairPluginGetInstance()
{
#ifdef SUSPEND_API_INSTANCE
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif

#ifdef _DUMP_ENUM
	HMODULE qtCoreModule = GetModuleHandle(_T("Qt5Core.dll"));
	HMODULE baseAddress = GetModuleHandle(nullptr);
	QMetaObject* metaObject = reinterpret_cast<QMetaObject*>(uintptr_t(baseAddress) + 0x0088420);


	using _QMetaObject_indexOfEnumerator = int (*)(QMetaObject*, const char* name);
	using _QMetaObject_enumerator = void (*)(QMetaObject*, QMetaEnum&, int);
	using _QMetaObject_enumeratorCount = int (*)(QMetaObject*);
	using _QMetaObject_className = const char* (*)(QMetaObject*);

	using _QMetaType_type = int (*)(const char*);
	using _QMetaType_metaObjectForType = QMetaObject* (*)(int);

	using _QMetaEnum_valueToKey = const char* (*)(QMetaEnum*, int);
	using _QMetaEnum_keyCount = int (*)(QMetaEnum*);
	using _QMetaEnum_value = int (*)(QMetaEnum*, int);
	using _QMetaEnum_enumName = const char* (*)(QMetaEnum*);


	_QMetaObject_indexOfEnumerator QMetaObject_indexOfEnumerator = (_QMetaObject_indexOfEnumerator)GetProcAddress(qtCoreModule, "?indexOfEnumerator@QMetaObject@@QEBAHPEBD@Z");
	_QMetaObject_enumerator QMetaObject_enumerator = (_QMetaObject_enumerator)GetProcAddress(qtCoreModule, "?enumerator@QMetaObject@@QEBA?AVQMetaEnum@@H@Z");
	_QMetaEnum_valueToKey QMetaEnum_valueToKey = (_QMetaEnum_valueToKey)GetProcAddress(qtCoreModule, "?valueToKey@QMetaEnum@@QEBAPEBDH@Z");
	_QMetaEnum_keyCount QMetaEnum_keyCount = (_QMetaEnum_keyCount)GetProcAddress(qtCoreModule, "?keyCount@QMetaEnum@@QEBAHXZ");
	_QMetaEnum_value QMetaEnum_value = (_QMetaEnum_value)GetProcAddress(qtCoreModule, "?value@QMetaEnum@@QEBAHH@Z");
	_QMetaEnum_enumName QMetaEnum_enumName = (_QMetaEnum_enumName)GetProcAddress(qtCoreModule, "?enumName@QMetaEnum@@QEBAPEBDXZ");
	_QMetaObject_enumeratorCount QMetaObject_enumeratorCount = (_QMetaObject_enumeratorCount)GetProcAddress(qtCoreModule, "?enumeratorCount@QMetaObject@@QEBAHXZ");
	_QMetaObject_className QMetaObject_className = (_QMetaObject_className)GetProcAddress(qtCoreModule, "?className@QMetaObject@@QEBAPEBDXZ");

	_QMetaType_type QMetaType_type = (_QMetaType_type)GetProcAddress(qtCoreModule, "?type@QMetaType@@SAHPEBD@Z");
	_QMetaType_metaObjectForType QMetaType_metaObjectForType = (_QMetaType_metaObjectForType)GetProcAddress(qtCoreModule, "?metaObjectForType@QMetaType@@SAPEBUQMetaObject@@H@Z");

	//int idx = QMetaObject_indexOfEnumerator(metaObject, "DevicePropertyId");
	//int typeId = QMetaType_type("cue::dev::plugin");

	auto pluginMetaObject = metaObject;
	if (pluginMetaObject)
	{
		int enumCount = QMetaObject_enumeratorCount(pluginMetaObject);
		if (enumCount)
		{
			OutputDebugMessage("struct %s\n{\n", QMetaObject_className(pluginMetaObject));

			for (int idx = 0; idx < enumCount; ++idx)
			{
				QMetaEnum enumerator;
				QMetaObject_enumerator(pluginMetaObject, enumerator, idx);
				OutputDebugMessage("\tenum %s\n\t{\n", QMetaEnum_enumName(&enumerator));
				int keys = QMetaEnum_keyCount(&enumerator);
				for (int i = 0; i < keys; ++i)
				{
					int value = QMetaEnum_value(&enumerator, i);
					const char* key = QMetaEnum_valueToKey(&enumerator, value);
					OutputDebugMessage("\t\t%s = %d,\n", key, value);
				}
				OutputDebugMessage("\t};\n");
			}
			OutputDebugMessage("};\n");
		}
	}
#endif

	CorsairGetInstance* instance = static_cast<CorsairGetInstance*>(malloc(sizeof(CorsairGetInstance)));
	memset(instance, 0, sizeof(CorsairGetInstance));
	instance->getDeviceInfo = CorsairPluginGetDeviceInfo;
	instance->setLedsColors = CorsairSetLedsColors;
	instance->subscribeForDeviceConnectionStatusChanges = CorsairSubscribeForDeviceConnectionStatusChanges;
	instance->unsubscribeFromDeviceConnectionStatusChanges = CorsairPluginUnsubscribeFromDeviceStatusChanges;
	instance->getDeviceView = CorsairPluginGetDeviceView;
	instance->freeDeviceInfo = CorsairPluginFreeDeviceInfo;
	instance->freeDeviceView = CorsairPluginFreeDeviceView;
	instance->subscribeForEvents = CorsairSubscribeForEvents;
	instance->unsubscribeFromEvents = CorsairUnsubscribeFromEvents;
	instance->configureKeyEvent = CorsairConfigureKeyEvent;
	instance->setMode = CorsairSetMode;
#ifdef _DEBUG
	instance->getPropertyInfo = CorsairGetPluginPropertyInfo;
	instance->readPropertyData = CorsairReadPluginPropertyData;
	instance->writePropertyData = CorsairWritePluginPropertyData;
	instance->freePropertyData = CorsairFreePropertyData;
	instance->getDevicePropertyInfo = CorsairGetDevicePropertyInfo;
	instance->readDevicePropertyData = CorsairReadDevicePropertyData;
	instance->writeDevicePropertyData = CorsairWriteDevicePropertyData;
#endif
	return instance;
}
}