#ifndef TV_STATIC_EFFECT_H
#define TV_STATIC_EFFECT_H

#include <Arduino.h>

#ifdef VIRTUAL_PANE
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
typedef VirtualMatrixPanel DisplayOutputType;
#else
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
typedef MatrixPanel_I2S_DMA DisplayOutputType;
#endif

// Initialise once after the panel is ready (same pattern as matrix rain).
void initTvStaticEffect(DisplayOutputType *display);

// Full-panel TV static (VIEW_TV_STATIC). Clears and fills every frame.
void updateAndDrawTvStaticEffect();

// Face-mask path: stamp greyscale noise through an XBM stencil.
// Callers that also drive matrix rain should advance their own sim; this
// helper is stateless apart from the PRNG seed.
void drawTvStaticThroughXbm(int x, int y, int width, int height,
                            const uint8_t *xbm);

#endif
