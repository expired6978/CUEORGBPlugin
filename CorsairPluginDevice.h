#pragma once

#include <stdint.h>
#include <map>
#include <string>
#include <tchar.h>

#include "json/json.hpp"

class RGBController;

struct CorsairPluginDeviceInfo;
struct CorsairPluginDeviceView;

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

	const DeviceInfo& GetInfo() const { return mDeviceInfo; }
	const DeviceViews& GetViews() const { return mDeviceViews; }
	RGBController* GetController() const { return mController; }

	bool ReadFromJson(const nlohmann::json& settings, const nlohmann::json& devices);

	CorsairPluginDeviceInfo* GetDeviceInfo();
	CorsairPluginDeviceView* GetDeviceView(std::int32_t index);
	void SetImageHasher(std::function<std::string(const std::string&)> functor) { mImageHasher = functor; };

protected:
	void GetDeviceInfoFromJson(const nlohmann::json& settings, const nlohmann::json& devices);
	bool GetDeviceViewFromJson(const nlohmann::json& settings, const nlohmann::json& devices);

	bool ReadZonesFromJson(const nlohmann::json& zone);
	void ReadViewFromJson(const nlohmann::json& view, DeviceView& deviceView);

private:
	RGBController* mController;
	std::function<std::string(const std::string&)> mImageHasher;
	DeviceInfo mDeviceInfo;
	DeviceViews mDeviceViews;
};