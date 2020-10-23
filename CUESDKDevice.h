#pragma once

#include "CorsairLedIdEnum.h"
#include <cstdint>

struct CorsairEvent;

enum class CorsairZoneAppearance : std::uint32_t
{
	CZA_SVG_Path = 0,		// This seems to be a list of draw commands for a rectangle: "M 155 111 L 233 111 L 233 166 L 155 166 Z"
	CZA_UI_Text = 1,		// Doesn't seem to work? This should be the flow where it takes a text key and converts to UI text
	CZA_Literal_Text = 2	// Doesn't seem to work? This should be the flow that takes the text from 
};

// 08
struct CorsairPluginImage
{
	char* path;		// 00
	char* hash;		// 04 - This is a SHA256 of the image data at the provided path
};

struct CorsairLedColor		// contains information about led and its color.
{
	CorsairLedId ledId;		// identifier of LED to set.
	std::int32_t r;			// red   brightness[0..255].
	std::int32_t g;			// green brightness[0..255].
	std::int32_t b;			// blue  brightness[0..255].
};

// 20
struct CorsairLedPosition
{
	CorsairLedId ledId;		// 00
	std::int32_t unk04;		// 04 - Don't know?
	double x;				// 08 - Is this supposed to be flipped?
	double y;				// 10
	std::int32_t unk18;		// 18 - Don't know?
	std::int32_t unk1C;		// 1C - Don't know?
};

// 08
struct CorsairLedPositions
{
	std::int32_t		numberOfLed;	// 00
	CorsairLedPosition* ledPosition;	// 04
};

// 10
struct CorsairLedView
{
	CorsairLedId ledId;					// 00
	char* path;							// 04
	CorsairZoneAppearance appearance;	// 08
	char* text;							// 0C
};

// 08
struct CorsairLedViews
{
	std::int32_t numberOfLed;	// 00
	CorsairLedView* view;		// 04
};

enum class CorsairDeviceType : std::uint32_t
{
	CDT_Invalid = 0,
	CDT_Keyboard,
	CDT_Mouse
};

// 1C
struct CorsairPluginDeviceInfo
{
	char* deviceName;					// 00 - UI name of the device
	CorsairDeviceType deviceType;		// 04
	CorsairLedPositions* ledPositions;	// 08
	CorsairPluginImage* thumbnail;		// 0C - Thumbnail shown when editing a device
	char* deviceId;						// 10 - Unique name of the device, passed to other calls to determine what device it is
	std::int32_t numberOfDeviceView;	// 14 - Number of device views to display in the UI, this will result in multiple calls to CorsairPluginGetDeviceView with a different View index
	CorsairPluginImage* promoImage;		// 18 - Image shown in the Devices summary
};

// 10
struct CorsairPluginDeviceView
{
	CorsairPluginImage* view;		// 00 - Image shown in the editing view of the device
	CorsairPluginImage* mask;		// 04 - Masks the view image, white is opaque, black is transparent
	CorsairLedViews*	ledView;	// 08
	CorsairLedViews*	unk0C;		// 0C - I don't know why there are 2 of these
};

typedef CorsairPluginDeviceInfo* (*_CorsairPluginGetDeviceInfo)(const char* deviceId);
typedef bool (*_CorsairSetLedsColors)(const char* deviceId, std::int32_t size, CorsairLedColor* ledsColors);
typedef std::int32_t(*_DeviceConnectionStatusChangeCallback)(void* context, const char* deviceName, std::int32_t unk3);
typedef void (*_CorsairSubscribeForDeviceConnectionStatusChanges)(void* context, _DeviceConnectionStatusChangeCallback callback);
typedef void (*_CorsairPluginUnsubscribeFromDeviceStatusChanges)();
typedef CorsairPluginDeviceView* (*_CorsairPluginGetDeviceView)(const char* deviceId, std::int32_t index);
typedef void (*_CorsairPluginFreeDeviceInfo)(CorsairPluginDeviceInfo* memory);
typedef void (*_CorsairPluginFreeDeviceView)(CorsairPluginDeviceView* memory);
typedef bool (*_CorsairConfigureKeyEvent)(void* unk1, std::int32_t unk2);
typedef void (*CorsairEventHandler)(void* context, const CorsairEvent* event);
typedef bool (*_CorsairSubscribeForEvents)(CorsairEventHandler onEvent, void* context);
typedef bool (*_CorsairUnsubscribeFromEvents)();
typedef void (*_CorsairSetMode)(std::int32_t mode);

struct CorsairGetInstance
{
	_CorsairPluginGetDeviceInfo CorsairPluginGetDeviceInfo;													// 00
	_CorsairSetLedsColors CorsairPluginSetLedsColors;														// 04
	_CorsairSubscribeForDeviceConnectionStatusChanges CorsairSubscribeForDeviceConnectionStatusChanges;		// 08
	_CorsairPluginUnsubscribeFromDeviceStatusChanges CorsairPluginUnsubscribeFromDeviceStatusChanges;		// 0C
	_CorsairPluginGetDeviceView CorsairPluginGetDeviceView;													// 10
	_CorsairPluginFreeDeviceInfo CorsairPluginFreeDeviceInfo;												// 14
	_CorsairPluginFreeDeviceView CorsairPluginFreeDeviceView;												// 18
	_CorsairConfigureKeyEvent CorsairConfigureKeyEvent;														// 1C
	_CorsairSubscribeForEvents CorsairSubscribeForEvents;													// 20
	_CorsairUnsubscribeFromEvents CorsairUnsubscribeFromEvents;												// 24
	_CorsairSetMode CorsairSetMode;																			// 28
};