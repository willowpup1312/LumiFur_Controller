#pragma once
#include <Preferences.h>
#include "config/debug_config.h"

static constexpr char NAMESPACE[] = "LumiFur";
static constexpr char KEY_LAST[] = "lastView";

// Returns a reference to a static Preferences instance.
inline Preferences &getPrefs()
{
  static Preferences prefs;
  return prefs;
}

// Initializes the Preferences system; call this in setup().
inline void initPreferences()
{
  // Open the "LumiFur" namespace in read/write mode.
  getPrefs().begin("LumiFur", false);
}

// Debug mode functions
inline bool getDebugMode()
{
  return getPrefs().getBool("debug", true);
}
inline void setDebugMode(bool debugMode)
{
  getPrefs().putBool("debug", debugMode);
}

// Brightness functions
inline int getUserBrightness()
{
  return getPrefs().getInt("brightness", 255); // Default to 255 (full brightness)
}
inline void setUserBrightness(int brightness)
{
  getPrefs().putInt("brightness", brightness);
}

// Blink mode functions
inline bool getBlinkMode()
{
  return getPrefs().getBool("blinkmode", true);
}
inline void setBlinkMode(bool blinkMode)
{
  getPrefs().putBool("blinkmode", blinkMode);
}

// Sleep mode functions
inline bool getSleepMode()
{
  return getPrefs().getBool("sleepmode", true);
}
inline void setSleepMode(bool sleepMode)
{
  getPrefs().putBool("sleepmode", sleepMode);
}

// Dizzy mode functions
inline bool getDizzyMode()
{
  return getPrefs().getBool("dizzymode", true);
}
inline void setDizzyMode(bool dizzyMode)
{
  getPrefs().putBool("dizzymode", dizzyMode);
}

// Auto brightness functions
inline bool getAutoBrightness()
{
  return getPrefs().getBool("autobrightness", true);
}
inline void setAutoBrightness(bool autoBrightness)
{
  getPrefs().putBool("autobrightness", autoBrightness);
}

// Let microphone-driven mouth activity temporarily override the panel
// brightness. Disabled by default to preserve the user's power/brightness cap.
inline bool getMouthMicBrightnessOverride()
{
  return getPrefs().getBool("mouthmicmax", false);
}
inline void setMouthMicBrightnessOverride(bool enabled)
{
  getPrefs().putBool("mouthmicmax", enabled);
}

// Accelerometer functions
inline bool getAccelerometerEnabled()
{
  return getPrefs().getBool("accelerometer", true);
}
inline void setAccelerometerEnabled(bool enabled)
{
  getPrefs().putBool("accelerometer", enabled);
}

// Aurora mode functions
inline bool getAuroraMode()
{
  return getPrefs().getBool("auroramode", true);
}
inline void setAuroraMode(bool enabled)
{
  getPrefs().putBool("auroramode", enabled);
}

inline bool getStaticColorMode()
{
  return getPrefs().getBool("staticcolormode", false);
}
inline void setStaticColorMode(bool enabled)
{
  getPrefs().putBool("staticcolormode", enabled);
}

inline uint8_t getLastView()
{
  return getPrefs().getUChar(KEY_LAST, 0);
}

inline void saveLastView(uint8_t v)
{
  getPrefs().putUChar(KEY_LAST, v);
}

inline void saveUserText(const String &text)
{
  getPrefs().putString("userText", text);
}

inline String getUserText()
{
  return getPrefs().getString("userText", "");
}

inline void saveScrollSpeed(uint16_t speed)
{
  getPrefs().putUShort("scrollSpeed", speed);
}

inline uint16_t getScrollSpeed()
{
  return getPrefs().getUShort("scrollSpeed", 4);
}

inline void saveStrobeColorPreference(const String &text)
{
  getPrefs().putString("strobeColor", text);
}

inline String getStrobeColorPreference()
{
  return getPrefs().getString("strobeColor", "FFFFFF");
}

inline void saveStrobeSpeedPreference(uint16_t speedMs)
{
  getPrefs().putUShort("strobeSpeed", speedMs);
}

inline uint16_t getStrobeSpeedPreference()
{
  return getPrefs().getUShort("strobeSpeed", 120);
}

inline void saveSelectedColor(const String &text)
{
  getPrefs().putString("selectedColor", text);
}

inline String getSelectedColor()
{
  return getPrefs().getString("selectedColor", "");
}

// Unwritten NVS keys return the compile-time flag, so first boot matches the old #define.

// DEBUG_DISABLE_BLE_INDICATOR_LIGHT
inline bool getDisableBleIndicatorLight()
{
  return getPrefs().getBool("bleindoff", DEBUG_DISABLE_BLE_INDICATOR_LIGHT);
}
inline void setDisableBleIndicatorLight(bool v)
{
  getPrefs().putBool("bleindoff", v);
}

// DEBUG_DISABLE_BLE_STATUS_ICON
inline bool getDisableBleStatusIcon()
{
  return getPrefs().getBool("blestatusoff", DEBUG_DISABLE_BLE_STATUS_ICON);
}
inline void setDisableBleStatusIcon(bool v)
{
  getPrefs().putBool("blestatusoff", v);
}

// DEBUG_ENABLE_BRIGTHNESS_BOOST_WAVESHARE
inline bool getWaveshareBrightnessBoost()
{
  return getPrefs().getBool("wsbrightboost", DEBUG_ENABLE_BRIGTHNESS_BOOST_WAVESHARE);
}
inline void setWaveshareBrightnessBoost(bool v)
{
  getPrefs().putBool("wsbrightboost", v);
}

// Auto-brightness floor, 0–255. 15 matches the old hardcoded minimum.
inline uint8_t getAutoBrightnessFloor()
{
  return getPrefs().getUChar("autobrightfloor", 15);
}
inline void setAutoBrightnessFloor(uint8_t floor)
{
  getPrefs().putUChar("autobrightfloor", floor);
}
// Config byte 9 — face overlay mode (0 plasma, 1 matrix rain, 2 TV static, 3 solid).
// Migrates the old bool "matrixrain" key: if the new key is missing and the old
// bool was true, first read returns 1 so existing devices keep matrix rain.
inline uint8_t getOverlayMode()
{
  // 255 = never written (sentinel). Avoids Preferences::isKey for older cores.
  uint8_t m = getPrefs().getUChar("overlaymode", 255);
  if (m == 255)
  {
    // Legacy migration from matrixRainInsteadOfPlasma bool.
    return getPrefs().getBool("matrixrain", false) ? 1 : 0;
  }
  return (m > 3) ? 0 : m;
}
inline void setOverlayMode(uint8_t mode)
{
  if (mode > 3)
  {
    mode = 0;
  }
  getPrefs().putUChar("overlaymode", mode);
}
// 0 default, 1 advanced boop 1, 2 advanced boop 2, 3 advanced boop 4.
inline uint8_t getBoopMode()
{
  return getPrefs().getUChar("boopmode", 0);
}
inline void setBoopMode(uint8_t mode)
{
  getPrefs().putUChar("boopmode", mode);
}

// Fan speed 0–255 (255 = 100 %). Default full speed.
inline uint8_t getFanSpeed()
{
  return getPrefs().getUChar("fanspeed", 255);
}
inline void setFanSpeed(uint8_t speed)
{
  getPrefs().putUChar("fanspeed", speed);
}

// HUD / OLED enable (default on).
inline bool getHudEnabled()
{
  return getPrefs().getBool("hudenabled", true);
}
inline void setHudEnabled(bool enabled)
{
  getPrefs().putBool("hudenabled", enabled);
}

// OLED contrast 0x00–0xFF (default ultra-dim 0x01).
inline uint8_t getOledContrast()
{
  return getPrefs().getUChar("oledcontrast", 0x01);
}
inline void setOledContrast(uint8_t contrast)
{
  getPrefs().putUChar("oledcontrast", contrast);
}

// Clear all preferences - Implement feature to reset controller to default settings
inline void clearPreferences()
{
  getPrefs().clear();
}
