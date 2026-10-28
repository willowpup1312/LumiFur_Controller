#ifndef BLE_CONSTANTS_H
#define BLE_CONSTANTS_H

/**
 * @file ble_constants.h
 * @brief BLE UUIDs and firmware metadata for LumiFur Controller.
 *
 * The OLED_BRIGHTNESS characteristic was added so the companion app can
 * control the internal diagnostic HUD contrast (0x00–0xFF). Default 0x01
 * keeps the display ultra-dim.
 */

// ---------------------------------------------------------------------------
// Service & characteristic UUIDs
// ---------------------------------------------------------------------------
#define SERVICE_UUID "01931c44-3867-7740-9867-c822cb7df308"
#define CHARACTERISTIC_UUID "01931c44-3867-7427-96ab-8d7ac0ae09fe"
#define CONFIG_CHARACTERISTIC_UUID "01931c44-3867-7427-96ab-8d7ac0ae09ff"
#define TEMPERATURE_CHARACTERISTIC_UUID "01931c44-3867-7b5d-9774-18350e3e27db"
#define TEMPERATURE_LOGS_CHARACTERISTIC_UUID "0195eec2-ae6e-74a1-bcd5-215e2365477c"
#define COMMAND_CHARACTERISTIC_UUID "0195eec3-06d2-7fd4-a561-49493be3ee41"
#define BRIGHTNESS_CHARACTERISTIC_UUID "01931c44-3867-7427-96ab-8d7ac0ae09ef"

#define SCROLL_TEXT_CHARACTERISTIC_UUID "7f9b8b12-1234-4c55-9b77-a19d55aa0011"
#define STATIC_COLOR_CHARACTERISTIC_UUID "7f9b8b12-1234-4c55-9b77-a19d55aa0022"
#define STROBE_SETTINGS_CHARACTERISTIC_UUID "7f9b8b12-1234-4c55-9b77-a19d55aa0033"
#define LUX_CHARACTERISTIC_UUID "01931c44-3867-7427-96ab-8d7ac0ae09f0"

#define OTA_CHARACTERISTIC_UUID "01931c44-3867-7427-96ab-8d7ac0ae09ee"
#define INFO_CHARACTERISTIC_UUID "cba1d466-344c-4be3-ab3f-189f80dd7599"

#define AUTO_BRIGHTNESS_FLOOR_CHARACTERISTIC_UUID "BEB5483E-36E1-4688-B7F5-EA07361B26A9"

#define VIDEO_LIST_CHARACTERISTIC_UUID "7f9b8b12-1234-4c55-9b77-a19d55aa0044"

// New: internal OLED contrast control (uint8_t 0x00–0xFF)
#define OLED_BRIGHTNESS_CHARACTERISTIC_UUID "7f9b8b12-1234-4c55-9b77-a19d55aa0055"

// ---------------------------------------------------------------------------
// Firmware metadata (overridden by PlatformIO build flags when present)
// ---------------------------------------------------------------------------
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "5.1.0"
#endif

#define GIT_COMMIT GIT_BRANCH
#ifndef GIT_COMMIT
#define GIT_COMMIT "unknown"
#endif

#ifndef GIT_BRANCH
#ifdef DEBUG_MODE
#define GIT_BRANCH "dev"
#else
#define GIT_BRANCH "main"
#endif
#endif

#ifndef BUILD_DATE
#define BUILD_DATE __DATE__
#endif

#ifndef BUILD_TIME
#define BUILD_TIME __TIME__
#endif

#ifndef DEVICE_MODEL
#define DEVICE_MODEL "mps3"
#endif

#ifndef APP_COMPAT_VERSION
#define APP_COMPAT_VERSION "14.1.0"
#endif

#endif // BLE_CONSTANTS_H
