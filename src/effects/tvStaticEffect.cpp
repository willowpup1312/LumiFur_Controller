#include "effects/tvStaticEffect.h"

extern uint16_t globalBrightnessScaleFixed;

static DisplayOutputType *g_display = nullptr;
static int g_w = 0;
static int g_h = 0;

// Cheap xorshift32 — good enough for TV snow, no heap, no float.
static uint32_t g_rng = 0xA5A5A5A5u;

static inline uint32_t nextRand()
{
  uint32_t x = g_rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  g_rng = x;
  return x;
}

void initTvStaticEffect(DisplayOutputType *display)
{
  g_display = display;
  if (g_display)
  {
    g_w = g_display->width();
    g_h = g_display->height();
  }
  // Mix in a bit of wall time so cold boots don't always start on the same pattern.
  g_rng ^= static_cast<uint32_t>(millis());
  if (g_rng == 0)
  {
    g_rng = 0xA5A5A5A5u;
  }
}

// Map 0–255 grey through global brightness into RGB565.
static inline uint16_t greyTo565(uint8_t level)
{
  if (!g_display)
  {
    return 0;
  }
  const uint16_t s = globalBrightnessScaleFixed;
  const uint8_t v = static_cast<uint8_t>((static_cast<uint16_t>(level) * s + 128u) >> 8);
  return g_display->color565(v, v, v);
}

void updateAndDrawTvStaticEffect()
{
  if (!g_display || g_w <= 0 || g_h <= 0)
  {
    return;
  }

  // Full-frame snow. Slight bias toward mid/high greys keeps it readable on HUB75.
  for (int y = 0; y < g_h; ++y)
  {
    for (int x = 0; x < g_w; ++x)
    {
      const uint8_t n = static_cast<uint8_t>(nextRand() & 0xFF);
      // Occasional pure black flecks read more like analogue static.
      const uint8_t level = (n < 18) ? 0 : static_cast<uint8_t>(40 + ((n * 215u) >> 8));
      g_display->drawPixel(x, y, greyTo565(level));
    }
  }
}

void drawTvStaticThroughXbm(int x, int y, int width, int height, const uint8_t *xbm)
{
  if (!g_display || !xbm || width <= 0 || height <= 0)
  {
    return;
  }

  const int byteWidth = (width + 7) >> 3;
  for (int j = 0; j < height; ++j)
  {
    const int py = y + j;
    if ((unsigned)py >= (unsigned)g_h)
    {
      continue;
    }
    for (int i = 0; i < width; ++i)
    {
      const int byteIndex = j * byteWidth + (i >> 3);
      const uint8_t bit = 1u << (i & 7);
      if (!(xbm[byteIndex] & bit))
      {
        continue;
      }
      const int px = x + i;
      if ((unsigned)px >= (unsigned)g_w)
      {
        continue;
      }
      const uint8_t n = static_cast<uint8_t>(nextRand() & 0xFF);
      const uint8_t level = (n < 18) ? 0 : static_cast<uint8_t>(40 + ((n * 215u) >> 8));
      g_display->drawPixel(px, py, greyTo565(level));
    }
  }
}
