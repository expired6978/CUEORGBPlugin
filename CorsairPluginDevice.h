#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include <tchar.h>

#include "json/json.hpp"

class RGBController;

namespace cue
{
namespace dev
{
namespace plugin
{
struct DeviceInfo;
struct DeviceView;
}
}
}


struct LEDData
{
	double x;
	double y;
};

struct ZoneData
{
	std::map<std::uint32_t, LEDData> ledData;
};

struct DeviceInfo
{
	std::string deviceName;
	std::string deviceId;
	std::string thumbnail;
	std::string promo;
	std::unordered_map<std::uint32_t, std::pair<std::uint32_t, std::uint32_t>> ledMapping;
	std::map<std::uint32_t, ZoneData> zones;
};

struct DeviceView
{
	std::string image;
	std::string mask;
	std::map<std::int32_t, std::string> drawPath;
};
typedef std::map<std::int32_t, DeviceView> DeviceViews;

class CorsairPluginDevice
{
public:
	CorsairPluginDevice(RGBController* controller) : mController(controller) { }

	cue::dev::plugin::DeviceInfo* CreateDeviceInfo();
	cue::dev::plugin::DeviceView* CreateDeviceView(std::int32_t index);
	static void DestroyDeviceInfo(cue::dev::plugin::DeviceInfo* deviceInfo);
	static void DestroyDeviceView(cue::dev::plugin::DeviceView* deviceInfo);

	const DeviceInfo& GetInfo() const { return mDeviceInfo; }
	const DeviceViews& GetViews() const { return mDeviceViews; }
	RGBController* GetController() const { return mController; }

	bool ReadFromJson(const nlohmann::json& settings, const nlohmann::json& devices, bool clear = false);
	void SetImageHasher(std::function<std::string(const std::string&)> functor) { mImageHasher = functor; };

	typedef std::unordered_map<std::uint32_t, std::uint32_t> ResizeMap;
	ResizeMap& GetResizeMap() { return mZoneResize; }

protected:

	void GetDeviceInfoFromJson(const nlohmann::json& settings, const nlohmann::json& devices);
	bool GetDeviceViewFromJson(const nlohmann::json& settings, const nlohmann::json& devices);

	bool ReadZonesFromJson(const nlohmann::json& zone);
	void ReadViewFromJson(const nlohmann::json& view, DeviceView& deviceView);

private:
	ResizeMap mZoneResize;
	RGBController* mController;
	std::function<std::string(const std::string&)> mImageHasher;
	DeviceInfo mDeviceInfo;
	DeviceViews mDeviceViews;
};