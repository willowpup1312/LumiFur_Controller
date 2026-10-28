/**
 * @file LumiFur_HUD_Task.cpp
 * @brief Implementation of the low-priority internal OLED HUD task.
 *
 * Priority 1 (lowest practical). Pinned to core 1 so it stays off the core
 * that normally runs the HUB75 DMA / display task. Stack is 4 KB – more than
 * enough for the short strings used by the HUD.
 *
 * The task never polls. It sleeps until another part of the firmware calls
 * updateLumiFurHudState() (or one of the convenience setters), which takes
 * the mutex, updates the shared struct, and gives a task notification.
 */

#include "LumiFur_HUD_Task.h"
#include <Arduino.h>

// Shared state (defined here, declared in the header)
LumiFurSystemState gHudState;
SemaphoreHandle_t gHudStateMutex = nullptr;

static TaskHandle_t sHudTaskHandle = nullptr;
static LumiFurHUD sHud;

// ---------------------------------------------------------------------------
// HUD task – blocks forever until notified
// ---------------------------------------------------------------------------
static void hudTask(void *parameter) {
  (void)parameter;

  // Try to bring the OLED up. If it fails we just exit; the rest of the
  // firmware continues unaffected.
  if (!sHud.begin(0x01)) {
#if DEBUG_MODE
    Serial.println("LumiFur HUD: OLED not found, task exiting");
#endif
    vTaskDelete(nullptr);
    return;
  }

#if DEBUG_MODE
  Serial.println("LumiFur HUD: task started (priority 1)");
#endif

  for (;;) {
    // Sleep until someone notifies us of a state change
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Copy the shared state under the mutex (very short critical section)
    LumiFurSystemState snapshot;
    if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      snapshot = gHudState;
      xSemaphoreGive(gHudStateMutex);
    } else {
      continue; // mutex contention – try again next notification
    }

    // The HUD itself decides whether a redraw is required
    sHud.updateHUDOnChange(snapshot);
  }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void startLumiFurHudTask() {
  if (sHudTaskHandle != nullptr) {
    return; // already running
  }

  gHudStateMutex = xSemaphoreCreateMutex();
  if (gHudStateMutex == nullptr) {
    return;
  }

  // Priority 1, 4 KB stack, pinned to core 1
  BaseType_t created = xTaskCreatePinnedToCore(
      hudTask,
      "LumiFurHUD",
      4096,
      nullptr,
      1,                 // lowest practical priority
      &sHudTaskHandle,
      1                  // core 1
  );

  if (created != pdPASS) {
    sHudTaskHandle = nullptr;
#if DEBUG_MODE
    Serial.println("LumiFur HUD: failed to create task");
#endif
  }
}

void updateLumiFurHudState(const LumiFurSystemState &newState) {
  if (gHudStateMutex == nullptr || sHudTaskHandle == nullptr) {
    return;
  }

  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    gHudState = newState;
    xSemaphoreGive(gHudStateMutex);
    // Wake the HUD task (it will re-check equality and skip if unchanged)
    xTaskNotifyGive(sHudTaskHandle);
  }
}

// ---------------------------------------------------------------------------
// Convenience setters – each updates one field and notifies
// ---------------------------------------------------------------------------
static void notifyIfChanged() {
  if (sHudTaskHandle != nullptr) {
    xTaskNotifyGive(sHudTaskHandle);
  }
}

void hudSetFace(uint8_t index, const String &name) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.faceIndex = index;
    gHudState.currentFaceName = name;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetOverlay(const String &overlay) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.overlayName = overlay;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetLux(uint16_t lux) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.lux = lux;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetBoop(bool active) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.boopActive = active;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetMode(SystemOperatingMode mode, const String &detail) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.sysMode = mode;
    gHudState.modeDetailText = detail;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetPairingPasskey(const String &passkey) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.pairingPasskey = passkey;
    if (passkey.length() > 0) {
      gHudState.sysMode = MODE_BLE_PAIRING;
    }
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetMatrixBrightness(uint8_t percent) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.matrixBrightness = percent;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetFanSpeed(uint8_t percent) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.fanSpeed = percent;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetOledBrightness(uint8_t contrast) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.oledBrightness = contrast;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetNfcPower(bool on) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.nfcPower = on;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}

void hudSetBleConnected(bool connected) {
  if (gHudStateMutex == nullptr) return;
  if (xSemaphoreTake(gHudStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    gHudState.bleConnected = connected;
    xSemaphoreGive(gHudStateMutex);
    notifyIfChanged();
  }
}
