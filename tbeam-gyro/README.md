# TbeamTracker

## Menu

1. [TTN configurations](#ttn-configurations)   
2. [libraries](#software)  
  2.1 [LMIC](#end-devices-key-configuration)  
  2.2 [TinyGPS++](#lmic-library)  
  2.3 [AXP202X](#pinmap)
3. [TTGO Tbeam](#hardware)  
4. [Other issues](#other-issues)  
5. [Credits](#credits)  


## TTN configurations
For TTN configurations, just follow the guide [TTN Hello World](https://github.com/jamersonm/TTNHelloWorld).

## Libraries

#### [LMIC](https://github.com/mcci-catena/arduino-lmic)
By .pio file, this library will be automatically imported.    
On `/project_config/lmic_project_config.h` comment the line **`#define CFG_us915 1`** and uncomment the line **`//#define CFG_au915 1`**  
This file should be:  
```
// project-specific definitions
//#define CFG_eu868 1
//#define CFG_us915 1
#define CFG_au915 1
//#define CFG_as923 1
// #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP      /* for as923-JP; also define CFG_as923 */
//#define CFG_kr920 1
//#define CFG_in866 1
#define CFG_sx1276_radio 1
//#define LMIC_USE_INTERRUPTS
```

#### [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus)
By .pio file, this library will be automatically imported.

#### [AXP20X](https://github.com/lewisxhe/AXP202X_Library)
Import to .pio/libdeps/ttgo-t-beam/ directory

## Tbeam

## Other Issues

## Credits
