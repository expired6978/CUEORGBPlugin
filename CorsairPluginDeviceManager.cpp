#include "CorsairPluginDeviceManager.h"
#include "NetworkClient.h"

#include <future>
#include <fstream>
#include <unordered_set>

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
	, mNetworkClient(std::make_unique<NetworkClient>(mControllerList))
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
	}
	if (mDeviceUpdateRequest.valid())
	{
		mDeviceUpdateRequest.wait();
	}
}

void CorsairPluginDeviceManager::Start()
{
	if (ReadJson())
	{
		if (mSettings.contains("OpenRGB"))
		{
			const auto& openRGB = mSettings["OpenRGB"];
			if (openRGB.contains("Host"))
			{
				mNetworkClient->SetIP(openRGB["Host"].get<std::string>().c_str());
			}
			if (openRGB.contains("Port"))
			{
				mNetworkClient->SetPort(openRGB["Port"]);
			}
			if (openRGB.contains("Client"))
			{
				mNetworkClient->SetName(openRGB["Client"].get<std::string>().c_str());
			}
		}

		mNetworkClient->StartClient();
	}
}

void CorsairPluginDeviceManager::Stop()
{
	mNetworkClient->StopClient();
}

bool CorsairPluginDeviceManager::SetColor(const char* deviceId, std::int32_t size, cue::dev::plugin::LedColor* ledsColors)
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
			auto ledIt = it->second->GetInfo().ledMapping.find(ledColor.ledId);
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

cue::dev::plugin::DeviceInfo* CorsairPluginDeviceManager::GetDeviceInfo(const char* deviceId)
{
	std::lock_guard<std::mutex> networkLock(mNetworkClient->ControllerListMutex);
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	auto it = mDeviceMap.find(deviceId);
	if (it != mDeviceMap.end())
	{
		return it->second->CreateDeviceInfo();
	}

	return nullptr;
}

cue::dev::plugin::DeviceView* CorsairPluginDeviceManager::GetDeviceView(const char* deviceId, std::int32_t index)
{
	std::lock_guard<std::mutex> networkLock(mNetworkClient->ControllerListMutex);
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	auto it = mDeviceMap.find(deviceId);
	if (it != mDeviceMap.end())
	{
		return it->second->CreateDeviceView(index);
	}

	return nullptr;
}

void CorsairPluginDeviceManager::UpdateDevices(std::unordered_set<std::string> deviceSet, bool notifyHost)
{
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);
	for (auto& deviceId : deviceSet)
	{
		auto it = mDeviceMap.find(deviceId);
		if (it != mDeviceMap.end())
		{
			it->second->GetController()->SetCustomMode(); // Calls SendRequest_ControllerData and waits for response
			if (it->second->ReadFromJson(mSettings, mDevices, true) && notifyHost)
			{
				mDeviceCallback(mPluginContext, deviceId.c_str(), 1);
			}
		}
	}
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
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	std::unordered_set<std::string> deviceUpdate;
	for (auto controller : mControllerList)
	{
		std::unique_ptr<CorsairPluginDevice> device = std::make_unique<CorsairPluginDevice>(controller);
		device->SetImageHasher(mImageHasher);
		if (device->ReadFromJson(mSettings, mDevices))
		{
			// Device needs a resize, send the resize packet and re-request the controller data
			// We will delay sending the notification to the host until the new data has come back
			if (device->GetResizeMap().size())
			{
				for (auto& zone : device->GetResizeMap())
				{
					controller->ResizeZone(zone.first, zone.second);
					deviceUpdate.emplace(device->GetInfo().deviceId);
				}
			}
			else
			{
				mDeviceCallback(mPluginContext, device->GetInfo().deviceId.c_str(), 1);
			}

			mDeviceMap.emplace(device->GetInfo().deviceId, std::move(device));
		}
	}

	// If there's a device that's the wrong size, send a device update and wait for the response
	// We need this in another thread because we call ConnectDevices from within the Changed devices callback, which has a lock on ControllerListMutex
	if (deviceUpdate.size())
	{
		if (mDeviceUpdateRequest.valid())
		{
			mDeviceUpdateRequest.wait();
		}

		mDeviceUpdateRequest = std::async(std::launch::async, &CorsairPluginDeviceManager::UpdateDevices, this, deviceUpdate, true);
	}
}

void CorsairPluginDeviceManager::DisconnectDevices()
{
	std::lock_guard<std::mutex> deviceLock(mDeviceLock);

	for (auto& device : mDeviceMap)
	{
		mDeviceCallback(mPluginContext, device.first.c_str(), 0);
	}
	mDeviceMap.clear();
}
