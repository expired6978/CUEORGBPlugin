#include "CorsairPluginDeviceManager.h"
#include "NetworkClient.h"

#include <fstream>

void ClientChanged(void* arg)
{
	CorsairPluginDeviceManager* pluginManager = static_cast<CorsairPluginDeviceManager*>(arg);
	pluginManager->DisconnectDevices();
	pluginManager->ConnectDevices();
}

CorsairPluginDeviceManager::CorsairPluginDeviceManager(void* pluginContext, _DeviceConnectionStatusChangeCallback callback, 
	std::function<std::string(const std::string&)> imageHasher,
	std::function<std::wstring(const std::wstring&)> localPath)
	: mPluginContext(pluginContext)
	, mDeviceCallback(callback)
	, mNetworkClient(new NetworkClient(mControllerList))
	, mImageHasher(imageHasher)
	, mLocalFile(localPath)
{
	mNetworkClient->RegisterClientInfoChangeCallback(ClientChanged, this);
}

CorsairPluginDeviceManager::~CorsairPluginDeviceManager()
{
	if (mNetworkClient)
	{
		mNetworkClient->StopClient();
		delete mNetworkClient;
	}
}

void CorsairPluginDeviceManager::Start()
{
	if (ReadJson())
	{
		if (mSettings.contains("OpenRGB"))
		{
			auto openRGB = mSettings["OpenRGB"];
			if (openRGB.contains("Host"))
			{
				std::string host = openRGB["Host"];
				mNetworkClient->SetIP(host.c_str());
			}
			if (openRGB.contains("Port"))
			{
				mNetworkClient->SetPort(openRGB["Port"]);
			}
			if (openRGB.contains("Client"))
			{
				std::string client = openRGB["Client"];
				mNetworkClient->SetName(client.c_str());
			}
		}
		
		if (mNetworkClient)
		{
			mNetworkClient->StartClient();
		}
	}
}

void CorsairPluginDeviceManager::Stop()
{
	if (mNetworkClient)
	{
		mNetworkClient->StopClient();
	}
}

bool CorsairPluginDeviceManager::SetColor(const char* deviceId, std::int32_t size, CorsairLedColor* ledsColors)
{
	std::lock_guard<std::mutex> networkLock(mNetworkClient->ControllerListMutex);
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	auto it = mDeviceMap.find(deviceId);
	if (it != mDeviceMap.end())
	{
		auto controller = it->second->GetController();
		for (std::int32_t i = 0; i < size; ++i)
		{
			auto& ledColor = ledsColors[i];
			auto ledIt = it->second->GetInfo().ledMapping.find(ledsColors[i].ledId);
			if (ledIt != it->second->GetInfo().ledMapping.end())
			{
				std::uint32_t zoneId = ledIt->second.first;
				std::uint32_t zoneIndex = ledIt->second.second;
				controller->SetLED(controller->zones.at(zoneId).start_idx + zoneIndex, ToRGBColor(ledColor.r, ledColor.g, ledColor.b));
			}
		}
		
		controller->UpdateLEDs();
		return true;
	}

	return false;
}

CorsairPluginDeviceInfo* CorsairPluginDeviceManager::GetDeviceInfo(const char* deviceId)
{
	std::lock_guard<std::mutex> networkLock(mNetworkClient->ControllerListMutex);
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	auto it = mDeviceMap.find(deviceId);
	if (it != mDeviceMap.end())
	{
		return it->second->GetDeviceInfo();
	}

	return nullptr;
}

CorsairPluginDeviceView* CorsairPluginDeviceManager::GetDeviceView(const char* deviceId, std::int32_t index)
{
	std::lock_guard<std::mutex> networkLock(mNetworkClient->ControllerListMutex);
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	auto it = mDeviceMap.find(deviceId);
	if (it != mDeviceMap.end())
	{
		return it->second->GetDeviceView(index);
	}

	return nullptr;
}

bool CorsairPluginDeviceManager::ReadJson()
{
	std::ifstream devices(mLocalFile(L"devices.json").c_str());
	nlohmann::json jd;
	try
	{
		devices >> jd;
	}
	catch (...)
	{
		devices.close();
		return false;
	}

	devices.close();

	std::ifstream settings(mLocalFile(L"settings.json").c_str());
	nlohmann::json js;
	try
	{
		settings >> js;
	}
	catch (...)
	{
		settings.close();
		return false;
	}
	settings.close();

	mSettings = js;
	mDevices = jd;
	return true;
}

void CorsairPluginDeviceManager::ConnectDevices()
{
	//std::lock_guard<std::mutex> networkLock(mNetworkClient->ControllerListMutex);
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	for (auto controller : mControllerList)
	{
		std::unique_ptr<CorsairPluginDevice> device = std::make_unique<CorsairPluginDevice>(controller);
		device->SetImageHasher(mImageHasher);
		device->ReadFromJson(mSettings, mDevices);
		mDeviceCallback(mPluginContext, device->GetInfo().deviceId.c_str(), 1);
		mDeviceMap.emplace(device->GetInfo().deviceId, std::move(device));
	}

}

void CorsairPluginDeviceManager::DisconnectDevices()
{
	//std::lock_guard<std::mutex> networkLock(mNetworkClient->ControllerListMutex);
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	for (auto& device : mDeviceMap)
	{
		mDeviceCallback(mPluginContext, device.first.c_str(), 0);
	}
	mDeviceMap.clear();
}
