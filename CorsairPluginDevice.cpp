#include "CorsairPluginDevice.h"
#include "RGBController.h"
#include "CUESDKDevice.h"

#include <json/json.hpp>
#include <sstream>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

using json = nlohmann::json;

bool CorsairPluginDevice::ReadFromJson(const nlohmann::json& settings, const nlohmann::json& devices, bool clear)
{
	if (clear)
	{
		mDeviceInfo.zones.clear();
		mDeviceInfo.ledMapping.clear();
		mDeviceViews.clear();
	}
	try
	{
		GetDeviceInfoFromJson(settings, devices);
		GetDeviceViewFromJson(settings, devices);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

cue::dev::plugin::DeviceInfo* CorsairPluginDevice::CreateDeviceInfo()
{
	cue::dev::plugin::DeviceInfo* deviceInfo = new cue::dev::plugin::DeviceInfo;
	deviceInfo->deviceName = _strdup(mDeviceInfo.deviceName.c_str());
	deviceInfo->deviceId = _strdup(mDeviceInfo.deviceId.c_str());
	deviceInfo->deviceType = cue::dev::plugin::DeviceType::Keyboard; // We are using keyboard because it treats the polys as buttons and doesn't litter the screen with key buttons
	deviceInfo->numberOfDeviceView = static_cast<std::int32_t>(mDeviceViews.size());
	if (mDeviceInfo.thumbnail.size())
	{
		deviceInfo->thumbnail = new cue::dev::plugin::Image;
		deviceInfo->thumbnail->path = _strdup(mDeviceInfo.thumbnail.c_str());
		deviceInfo->thumbnail->hash = mImageHasher ? _strdup(mImageHasher(mDeviceInfo.thumbnail).c_str()) : _strdup("");
	}
	else
	{
		deviceInfo->thumbnail = nullptr;
	}

	if (mDeviceInfo.promo.size())
	{
		deviceInfo->promoImage = new cue::dev::plugin::Image;
		deviceInfo->promoImage->path = _strdup(mDeviceInfo.promo.c_str());
		deviceInfo->promoImage->hash = mImageHasher ? _strdup(mImageHasher(mDeviceInfo.promo).c_str()) : _strdup("");
	}
	else
	{
		deviceInfo->promoImage = nullptr;
	}

	deviceInfo->ledPositions = new cue::dev::plugin::LedPositions;

	std::int32_t totalLEDs = 0;
	for (const auto& zone : mDeviceInfo.zones)
	{
		totalLEDs += static_cast<std::int32_t>(zone.second.ledData.size());
	}

	deviceInfo->ledPositions->numberOfLed = totalLEDs;
	deviceInfo->ledPositions->ledPosition = new cue::dev::plugin::LedPosition[totalLEDs];

	for (std::int32_t i = 0; i < totalLEDs; ++i)
	{
		auto& led = deviceInfo->ledPositions->ledPosition[i];
		led.ledId = CorsairLedId::CLI_Invalid;
		led.unk04 = 0;
		led.unk18 = 0;
		led.unk1C = 0;
		led.x = 0;
		led.y = 0;
	}

	std::int32_t index = 0;
	for (const auto& zone : mDeviceInfo.zones)
	{
		for (const auto& led : zone.second.ledData)
		{
			auto& ledPosition = deviceInfo->ledPositions->ledPosition[index];
			ledPosition.ledId = static_cast<CorsairLedId>(led.first);
			ledPosition.x = led.second.x;
			ledPosition.y = led.second.y;
			index++;
		}
	}

	return deviceInfo;
}

cue::dev::plugin::DeviceView* CorsairPluginDevice::CreateDeviceView(std::int32_t index)
{
	if (static_cast<size_t>(index) < mDeviceViews.size())
	{
		auto viewIt = mDeviceViews.begin();
		std::advance(viewIt, index);
		auto& view = viewIt->second;

		cue::dev::plugin::DeviceView* deviceView = new cue::dev::plugin::DeviceView;
		if (view.image.size())
		{
			deviceView->view = new cue::dev::plugin::Image;
			deviceView->view->path = _strdup(view.image.c_str());
			deviceView->view->hash = mImageHasher ? _strdup(mImageHasher(view.image).c_str()) : _strdup("");
		}
		else
		{
			deviceView->view = nullptr;
		}

		if (view.mask.size())
		{
			deviceView->mask = new cue::dev::plugin::Image;
			deviceView->mask->path = _strdup(view.mask.c_str());
			deviceView->mask->hash = mImageHasher ? _strdup(mImageHasher(view.mask).c_str()) : _strdup("");
		}
		else
		{
			deviceView->mask = nullptr;
		}

		deviceView->ledView = new cue::dev::plugin::LedViews;
		deviceView->ledView->numberOfLed = static_cast<std::int32_t>(view.drawPath.size());
		deviceView->ledView->view = new cue::dev::plugin::LedView[view.drawPath.size()];

		std::int32_t index = 0;
		for (auto& led : view.drawPath)
		{
			auto& ledAppearance = deviceView->ledView->view[index];
			ledAppearance.ledId = static_cast<CorsairLedId>(led.first);
			ledAppearance.path = _strdup(led.second.c_str());
			ledAppearance.appearance = cue::dev::plugin::ZoneAppearance::SVG_Path;
			ledAppearance.text = nullptr;
			index++;
		}
		deviceView->actionZones = nullptr;

		return deviceView;
	}

	return nullptr;
}

void CorsairPluginDevice::DestroyDeviceInfo(cue::dev::plugin::DeviceInfo* deviceInfo)
{
	if (deviceInfo->ledPositions)
	{
		if (deviceInfo->ledPositions->ledPosition)
		{
			delete[] deviceInfo->ledPositions->ledPosition;
		}

		delete deviceInfo->ledPositions;
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

		delete deviceInfo->promoImage;
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

		delete deviceInfo->thumbnail;
	}

	if (deviceInfo->deviceId)
	{
		free(deviceInfo->deviceId);
	}

	if (deviceInfo->deviceName)
	{
		free(deviceInfo->deviceName);
	}

	delete deviceInfo;
}

void CorsairPluginDevice::DestroyDeviceView(cue::dev::plugin::DeviceView* deviceView)
{
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

			delete[] deviceView->ledView->view;
		}

		delete deviceView->ledView;
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

		delete deviceView->mask;
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
		delete deviceView->view;
	}

	delete deviceView;
}


bool CorsairPluginDevice::ReadZonesFromJson(const json& zone)
{
	std::int32_t zoneIndex = -1;
	if (!zone.contains("Zone"))
	{
		return false;
	}

	if (zone["Zone"].is_number())
	{
		zoneIndex = zone["Zone"];
	}
	else if (zone["Zone"].is_string())
	{
		for (std::size_t z = 0; z < mController->zones.size(); ++z)
		{
			if (mController->zones[z].name == zone["Zone"])
			{
				zoneIndex = static_cast<std::int32_t>(z);
				break;
			}
		}
	}

	if (static_cast<size_t>(zoneIndex) > mController->zones.size())
	{
		return false;
	}

	if (zone.contains("Size"))
	{
		std::uint32_t newSize = zone["Size"];
		if (mController->zones[zoneIndex].leds_count != newSize)
		{
			mZoneResize[zoneIndex] = newSize;
		}
	}

	if (!mController->zones[zoneIndex].leds_count)
	{
		return false;
	}

	if (zone.contains("LEDs"))
	{
		if (zone["LEDs"].is_array())
		{
			for (const auto& led : zone["LEDs"])
			{
				std::uint32_t ledId = led["Id"];
				auto& ledData = mDeviceInfo.zones[zoneIndex].ledData[ledId];

				auto& ledMapping = mDeviceInfo.ledMapping[ledId];
				ledMapping.first = zoneIndex;
				if (led.contains("Index"))
				{
					ledMapping.second = led["Index"];
				}

				if (led.contains("Position") && led["Position"].is_array())
				{
					ledData.x = led["Position"][0];
					ledData.y = led["Position"][1];
				}
			}
		}
	}
	else
	{
		std::function<void(LEDData&, std::uint32_t, std::uint32_t)> patternFunc = [](LEDData& ledData, std::uint32_t led, std::uint32_t count)
		{
			ledData.x = std::sin(2 * M_PI * (led + 1) / count) + 1;
			ledData.y = std::cos(2 * M_PI * (led + 1) / count) + 1;
		};

		if (zone.contains("Pattern"))
		{
			if (zone["Pattern"] == "LinearX")
			{
				patternFunc = [](LEDData& ledData, std::uint32_t led, std::uint32_t count)
				{
					ledData.x = (double)led / (double)count;
					ledData.y = 1.0;
				};
			}
			else if (zone["Pattern"] == "LinearY")
			{
				patternFunc = [](LEDData& ledData, std::uint32_t led, std::uint32_t count)
				{
					ledData.x = 1.0;
					ledData.y = (double)led / (double)count;
				};
			}
		}

		std::uint32_t ledCount = mController->zones[zoneIndex].leds_count;
		for (std::uint32_t led = 0; led < ledCount; ++led)
		{
			std::uint32_t ledId = mController->zones[zoneIndex].start_idx + led + 1; // Plus one because 0 is invalid key
			auto& ledData = mDeviceInfo.zones[zoneIndex].ledData[ledId];
			auto& ledMapping = mDeviceInfo.ledMapping[ledId];
			ledMapping.first = zoneIndex;
			ledMapping.second = led;
			patternFunc(ledData, led, ledCount);
		}
	}

	return true;
}

void CorsairPluginDevice::GetDeviceInfoFromJson(const json& settings, const json& devices)
{
	mDeviceInfo.deviceName = mController->name;
	mDeviceInfo.deviceId = mDeviceHasher ? mDeviceHasher(mController->location) : mController->location;

	auto deviceTypeName = device_type_to_str(mController->type);
	if (settings.contains("Defaults"))
	{
		const auto defaults = settings["Defaults"];
		if (defaults.contains(deviceTypeName))
		{
			auto deviceDefault = defaults[deviceTypeName];

			mDeviceInfo.thumbnail = deviceDefault["Thumbnail"];
			mDeviceInfo.promo = deviceDefault["Image"];

			if (deviceDefault.contains("Zones") && deviceDefault["Zones"].is_array())
			{
				for (const auto& zone : deviceDefault["Zones"])
				{
					ReadZonesFromJson(zone);
				}
			}
		}
	}


	json device;
	for (const auto& deviceObject : devices)
	{
		// Match device by location
		if (deviceObject.contains("Location") && mController->location == deviceObject["Location"])
		{
			// Allow the device to be renamed if the Location is specified
			if (deviceObject.contains("Name"))
			{
				mDeviceInfo.deviceName = deviceObject["Name"];
			}
			device = deviceObject;
			break;
		}

		// Match device by name
		if (deviceObject.contains("Name") && mDeviceInfo.deviceName == deviceObject["Name"])
		{
			device = deviceObject;
			break;
		}
	}

	if (device.is_object())
	{
		if (device.contains("InheritDefault"))
		{
			if (!device["InheritDefault"])
			{
				mDeviceInfo.zones.clear();
				mDeviceInfo.ledMapping.clear();
			}
		}

		if (device.contains("Zones") && device["Zones"].is_array())
		{
			for (const auto& zone : device["Zones"])
			{
				ReadZonesFromJson(zone);
			}
		}
	}
}

void CorsairPluginDevice::ReadViewFromJson(const json& view, DeviceView& deviceView)
{
	deviceView.image = view["Image"];
	if (view.contains("Mask"))
	{
		deviceView.mask = view["Mask"];
	}

	if (view.contains("PolyGenerator"))
	{
		auto& polyGenerator = view["PolyGenerator"];
		if (polyGenerator.contains("Rect") && polyGenerator.contains("Spacing") && polyGenerator.contains("Zone"))
		{
			std::int32_t zoneIndex = -1;
			if (polyGenerator["Zone"].is_number())
			{
				zoneIndex = polyGenerator["Zone"];
			}
			else if (polyGenerator["Zone"].is_string())
			{
				for (std::size_t z = 0; z < mController->zones.size(); ++z)
				{
					if (mController->zones[z].name == polyGenerator["Zone"])
					{
						zoneIndex = static_cast<std::int32_t>(z);
						break;
					}
				}
			}

			if (polyGenerator["Rect"].is_array() && polyGenerator["Rect"].size() == 4)
			{
				std::int32_t spacing = polyGenerator["Spacing"];
				auto& rect = polyGenerator["Rect"];
				auto it = mDeviceInfo.zones.find(zoneIndex);
				if (it != mDeviceInfo.zones.end())
				{
					std::size_t ledZoneCount = it->second.ledData.size();
					std::int32_t totalSpacing = ledZoneCount ? (static_cast<std::int32_t>(ledZoneCount) - 1) * spacing : 0;
					std::int32_t left = rect[0];
					std::int32_t top = rect[1];
					std::int32_t right = rect[2];
					std::int32_t bottom = rect[3];

					std::int32_t totalWidth = right - left;
					std::int32_t totalHeight = bottom - top;

					std::int32_t rectWidth = std::abs(totalWidth - totalSpacing) / static_cast<std::int32_t>(ledZoneCount);

					std::int32_t idx = 1;
					for (const auto& led : it->second.ledData)
					{
						std::ostringstream stringStream;
						stringStream << "M " << left << " " << top << " L " << (left + rectWidth) << " " << top << " L " << (left + rectWidth) << " " << (top + totalHeight) << " L " << left << " " << (top + totalHeight) << " Z";
						deviceView.drawPath[led.first] = stringStream.str();
						left += rectWidth + spacing;
						idx++;
					}
				}
			}
		}
	}
	else if (view.contains("Polygons") && view["Polygons"].is_array())
	{
		for (const auto& polygon : view["Polygons"])
		{
			deviceView.drawPath[polygon["Id"]] = polygon["Path"];
		}
	}
}

bool CorsairPluginDevice::GetDeviceViewFromJson(const json& settings, const json& devices)
{
	auto deviceTypeName = device_type_to_str(mController->type);
	if (!settings.contains("Defaults") || !settings["Defaults"].contains(deviceTypeName))
	{
		return false;
	}

	const auto deviceDefault = settings["Defaults"][deviceTypeName];
	if (deviceDefault.is_object())
	{
		if (deviceDefault["Views"].is_array())
		{
			for (std::size_t i = 0; i < deviceDefault["Views"].size(); ++i)
			{
				ReadViewFromJson(deviceDefault["Views"][i], mDeviceViews[static_cast<std::int32_t>(i)]);
			}
		}
	}

	json device;
	for (const auto& deviceObject : devices)
	{
		// Match device by location
		if (deviceObject.contains("Location") && mController->location == deviceObject["Location"])
		{
			device = deviceObject;
			break;
		}

		// Match device by name
		if (deviceObject.contains("Name") && mDeviceInfo.deviceName == deviceObject["Name"])
		{
			device = deviceObject;
			break;
		}
	}

	if (device.is_object())
	{
		if (device.contains("InheritDefault"))
		{
			if (!device["InheritDefault"])
			{
				mDeviceViews.clear();
			}
		}

		for (std::size_t i = 0; i < device["Views"].size(); ++i)
		{
			ReadViewFromJson(device["Views"][i], mDeviceViews[static_cast<std::int32_t>(i)]);
		}
	}

	for (auto it = mDeviceViews.begin(); it != mDeviceViews.end();)
	{
		if (!it->second.drawPath.size())
		{
			it = mDeviceViews.erase(it);
			continue;
		}
		++it;
	}

	return true;
}

