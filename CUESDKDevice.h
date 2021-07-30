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

enum CorsairSupportedFeatures : std::uint32_t
{
	CPSF_KeyEvents = (1 << 0),
	CPSF_KeyEventsConfiguration = (1 << 1),
	CPSF_DetachedMode = (1 << 2),
	CPSF_PluginProperties = (1 << 3),
	CPSF_DeviceProperties = (1 << 4),
	CPSF_DeviceChannels = (1 << 5)
};

enum CorsairSensorType : std::uint32_t
{
	CST_Invalid,
	CST_Temperature,
	CST_FanRpm,
	CST_PumpRpm,
	CST_Voltage,
	CST_Current,
	CST_Power
};

enum CorsairPropertyFlags : std::uint32_t
{
	CPF_CanRead = (1 << 0),
	CPF_CanWrite = (1 << 1),
	CPF_Indexed = (1 << 2)
};

enum CorsairPluginPropertyId : std::uint32_t
{
	CPPI_Invalid,
	CPPI_PropertiesList,
	CPPI_Locale
};

enum CorsairDevicePropertyId : std::uint32_t
{
	CPDPI_Invalid,
	CPDPI_PropertiesList,
	CPDPI_ChannelsCount,
	CPDPI_ChannelName,
	CPDPI_SensorsCount,
	CPDPI_SensorType,
	CPDPI_SensorName,
	CPDPI_SensorValue,
	CPDPI_FanPerformanceMode,
	CPDPI_FanPerformanceModesList,
	CPDPI_PerformanceCurveTemplateDefault,
	CPDPI_SensorValueMin,
	CPDPI_SensorValueMax,
	CPDPI_SingleColorZonesList
};

enum CorsairFanPerformanceMode : std::uint32_t
{
	CFPM_Invalid,
	CFPM_Default,
	CFPM_Quiet,
	CFPM_Balanced,
	CFPM_Performance,
	CFPM_FixedPwm,
	CFPM_FixedRpm,
	CFPM_Custom,
	CFPM_ZeroRpm
};

enum class CorsairPropertyDataType : std::uint32_t
{
	CPDT_Bool,
	CPDT_Integer,
	CPDT_Double,
	CPDT_String,
	CPDT_BoolArray = 16,
	CPDT_IntegerArray,
	CPDT_DoubleArray,
	CPDT_StringArray,
	CPDT_LedColorArray
};

enum CorsairMode : std::uint32_t
{
	Invalid,
	Detached,
	Attached
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
	CorsairPluginImage* view;			// 00 - Image shown in the editing view of the device
	CorsairPluginImage* mask;			// 04 - Masks the view image, white is opaque, black is transparent
	CorsairLedViews*	ledView;		// 08
	CorsairLedViews*	actionZones;	// 0C - I don't know what this is yet
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

// 2C
struct CorsairGetInstance_v66
{
	_CorsairPluginGetDeviceInfo getDeviceInfo;														// 00
	_CorsairSetLedsColors setLedsColors;															// 04
	_CorsairSubscribeForDeviceConnectionStatusChanges subscribeForDeviceConnectionStatusChanges;	// 08
	_CorsairPluginUnsubscribeFromDeviceStatusChanges unsubscribeFromDeviceConnectionStatusChanges;	// 0C
	_CorsairPluginGetDeviceView getDeviceView;														// 10
	_CorsairPluginFreeDeviceInfo freeDeviceInfo;													// 14
	_CorsairPluginFreeDeviceView freeDeviceView;													// 18
	_CorsairConfigureKeyEvent configureKeyEvent;													// 1C
	_CorsairSubscribeForEvents subscribeForEvents;													// 20
	_CorsairUnsubscribeFromEvents unsubscribeFromEvents;											// 24
	_CorsairSetMode setMode;																		// 28
};

struct CorsairPropertyData
{
	void* data;				// 00 Data
	std::int32_t count;		// 04
};

typedef bool (*_CorsairGetDevicePropertyInfo)(const char* deviceId, CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyDataType& dataType, std::int32_t& flags);
typedef bool (*_CorsairGetPluginPropertyInfo)(CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyDataType& dataType, std::int32_t& flags);
typedef CorsairPropertyData* (*_CorsairReadDevicePropertyData)(const char* deviceId, CorsairDevicePropertyId propertyId, std::int32_t index);
typedef CorsairPropertyData* (*_CorsairReadPluginPropertyData)(CorsairDevicePropertyId propertyId, std::int32_t index);
typedef bool (*_CorsairWriteDevicePropertyData)(const char* deviceId, CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyData&);
typedef bool (*_CorsairWritePluginPropertyData)(CorsairDevicePropertyId propertyId, std::int32_t index, CorsairPropertyData&);
typedef void (*_CorsairFreePropertyData)(CorsairPropertyDataType dataType, CorsairPropertyData* data);

// 4C
struct CorsairGetInstance_v67
{
	_CorsairPluginGetDeviceInfo getDeviceInfo;														// 00
	_CorsairSetLedsColors setLedsColors;															// 04
	_CorsairSubscribeForDeviceConnectionStatusChanges subscribeForDeviceConnectionStatusChanges;	// 08
	_CorsairPluginUnsubscribeFromDeviceStatusChanges unsubscribeFromDeviceConnectionStatusChanges;	// 0C
	_CorsairPluginGetDeviceView getDeviceView;														// 10
	_CorsairPluginFreeDeviceInfo freeDeviceInfo;													// 14
	_CorsairPluginFreeDeviceView freeDeviceView;													// 18
	_CorsairConfigureKeyEvent configureKeyEvent;													// 1C
	_CorsairSubscribeForEvents subscribeForEvents;													// 20
	_CorsairUnsubscribeFromEvents unsubscribeFromEvents;											// 24
	_CorsairSetMode setMode;																		// 28
	_CorsairGetPluginPropertyInfo getPropertyInfo;													// 2C
	_CorsairReadPluginPropertyData readPropertyData;												// 30
	_CorsairWritePluginPropertyData writePropertyData;												// 34
	_CorsairGetDevicePropertyInfo getDevicePropertyInfo;											// 38
	_CorsairReadDevicePropertyData readDevicePropertyData;											// 3C
	_CorsairWriteDevicePropertyData writeDevicePropertyData;										// 40
	_CorsairFreePropertyData freePropertyData;														// 44
	void* setLedsColorsAtChannel;																	// 48
};
