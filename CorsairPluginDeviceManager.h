#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <mutex>
#include <memory>

#include "CUESDKDevice.h"
#include "CorsairPluginDevice.h"
#include "json/json.hpp"

class NetworkClient;
class RGBController;

class CorsairPluginDeviceManager
{
public:
	CorsairPluginDeviceManager(void* pluginContext, _DeviceConnectionStatusChangeCallback callback, 
		std::function<std::string(const std::string&)> imageHasher,
		std::function<std::wstring(const std::wstring&)> localPath);
	virtual ~CorsairPluginDeviceManager();

	virtual void Start();
	virtual void Stop();
	virtual bool ReadJson();
	virtual void ConnectDevices();
	virtual void DisconnectDevices();

	// CUE Calls
	virtual bool SetColor(const char* deviceId, std::int32_t size, CorsairLedColor* ledsColors);
	virtual CorsairPluginDeviceInfo* GetDeviceInfo(const char* deviceId);
	virtual CorsairPluginDeviceView* GetDeviceView(const char* deviceId, std::int32_t index);

protected:
	void* mPluginContext;
	_DeviceConnectionStatusChangeCallback mDeviceCallback;
	nlohmann::json mSettings;
	nlohmann::json mDevices;

private:
	std::mutex mDeviceLock;
	std::unordered_map<std::string, std::unique_ptr<CorsairPluginDevice>> mDeviceMap;
	NetworkClient* mNetworkClient;
	std::vector<RGBController*> mControllerList;

	std::function<std::string(const std::string&)> mImageHasher;
	std::function<std::wstring(const std::wstring&)> mLocalFile;
};