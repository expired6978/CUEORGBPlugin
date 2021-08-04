#pragma once

#include "CorsairLedIdEnum.h"
#include <cstdint>

#define ENABLE_BITMASK_OPERATORS(ENUMTYPE) \
constexpr ENUMTYPE operator |(ENUMTYPE lhs, ENUMTYPE rhs) {return static_cast<ENUMTYPE> (static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) | static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));} \
constexpr ENUMTYPE operator &(ENUMTYPE lhs, ENUMTYPE rhs) {return static_cast<ENUMTYPE>(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) & static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));} \
constexpr ENUMTYPE operator ^(ENUMTYPE lhs, ENUMTYPE rhs) {return static_cast<ENUMTYPE> (static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) ^ static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));} \
constexpr ENUMTYPE operator ~(ENUMTYPE rhs) {return static_cast<ENUMTYPE> (~static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));} \
constexpr ENUMTYPE& operator |=(ENUMTYPE& lhs, ENUMTYPE rhs) {lhs = static_cast<ENUMTYPE>(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) | static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));return lhs;} \
constexpr ENUMTYPE& operator &=(ENUMTYPE& lhs, ENUMTYPE rhs) {lhs = static_cast<ENUMTYPE>(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) & static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));return lhs;} \
constexpr ENUMTYPE& operator ^=(ENUMTYPE& lhs, ENUMTYPE rhs) {lhs = static_cast<ENUMTYPE>(static_cast<std::underlying_type<ENUMTYPE>::type>(lhs) ^ static_cast<std::underlying_type<ENUMTYPE>::type>(rhs));return lhs;}

namespace cue
{
namespace dev
{
namespace plugin
{
struct Event;

enum class ZoneAppearance : std::uint32_t
{
	SVG_Path = 0,		// This seems to be a list of draw commands for a rectangle: "M 155 111 L 233 111 L 233 166 L 155 166 Z"
	UI_Text = 1,		// Doesn't seem to work? This should be the flow where it takes a text key and converts to UI text
	Literal_Text = 2	// Doesn't seem to work? This should be the flow that takes the text from 
};

enum class SupportedFeatures : std::uint32_t
{
	KeyEvents = (1 << 0),
	KeyEventsConfiguration = (1 << 1),
	DetachedMode = (1 << 2),
	PluginProperties = (1 << 3),
	DeviceProperties = (1 << 4),
	DeviceChannels = (1 << 5)
};
ENABLE_BITMASK_OPERATORS(SupportedFeatures);

enum SensorType : std::uint32_t
{
	Invalid,
	Temperature,
	FanRpm,
	PumpRpm,
	Voltage,
	Current,
	Power
};

enum class PropertyFlags : std::uint32_t
{
	CanRead = (1 << 0),
	CanWrite = (1 << 1),
	Indexed = (1 << 2)
};
ENABLE_BITMASK_OPERATORS(PropertyFlags);

enum class PluginPropertyId : std::uint32_t
{
	Invalid,
	PropertiesList,
	Locale
};

enum class DevicePropertyId : std::int32_t
{
	Invalid,
	PropertiesList,
	ChannelsCount, // Max 2 channels
	ChannelName,
	SensorsCount,
	SensorType,
	SensorName,
	SensorValue,
	FanPerformanceMode,
	FanPerformanceModesList,
	PerformanceCurveTemplateDefault,
	SensorValueMin,
	SensorValueMax,
	SingleColorZonesList
};

struct PropertyData
{
	union
	{
		bool* boolArray;
		std::int32_t* intArray;
		double* doubleArray;
		char** strArray;
		std::int32_t i32;
		std::int64_t s64;
		double f64;
		char* str;
		DevicePropertyId* devicePropertyArray;
		PluginPropertyId* pluginPropertyArray;
	} data;
	std::int32_t count;		// 04
};

enum class FanPerformanceMode : std::uint32_t
{
	Invalid,
	Default,
	Quiet,
	Balanced,
	Performance,
	FixedPwm,
	FixedRpm,
	Custom,
	ZeroRpm
};

enum class PropertyDataType : std::uint32_t
{
	Bool,
	Integer,
	Double,
	String,
	BoolArray = 16,
	IntegerArray,
	DoubleArray,
	StringArray,
	LedColorArray
};

enum class Mode : std::uint32_t
{
	Invalid,
	Detached,
	Attached
};

// 08
struct Image
{
	char* path;		// 00 00
	char* hash;		// 04 08 - This is a SHA256 of the image data at the provided path
};

struct LedColor		// contains information about led and its color.
{
	CorsairLedId ledId;		// identifier of LED to set.
	std::int32_t r;			// red   brightness[0..255].
	std::int32_t g;			// green brightness[0..255].
	std::int32_t b;			// blue  brightness[0..255].
};

// 20
struct LedPosition
{
	CorsairLedId ledId;		// 00
	std::int32_t unk04;		// 04 - Don't know?
	double x;				// 08 - Is this supposed to be flipped?
	double y;				// 10
	std::int32_t unk18;		// 18 - Don't know?
	std::int32_t unk1C;		// 1C - Don't know?
};

// 08
struct LedPositions
{
	std::int32_t		numberOfLed;	// 00 00
	LedPosition* ledPosition;			// 04 08
};

// 10
struct LedView
{
	CorsairLedId ledId;					// 00 00
	char* path;							// 04 08
	ZoneAppearance appearance;			// 08 10
	char* text;							// 0C 18
};

// 08
struct LedViews
{
	std::int32_t numberOfLed;	// 00 00
	LedView* view;				// 04 08
};

enum class DeviceType : std::uint32_t
{
	Invalid = 0,
	Keyboard,
	Mouse
};

// 1C
struct DeviceInfo
{
	char* deviceName;					// 00 00 - UI name of the device
	DeviceType deviceType;				// 04 04
	LedPositions* ledPositions;			// 08 08
	Image* thumbnail;					// 0C 10 - Thumbnail shown when editing a device
	char* deviceId;						// 10 18 - Unique name of the device, passed to other calls to determine what device it is
	std::int32_t numberOfDeviceView;	// 14 20 - Number of device views to display in the UI, this will result in multiple calls to CorsairPluginGetDeviceView with a different View index
	Image* promoImage;					// 18 28 - Image shown in the Devices summary
};

// 10
struct DeviceView
{
	Image* view;			// 00 00 - Image shown in the editing view of the device
	Image* mask;			// 04 08 - Masks the view image, white is opaque, black is transparent
	LedViews* ledView;		// 08 10
	LedViews* actionZones;	// 0C 18 - I don't know what this is yet
};
}
}
}

typedef cue::dev::plugin::DeviceInfo* (*_CorsairPluginGetDeviceInfo)(const char* deviceId);
typedef bool (*_CorsairSetLedsColors)(const char* deviceId, std::int32_t size, cue::dev::plugin::LedColor* ledsColors);
typedef std::int32_t(*_DeviceConnectionStatusChangeCallback)(void* context, const char* deviceName, std::int32_t unk3);
typedef void (*_CorsairSubscribeForDeviceConnectionStatusChanges)(void* context, _DeviceConnectionStatusChangeCallback callback);
typedef void (*_CorsairPluginUnsubscribeFromDeviceStatusChanges)();
typedef cue::dev::plugin::DeviceView* (*_CorsairPluginGetDeviceView)(const char* deviceId, std::int32_t index);
typedef void (*_CorsairPluginFreeDeviceInfo)(cue::dev::plugin::DeviceInfo* memory);
typedef void (*_CorsairPluginFreeDeviceView)(cue::dev::plugin::DeviceView* memory);
typedef bool (*_CorsairConfigureKeyEvent)(void* unk1, std::int32_t unk2);
typedef void (*CorsairEventHandler)(void* context, const cue::dev::plugin::Event* event);
typedef bool (*_CorsairSubscribeForEvents)(CorsairEventHandler onEvent, void* context);
typedef bool (*_CorsairUnsubscribeFromEvents)();
typedef void (*_CorsairSetMode)(std::int32_t mode);

// 2C
struct CorsairGetInstance_v66
{
	_CorsairPluginGetDeviceInfo getDeviceInfo;														// 00 00
	_CorsairSetLedsColors setLedsColors;															// 04 08
	_CorsairSubscribeForDeviceConnectionStatusChanges subscribeForDeviceConnectionStatusChanges;	// 08 10
	_CorsairPluginUnsubscribeFromDeviceStatusChanges unsubscribeFromDeviceConnectionStatusChanges;	// 0C 18
	_CorsairPluginGetDeviceView getDeviceView;														// 10 20
	_CorsairPluginFreeDeviceInfo freeDeviceInfo;													// 14 28
	_CorsairPluginFreeDeviceView freeDeviceView;													// 18 30
	_CorsairConfigureKeyEvent configureKeyEvent;													// 1C 38
	_CorsairSubscribeForEvents subscribeForEvents;													// 20 40
	_CorsairUnsubscribeFromEvents unsubscribeFromEvents;											// 24 48
	_CorsairSetMode setMode;																		// 28 50
};

typedef bool (*_CorsairGetPluginPropertyInfo)(cue::dev::plugin::PluginPropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyDataType& dataType, cue::dev::plugin::PropertyFlags& flags);
typedef cue::dev::plugin::PropertyData* (*_CorsairReadPluginPropertyData)(cue::dev::plugin::PluginPropertyId propertyId, std::int32_t index);
typedef bool (*_CorsairWritePluginPropertyData)(cue::dev::plugin::PluginPropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyData&);

typedef bool (*_CorsairGetDevicePropertyInfo)(const char* deviceId, cue::dev::plugin::DevicePropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyDataType& dataType, cue::dev::plugin::PropertyFlags& flags);
typedef cue::dev::plugin::PropertyData* (*_CorsairReadDevicePropertyData)(const char* deviceId, cue::dev::plugin::DevicePropertyId propertyId, std::int32_t index);
typedef bool (*_CorsairWriteDevicePropertyData)(const char* deviceId, cue::dev::plugin::DevicePropertyId propertyId, std::int32_t index, cue::dev::plugin::PropertyData&);

typedef void (*_CorsairFreePropertyData)(cue::dev::plugin::PropertyDataType dataType, cue::dev::plugin::PropertyData* data);

struct LedColorChannel
{
	void* unk00;
	cue::dev::plugin::LedColor* ledColors;
};
typedef void (*_CorsairSetLedColorAtChannel)(const char* deviceId, std::int32_t unk1, std::int32_t size, LedColorChannel& channel);

// 4C
struct CorsairGetInstance_v67
{
	_CorsairPluginGetDeviceInfo getDeviceInfo;														// 00 00
	_CorsairSetLedsColors setLedsColors;															// 04 08
	_CorsairSubscribeForDeviceConnectionStatusChanges subscribeForDeviceConnectionStatusChanges;	// 08 10
	_CorsairPluginUnsubscribeFromDeviceStatusChanges unsubscribeFromDeviceConnectionStatusChanges;	// 0C 18
	_CorsairPluginGetDeviceView getDeviceView;														// 10 20
	_CorsairPluginFreeDeviceInfo freeDeviceInfo;													// 14 28
	_CorsairPluginFreeDeviceView freeDeviceView;													// 18 30
	_CorsairConfigureKeyEvent configureKeyEvent;													// 1C 38
	_CorsairSubscribeForEvents subscribeForEvents;													// 20 40
	_CorsairUnsubscribeFromEvents unsubscribeFromEvents;											// 24 48
	_CorsairSetMode setMode;																		// 28 50
	_CorsairGetPluginPropertyInfo getPropertyInfo;													// 2C 58
	_CorsairReadPluginPropertyData readPropertyData;												// 30 60
	_CorsairWritePluginPropertyData writePropertyData;												// 34 68
	_CorsairGetDevicePropertyInfo getDevicePropertyInfo;											// 38 70
	_CorsairReadDevicePropertyData readDevicePropertyData;											// 3C 78
	_CorsairWriteDevicePropertyData writeDevicePropertyData;										// 40 80
	_CorsairFreePropertyData freePropertyData;														// 44 88
	_CorsairSetLedColorAtChannel setLedsColorsAtChannel;											// 48 90
};
