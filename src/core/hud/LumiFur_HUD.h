#ifndef LUMIFUR_HUD_H
#define LUMIFUR_HUD_H

/**
 * @file LumiFur_HUD.h
 * @brief Internal diagnostic OLED HUD for LumiFur Controller.
 *
 * Ultra-dim monochrome SSD1306 (128x64) mounted inside the helmet lining.
 * Shared on the main I2C bus (default address 0x3C) with the PN532 NFC reader.
 * Contrast is driven to 0x01 by default so the display does not bleed light
 * through the 2mm PETG visor while remaining readable from inside the suit.
 *
 * Design constraints (important):
 * - Event-driven only. The display is redrawn exclusively when LumiFurSystemState
 *   changes. No continuous polling or fixed-rate updates.
 * - Runs in its own low-priority FreeRTOS task so it never blocks matrix
 *   rendering, sensor reads, BLE work, animation, or any other real-time task.
 * - App-controllable brightness via the oledBrightness field (written over BLE).
 *
 * Drop this header into src/hardware/ (or include/) and call the integration
 * helpers from setup() and from any place that changes relevant state.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// Display geometry and I2C address (shared bus with PN532)
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

/**
 * System operating and interaction modes.
 * Values are stable and map directly to the BLE SYSTEM_MODE characteristic.
 */
enum SystemOperatingMode : uint8_t {
  MODE_STANDARD = 0,       ///< Normal app/remote control
  MODE_SLEEP,              ///< Sleep / dimmed state
  MODE_BOOP_TRIGGER,       ///< Physical boop sensor triggered a temporary face
  MODE_DIZZY,              ///< Dizzy / spinning matrix animation
  MODE_PROX_NEAR,          ///< Another LumiFur is nearby (RSSI threshold)
  MODE_INT_HEARTS,         ///< Hearts sync interaction
  MODE_INT_ANGRY,          ///< Angry counter-reaction interaction
  MODE_STROBE_TX,          ///< Strobe transmission mode
  MODE_VIRUS_ANIM,         ///< Virus infection animation in progress
  INT_VIRUS_STATIC,        ///< Fully infected – TV static face
  MODE_BLE_PAIRING         ///< Secure BLE pairing overlay active
};

/**
 * Unified system state passed to the HUD.
 * The inequality operator is used to skip redundant redraws.
 * All string fields are short; keep them concise to stay within 128 px width.
 */
struct LumiFurSystemState {
  uint8_t faceIndex = 0;                 ///< Current face / view index
  String currentFaceName = "";           ///< Human-readable face name
  String overlayName = "";               ///< Active overlay (Plasma, Matrix Rain, etc.)
  uint16_t lux = 0;                      ///< Ambient light reading
  bool boopActive = false;               ///< True while a boop is being processed
  SystemOperatingMode sysMode = MODE_STANDARD;
  String modeDetailText = "";            ///< Extra detail (speed, filename, Hz, etc.)
  String pairingPasskey = "";            ///< 6-digit passkey during pairing
  uint8_t matrixBrightness = 100;        ///< Matrix brightness 0–100 %
  uint8_t fanSpeed = 0;                  ///< Suit cooling fan speed 0–100 %
  uint8_t oledBrightness = 0x01;         ///< OLED contrast 0x00–0xFF (app controllable)
  bool nfcPower = false;                 ///< PN532 power / enable state
  bool bleConnected = false;             ///< True when a central is connected

  bool operator!=(const LumiFurSystemState &other) const {
    return faceIndex != other.faceIndex ||
           currentFaceName != other.currentFaceName ||
           overlayName != other.overlayName ||
           lux != other.lux ||
           boopActive != other.boopActive ||
           sysMode != other.sysMode ||
           modeDetailText != other.modeDetailText ||
           pairingPasskey != other.pairingPasskey ||
           matrixBrightness != other.matrixBrightness ||
           fanSpeed != other.fanSpeed ||
           oledBrightness != other.oledBrightness ||
           nfcPower != other.nfcPower ||
           bleConnected != other.bleConnected;
  }
};

/**
 * Diagnostic HUD driver.
 * Instantiate once, call begin() from setup(), then feed it state via
 * updateHUDOnChange(). The class itself is not thread-safe; call it only
 * from the dedicated HUD task (see LumiFur_HUD_Task).
 */
class LumiFurHUD {
private:
  Adafruit_SSD1306 display;
  LumiFurSystemState lastState;
  bool stateInitialized = false;

  // Build the two-line Zone 4 banner from the current mode
  void getSystemModeLines(const LumiFurSystemState &state, String &line1, String &line2) {
    if (state.sysMode == MODE_BLE_PAIRING) {
      line1 = "** SECURE BLE PAIRING **";
      line2 = "Passkey: " + (state.pairingPasskey.length() > 0 ? state.pairingPasskey : "------");
      return;
    }

    switch (state.sysMode) {
      case MODE_STANDARD:
        line1 = "Mode: STANDARD BEHAVIOR";
        line2 = "Sub:  App & Remote Active";
        break;
      case MODE_SLEEP:
        line1 = "Mode: SLEEP MODE";
        line2 = "Sub:  [ zzzzzz... ]";
        break;
      case MODE_BOOP_TRIGGER:
        line1 = "Mode: BOOP SENSOR ACTIVE";
        line2 = "Sub:  Temp Face Swap";
        break;
      case MODE_DIZZY:
        line1 = "Mode: DIZZY ANIMATION";
        line2 = "Sub:  Spinning Matrix";
        break;
      case MODE_PROX_NEAR:
        line1 = "Mode: TARGET NEARBY";
        line2 = "Sub:  Ready to Sync";
        break;
      case MODE_INT_HEARTS:
        line1 = "Mode: SYNC [HEARTS]";
        line2 = "Sub:  Co-op Animation";
        break;
      case MODE_INT_ANGRY:
        line1 = "Mode: SYNC [ANGRY]";
        line2 = "Sub:  Counter-Reaction";
        break;
      case MODE_STROBE_TX:
        line1 = "Mode: STROBE TRANSMISSION";
        line2 = "Sub:  " + (state.modeDetailText.length() > 0 ? state.modeDetailText : "15Hz Sync Active");
        break;
      case MODE_VIRUS_ANIM:
        line1 = "Mode: VIRUS ANIMATION";
        line2 = "Sub:  Infecting Visor...";
        break;
      case INT_VIRUS_STATIC:
        line1 = "Mode: INFECTED STATE";
        line2 = "Sub:  TV Static Face";
        break;
      default:
        line1 = "Mode: DYNAMIC / CUSTOM";
        line2 = "Sub:  " + state.modeDetailText;
        break;
    }
  }

public:
  LumiFurHUD() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

  /**
   * Initialise the OLED.
   * @param defaultContrast Initial contrast register value (0x01 = ultra-dim).
   * @return true if the display responded, false otherwise (safe to ignore).
   */
  bool begin(uint8_t defaultContrast = 0x01) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      return false;
    }
    setBrightness(defaultContrast);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    return true;
  }

  /**
   * Write a new contrast value to the SSD1306.
   * Called automatically when oledBrightness changes; can also be called directly.
   */
  void setBrightness(uint8_t contrastLevel) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(contrastLevel);
  }

  /**
   * Redraw the HUD only if the supplied state differs from the last drawn state.
   * Extremely cheap when nothing has changed.
   */
  void updateHUDOnChange(const LumiFurSystemState &newState) {
    if (!stateInitialized || newState != lastState) {
      // Apply contrast change immediately if the app updated it
      if (!stateInitialized || newState.oledBrightness != lastState.oledBrightness) {
        setBrightness(newState.oledBrightness);
      }

      display.clearDisplay();

      // Zone 1 – Face index, name and BLE status (y = 0)
      display.setCursor(0, 0);
      display.print("Idx:" + String(newState.faceIndex) + " " + newState.currentFaceName);
      display.setCursor(76, 0);
      if (newState.sysMode == MODE_BLE_PAIRING) {
        display.print(F("BLE:PAIR"));
      } else {
        display.print(newState.bleConnected ? F("BLE:OK") : F("BLE:OFF"));
      }

      // Zone 2 – Overlay and ambient light (y = 9)
      display.setCursor(0, 9);
      display.print("Ovrl: " + newState.overlayName);
      display.setCursor(85, 9);
      display.print(String(newState.lux) + "lx");

      // Zone 3 – Boop trigger and proximity interaction (y = 18)
      display.setCursor(0, 18);
      display.print("Boop:");
      display.print(newState.boopActive ? "[HIT!]" : "---");
      display.setCursor(68, 18);
      display.print("Int:");
      display.print((newState.sysMode >= MODE_PROX_NEAR && newState.sysMode != MODE_BLE_PAIRING) ? "[YES]" : "NO");

      // Zone 4 – Two-line mode banner (y = 28 and y = 37)
      String line1, line2;
      getSystemModeLines(newState, line1, line2);
      display.setCursor(0, 28);
      display.print(line1);
      display.setCursor(0, 37);
      display.print(line2);

      // Zone 5 – Compact hardware footer (y = 52)
      display.setCursor(0, 52);
      display.print("Mtx:" + String(newState.matrixBrightness) + "%");
      display.setCursor(44, 52);
      display.print("Fan:" + String(newState.fanSpeed) + "%");
      display.setCursor(90, 52);
      display.print(newState.nfcPower ? "NFC:ON" : "NFC:OFF");

      display.display();

      lastState = newState;
      stateInitialized = true;
    }
  }
};

#endif // LUMIFUR_HUD_H
