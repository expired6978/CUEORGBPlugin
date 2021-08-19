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

//#define SUSPEND_API
//#define C_PROPERTIES
//#define C_CHANNELS

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

#ifdef C_PROPERTIES
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

std::string GetDeviceHash(const std::string& deviceId)
{
	return picosha2::hash256_hex_string(deviceId);
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

cue::dev::plugin::DeviceInfo* CorsairPluginGetDeviceInfo(const char* deviceId)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairPluginGetDeviceInfo: %s", deviceId);
	return g_deviceManager ? g_deviceManager->GetDeviceInfo(deviceId) : nullptr;
}

bool CorsairSetLedsColors(const char* deviceId, std::int32_t size, cue::dev::plugin::LedColor* ledsColors)
{
	return g_deviceManager ? g_deviceManager->SetColor(deviceId, size, ledsColors) : false;
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

cue::dev::plugin::DeviceView* CorsairPluginGetDeviceView(const char* deviceId, std::int32_t viewId)
{
	OutputDebugMessage("CorsairPluginGetDeviceView: %s - %d", deviceId, viewId);
	return g_deviceManager ? g_deviceManager->GetDeviceView(deviceId, viewId) : nullptr;
}

void CorsairPluginFreeDeviceInfo(cue::dev::plugin::DeviceInfo* deviceInfo)
{
	OutputDebugMessage("CorsairPluginFreeDeviceInfo: %08X", deviceInfo);
	CorsairPluginDevice::DestroyDeviceInfo(deviceInfo);
}

void CorsairPluginFreeDeviceView(cue::dev::plugin::DeviceView* deviceView)
{
	OutputDebugMessage("CorsairPluginFreeDeviceView: %08X", deviceView);
	CorsairPluginDevice::DestroyDeviceView(deviceView);
}

void CorsairSubscribeForDeviceConnectionStatusChanges(void* context, _DeviceConnectionStatusChangeCallback deviceStatusCallback)
{
	OutputDebugMessage("CorsairSubscribeForDeviceConnectionStatusChanges: %08X - %08X", context, deviceStatusCallback);
	g_deviceManager = std::make_unique<CorsairPluginDeviceManager>(context, deviceStatusCallback, 
		GetImageHash,
		GetDeviceHash,
		GetLocalFile
	);
	g_deviceManager->Start();
}

void CorsairPluginUnsubscribeFromDeviceStatusChanges()
{
	OutputDebugMessage("CorsairPluginUnsubscribeFromDeviceStatusChanges");
	g_deviceManager.reset();
}

bool CorsairGetPluginPropertyInfo(cue::dev::plugin::PluginPropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyDataType& dataType, cue::dev::plugin::PropertyFlags& flags)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairGetPluginPropertyInfo: Index: %d, PropertyId: %d", index, propertyId);

	switch (propertyId)
	{
	case cue::dev::plugin::PluginPropertyId::PropertiesList:
	{
		dataType = cue::dev::plugin::PropertyDataType::IntegerArray;
		flags = cue::dev::plugin::PropertyFlags::CanRead | cue::dev::plugin::PropertyFlags::Indexed;
		return true;
	}
	break;
	default:
		break;
	}

	return false;
}

bool CorsairGetDevicePropertyInfo(const char* deviceId, cue::dev::plugin::DevicePropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyDataType& dataType, cue::dev::plugin::PropertyFlags& flags)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairGetDevicePropertyInfo: Device: %s, Index: %d, PropertyId: %d", deviceId, index, propertyId);
	switch (propertyId)
	{
	case cue::dev::plugin::DevicePropertyId::PropertiesList:
	{
		dataType = cue::dev::plugin::PropertyDataType::IntegerArray;
		flags = cue::dev::plugin::PropertyFlags::CanRead;
		return true;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::SensorsCount:
	{
		dataType = cue::dev::plugin::PropertyDataType::Integer;
		flags = cue::dev::plugin::PropertyFlags::CanRead;
		return true;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::SensorType:
	case cue::dev::plugin::DevicePropertyId::SensorValue:
	{
		dataType = cue::dev::plugin::PropertyDataType::Integer;
		flags = cue::dev::plugin::PropertyFlags::CanRead | cue::dev::plugin::PropertyFlags::Indexed;
		return true;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::SensorName:
	{
		dataType = cue::dev::plugin::PropertyDataType::String;
		flags = cue::dev::plugin::PropertyFlags::CanRead | cue::dev::plugin::PropertyFlags::Indexed;
		return true;
	}
	break;

#ifdef C_CHANNELS
	case cue::dev::plugin::DevicePropertyId::ChannelsCount:
	{
		dataType = cue::dev::plugin::PropertyDataType::Integer;
		flags = cue::dev::plugin::PropertyFlags::CanRead;
		return true;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::ChannelName:
	{
		dataType = cue::dev::plugin::PropertyDataType::String;
		flags = cue::dev::plugin::PropertyFlags::CanRead | cue::dev::plugin::PropertyFlags::Indexed;
		return true;
	}
	break;

#endif

	default:
		break;
	}
	return false;
}

cue::dev::plugin::PropertyData* CorsairReadDevicePropertyData(const char* deviceId, cue::dev::plugin::DevicePropertyId propertyId, std::int32_t index)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairReadDevicePropertyData: Device: %s, Index: %d, PropertyId: %d", deviceId, index, propertyId);
	switch (propertyId)
	{
	case cue::dev::plugin::DevicePropertyId::PropertiesList:
	{
#ifdef C_CHANNELS
		static int PROPERTIES = 6;
#else
		static int PROPERTIES = 4;
#endif
		auto propertyData = new cue::dev::plugin::PropertyData;
		propertyData->data.devicePropertyArray = new cue::dev::plugin::DevicePropertyId[PROPERTIES] {
			cue::dev::plugin::DevicePropertyId::SensorsCount,
			cue::dev::plugin::DevicePropertyId::SensorType,
			cue::dev::plugin::DevicePropertyId::SensorValue,
			cue::dev::plugin::DevicePropertyId::SensorName,
#ifdef C_CHANNELS
			cue::dev::plugin::DevicePropertyId::ChannelsCount,
			cue::dev::plugin::DevicePropertyId::ChannelName
#endif
		};
		propertyData->count = PROPERTIES;
		return propertyData;
	}
	break;

#ifdef C_CHANNELS
	case cue::dev::plugin::DevicePropertyId::ChannelsCount:
	{
		auto propertyData = new cue::dev::plugin::PropertyData;
		propertyData->data.s64 = 2;
		propertyData->count = 0;
		return propertyData;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::ChannelName:
	{
		auto propertyData = new cue::dev::plugin::PropertyData;
		if (index == 0)
		{
			propertyData->data.str = _strdup("Cool Channel 1");
		}
		else if (index == 1)
		{
			propertyData->data.str = _strdup("Cool Channel 2");
		}
		propertyData->count = 0;
		return propertyData;
	}
	break;
#endif

	case cue::dev::plugin::DevicePropertyId::SensorsCount:
	{
		auto propertyData = new cue::dev::plugin::PropertyData;
		propertyData->data.s64 = 2;
		propertyData->count = 0;
		return propertyData;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::SensorType:
	{
		auto propertyData = new cue::dev::plugin::PropertyData;
		if (index == 0)
		{
			propertyData->data.s64 = cue::dev::plugin::SensorType::Temperature;
		}
		else
		{
			propertyData->data.s64 = cue::dev::plugin::SensorType::Power;
		}
		
		propertyData->count = 0;
		return propertyData;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::SensorValue:
	{
		auto propertyData = new cue::dev::plugin::PropertyData;
		if (index == 0)
		{
			propertyData->data.s64 = 6000;
		}
		else
		{
			propertyData->data.s64 = 50;
		}
		propertyData->count = 0;
		return propertyData;
	}
	break;
	case cue::dev::plugin::DevicePropertyId::SensorName:
	{
		auto propertyData = new cue::dev::plugin::PropertyData;
		if (index == 0)
		{
			propertyData->data.str = _strdup("Temperature");
		}
		else
		{
			propertyData->data.str = _strdup("Power");
		}
		propertyData->count = 0;
		return propertyData;
	}
	break;
	default:
		break;
	}

	return nullptr;
}

cue::dev::plugin::PropertyData* CorsairReadPluginPropertyData(cue::dev::plugin::PluginPropertyId propertyId, std::int32_t index)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairReadPluginPropertyData: Index: %d, PropertyId: %d", index, propertyId);
	return nullptr;
}

bool CorsairWriteDevicePropertyData(const char* deviceId, cue::dev::plugin::DevicePropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyData& data)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairWriteDevicePropertyData: Device: %s, Index: %d, PropertyId: %d", deviceId, index, propertyId);
	return false;
}

bool CorsairWritePluginPropertyData(cue::dev::plugin::PluginPropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyData& data)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairWritePluginPropertyData: Index: %d, PropertyId: %d", index, propertyId);
	return false;
}

void CorsairFreePropertyData(cue::dev::plugin::PropertyDataType propertyId, cue::dev::plugin::PropertyData* data)
{
#ifdef SUSPEND_API
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif
	OutputDebugMessage("CorsairFreePropertyData: PropertyId: %d", propertyId);
	switch (propertyId)
	{
	case cue::dev::plugin::PropertyDataType::BoolArray:
		delete[] data->data.boolArray;
		break;
	case cue::dev::plugin::PropertyDataType::IntegerArray:
		delete[] data->data.intArray;
		break;
	case cue::dev::plugin::PropertyDataType::DoubleArray:
		delete[] data->data.doubleArray;
		break;
	case cue::dev::plugin::PropertyDataType::StringArray:
		for (int32_t i = 0; i < data->count; ++i)
		{
			free(data->data.strArray[i]);
		}
		delete[] data->data.strArray;
		break;
	case cue::dev::plugin::PropertyDataType::String:
		free(data->data.str);
		break;
	default:
		break;
	}

	delete data;
}

void CorsairSetLedColorAtChannel(const char* deviceId, std::int32_t unk1, std::int32_t size, LedColorChannel& channel)
{
#if 0
	while (!::IsDebuggerPresent())
		::Sleep(100);
#endif

	OutputDebugMessage("CorsairSetLedColorAtChannel: deviceId: %s", deviceId);
}

extern "C"
{
	void CorsairPluginFreeInstance(CorsairGetInstance* instance)
	{
		OutputDebugMessage("CorsairPluginFreeInstance: %08X", instance);
		delete instance;
	}

	std::int32_t CorsairPluginGetAPIVersion()
	{
		OutputDebugMessage("CorsairPluginGetAPIVersion");
#ifdef C_PROPERTIES
		return 0x67;
#else
		return 0x66;
#endif
	}

	cue::dev::plugin::SupportedFeatures CorsairPluginGetFeatures()
	{
		OutputDebugMessage("CorsairPluginGetFeatures");
		cue::dev::plugin::SupportedFeatures features = cue::dev::plugin::SupportedFeatures::DetachedMode;
#ifdef C_PROPERTIES
		features |= cue::dev::plugin::SupportedFeatures::DeviceProperties;
#endif
#ifdef C_CHANNELS
		features |= cue::dev::plugin::SupportedFeatures::DeviceChannels;
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
		using _QMetaType_metaObjectForType = QMetaObject * (*)(int);

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

		CorsairGetInstance* instance = new CorsairGetInstance;
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
#ifdef C_PROPERTIES
		instance->getPropertyInfo = CorsairGetPluginPropertyInfo;
		instance->readPropertyData = CorsairReadPluginPropertyData;
		instance->writePropertyData = CorsairWritePluginPropertyData;
		instance->freePropertyData = CorsairFreePropertyData;
		instance->getDevicePropertyInfo = CorsairGetDevicePropertyInfo;
		instance->readDevicePropertyData = CorsairReadDevicePropertyData;
		instance->writeDevicePropertyData = CorsairWriteDevicePropertyData;
#endif
#ifdef C_CHANNELS
		instance->setLedsColorsAtChannel = CorsairSetLedColorAtChannel;
#endif
		return instance;
	}
}