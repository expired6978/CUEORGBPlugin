#include <iostream>
#include <thread>
#include <fstream>
#include <tchar.h>
#include <sstream>
#include <future>
#include <sha256/picosha2.h>
#include "NetworkClient.h"
#include "RGBController.h"
#include "CorsairPluginDeviceManager.h"
#include "CorsairPluginDevice.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std::chrono_literals;
using json = nlohmann::json;

std::unique_ptr<CorsairPluginDeviceManager> g_deviceManager = nullptr;

std::int32_t DeviceConnectionStatusChangeCallback(void* context, const char* deviceName, std::int32_t unk3)
{
	std::cout << "Device: " << deviceName << " Status: " << unk3 << std::endl;
	return 0;
}

std::wstring GetLocalFile(const std::wstring& relativePath)
{
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW((HINSTANCE)&__ImageBase, path, MAX_PATH);
	PathRemoveFileSpecW(path);
	PathCombineW(path, path, L"..\\..\\dist\\plugins\\OpenRGB");
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

int main()
{
	g_deviceManager = std::make_unique<CorsairPluginDeviceManager>(nullptr, DeviceConnectionStatusChangeCallback, GetImageHash, nullptr, GetLocalFile);
	g_deviceManager->Start();

	std::string deviceCommand;

	while (true)
	{
		std::cout << "Enter JSON Command:" << std::endl;
		std::getline(std::cin, deviceCommand);

		nlohmann::json jd;
		try
		{
			jd = nlohmann::json::parse(deviceCommand);

			if (jd.contains("device") && jd["device"].is_string() && jd.contains("leds") && jd["leds"].is_array())
			{
				auto& ledArray = jd["leds"];
				auto leds = std::make_unique<cue::dev::plugin::LedColor[]>(ledArray.size());
				for (std::int32_t i = 0; i < ledArray.size(); ++i)
				{
					const auto& ledObject = ledArray[i];
					if (ledObject.is_object())
					{
						leds[i].ledId = ledObject["id"];
						leds[i].r = ledObject["r"];
						leds[i].g = ledObject["g"];
						leds[i].b = ledObject["b"];
					}
				}

				g_deviceManager->SetColor(jd["device"].get<std::string>().c_str(), ledArray.size(), leds.get());
			}
		}
		catch (const std::string& ex)
		{
			std::cout << "Exception: " << ex << std::endl;
		}
	}

	return 0;
}
