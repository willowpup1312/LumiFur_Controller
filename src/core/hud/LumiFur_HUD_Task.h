#ifndef LUMIFUR_HUD_TASK_H
#define LUMIFUR_HUD_TASK_H

/**
 * @file LumiFur_HUD_Task.h
 * @brief Low-priority FreeRTOS task that owns the internal OLED HUD.
 *
 * The task blocks on a notification. When notified it copies the shared
 * LumiFurSystemState (protected by a mutex) and calls updateHUDOnChange().
 * Because the HUD only redraws on actual changes, the task spends almost
 * all of its time sleeping and never interferes with matrix rendering,
 * sensor reads, BLE, or animation.
 *
 * Usage:
 *   1. Call startLumiFurHudTask() once from setup() after Wire.begin().
 *   2. From any other task or callback, update the shared state via
 *      updateLumiFurHudState() (or the individual setters) and the HUD
 *      will refresh itself if something changed.
 */

#include "LumiFur_HUD.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Shared state – written by the rest of the firmware, read by the HUD task.
extern LumiFurSystemState gHudState;
extern SemaphoreHandle_t gHudStateMutex;

// Start the low-priority HUD task. Safe to call even if the OLED is absent.
void startLumiFurHudTask();

// Thread-safe way to push a complete new state. Notifies the HUD task.
void updateLumiFurHudState(const LumiFurSystemState &newState);

// Convenience helpers (optional – use the full struct if you prefer)
void hudSetFace(uint8_t index, const String &name);
void hudSetOverlay(const String &overlay);
void hudSetLux(uint16_t lux);
void hudSetBoop(bool active);
void hudSetMode(SystemOperatingMode mode, const String &detail = "");
void hudSetPairingPasskey(const String &passkey);
void hudSetMatrixBrightness(uint8_t percent);
void hudSetFanSpeed(uint8_t percent);
void hudSetOledBrightness(uint8_t contrast);   // 0x00–0xFF, written by BLE
void hudSetNfcPower(bool on);
void hudSetBleConnected(bool connected);

#endif // LUMIFUR_HUD_TASK_H
