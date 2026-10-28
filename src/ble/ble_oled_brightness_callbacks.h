/**
 * @file ble_oled_brightness_callbacks.h
 * @brief NimBLE callback for the OLED brightness characteristic.
 *
 * Write a single byte (0x00–0xFF). The value is stored in the shared HUD
 * state and the low-priority HUD task applies it on the next notification.
 * Reading returns the current contrast.
 *
 * Integrate by creating the characteristic in your BLE server setup:
 *
 *   pOledBrightnessCharacteristic = pService->createCharacteristic(
 *       OLED_BRIGHTNESS_CHARACTERISTIC_UUID,
 *       NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
 *   pOledBrightnessCharacteristic->setCallbacks(new OledBrightnessCallbacks());
 *   pOledBrightnessCharacteristic->setValue(&gHudState.oledBrightness, 1);
 */

#ifndef BLE_OLED_BRIGHTNESS_CALLBACKS_H
#define BLE_OLED_BRIGHTNESS_CALLBACKS_H

#include <NimBLEDevice.h>
#include "core/hud/LumiFur_HUD_Task.h"
#include "ble/ble_constants.h"

class OledBrightnessCallbacks : public NimBLECharacteristicCallbacks {
public:
  void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override {
    (void)connInfo;
    std::string value = pCharacteristic->getValue();
    if (value.length() >= 1) {
      uint8_t contrast = static_cast<uint8_t>(value[0]);
      hudSetOledBrightness(contrast);
#if DEBUG_MODE
      Serial.printf("OLED brightness set to 0x%02X\n", contrast);
#endif
    }
  }

  void onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override {
    (void)connInfo;
    // Return the current value from shared state
    uint8_t current = 0x01;
    if (gHudStateMutex != nullptr &&
        xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      current = gHudState.oledBrightness;
      xSemaphoreGive(gHudStateMutex);
    }
    pCharacteristic->setValue(&current, 1);
  }
};

#endif // BLE_OLED_BRIGHTNESS_CALLBACKS_H
