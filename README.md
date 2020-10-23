# CUEORGBPlugin
 Custom iCUE plugin to control OpenRGB from within iCUE
 
# Description
This plugin allows creating custom device layouts using json files and custom images as well as generating some generic LED layouts if there is no defined device. Corsair has not released an official SDK for adding custom devices. The reverse engineering is contained in ```CUESDKDevices.h```. 

**Files of Importance**
* ```dsound.dll``` - This is a wrapper dll to disable iCUE's signature check on plugins loaded from the Plugins folder. Normally iCUE will run WinVerifyTrust on all plugins it is attempting to load. I haven't determined whether it needs to be signed by Corsair specifically, this wrapper when placed in the Corsair iCUE program directory will disable this check entirely.
* ```CUEORGBPlugin.dll``` - This implements the OpenRGB Client interface and utilizes/merges json files to create devices
* ```settings.json``` - This file specifies the default LED/Zone layout display of some devices in the event no specific device is found within ```devices.json```
* ```devices.json``` - This file maps devices by name given within OpenRGB and allows overriding the Default layout
 
# Installation
* Copy ```dist/plugins``` folder to ```C:\Program Files (x86)\Corsair\CORSAIR iCUE Software```
* Copy built ```dsound.dll``` to ```C:\Program Files (x86)\Corsair\CORSAIR iCUE Software```
* Copy built ```CUEORGBPlugin.dll``` to ```C:\Program Files (x86)\Corsair\CORSAIR iCUE Software\plugins\OpenRGB```

# Thirdparty Projects used

CUESDK
https://github.com/CorsairOfficial/cue-sdk

OpenRGB
https://gitlab.com/CalcProgrammer1/OpenRGB

Json C++
https://github.com/nlohmann/json

SHA256
https://github.com/okdshin/PicoSHA2
