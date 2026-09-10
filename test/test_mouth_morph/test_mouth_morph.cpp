#include <unity.h>

#include <cstdint>
#include <cstring>

#include "core/mouth/MouthMorph.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_openness_quantization_preserves_endpoints(void)
{
  TEST_ASSERT_EQUAL_UINT8(0, mouth::quantizeOpenness(0));
  TEST_ASSERT_EQUAL_UINT8(mouth::kFrameCount - 1, mouth::quantizeOpenness(255));
  TEST_ASSERT_EQUAL_UINT8(0, mouth::opennessForFrame(0));
  TEST_ASSERT_EQUAL_UINT8(255, mouth::opennessForFrame(mouth::kFrameCount - 1));
}

void test_vertical_warp_keeps_lower_jaw_anchored(void)
{
  TEST_ASSERT_EQUAL_INT(-1, mouth::warpedSourceRow(8, 22, 13, 0));
  TEST_ASSERT_EQUAL_INT(0, mouth::warpedSourceRow(9, 22, 13, 0));
  TEST_ASSERT_EQUAL_INT(21, mouth::warpedSourceRow(21, 22, 13, 0));
  TEST_ASSERT_EQUAL_INT(0, mouth::warpedSourceRow(0, 22, 13, 255));
  TEST_ASSERT_EQUAL_INT(21, mouth::warpedSourceRow(21, 22, 13, 255));
}

void test_interpolation_preserves_source_endpoints(void)
{
  constexpr std::uint8_t closed[] = {0x00, 0x00, 0x3c, 0x18};
  constexpr std::uint8_t open[] = {0x18, 0x3c, 0x7e, 0xff};
  std::uint8_t output[sizeof(closed)] = {};

  TEST_ASSERT_TRUE(mouth::buildInterpolatedXbm(
      closed, open, 8, 4, 2, 0, false, output, sizeof(output)));
  TEST_ASSERT_EQUAL_MEMORY(closed, output, sizeof(closed));

  TEST_ASSERT_TRUE(mouth::buildInterpolatedXbm(
      closed, open, 8, 4, 2, 255, false, output, sizeof(output)));
  TEST_ASSERT_EQUAL_MEMORY(open, output, sizeof(open));
}

void test_mirrored_halves_use_mirrored_dither(void)
{
  constexpr std::uint8_t closed[] = {0x00};
  constexpr std::uint8_t open[] = {0xff};
  std::uint8_t right[1] = {};
  std::uint8_t left[1] = {};

  TEST_ASSERT_TRUE(mouth::buildInterpolatedXbm(
      closed, open, 8, 1, 1, 128, false, right, sizeof(right)));
  TEST_ASSERT_TRUE(mouth::buildInterpolatedXbm(
      closed, open, 8, 1, 1, 128, true, left, sizeof(left)));
  TEST_ASSERT_EQUAL_HEX8(0xaa, right[0]);
  TEST_ASSERT_EQUAL_HEX8(0x55, left[0]);
}

void test_interpolation_rejects_undersized_output(void)
{
  constexpr std::uint8_t closed[] = {0x00, 0x00};
  constexpr std::uint8_t open[] = {0xff, 0xff};
  std::uint8_t output[1] = {};

  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(
      closed, open, 8, 2, 1, 128, false, output, sizeof(output)));
}


void test_frame_quantization_is_monotonic_and_round_trips(void)
{
  for (unsigned frame = 0; frame < mouth::kFrameCount; ++frame)
  {
    TEST_ASSERT_EQUAL_UINT8(frame, mouth::quantizeOpenness(mouth::opennessForFrame(frame)));
  }
  for (unsigned value = 1; value <= 255; ++value)
  {
    TEST_ASSERT_TRUE(mouth::quantizeOpenness(value) >= mouth::quantizeOpenness(value - 1));
  }
  TEST_ASSERT_EQUAL_UINT8(255, mouth::opennessForFrame(mouth::kFrameCount));
  TEST_ASSERT_EQUAL_UINT8(255, mouth::opennessForFrame(255));
}

void test_warp_rejects_invalid_geometry(void)
{
  TEST_ASSERT_EQUAL_INT(-1, mouth::warpedSourceRow(0, 0, 1, 128));
  TEST_ASSERT_EQUAL_INT(-1, mouth::warpedSourceRow(0, 4, 0, 128));
  TEST_ASSERT_EQUAL_INT(-1, mouth::warpedSourceRow(0, 4, 5, 128));
  TEST_ASSERT_EQUAL_INT(-1, mouth::warpedSourceRow(-1, 4, 2, 128));
  TEST_ASSERT_EQUAL_INT(-1, mouth::warpedSourceRow(4, 4, 2, 128));
  TEST_ASSERT_EQUAL_INT(3, mouth::warpedSourceRow(3, 4, 1, 0));
}

void test_interpolation_rejects_invalid_inputs_without_writing(void)
{
  const std::uint8_t source[] = {0xFF};
  std::uint8_t output[] = {0xA5};
  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(nullptr, source, 8, 1, 1, 128, false, output, 1));
  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(source, nullptr, 8, 1, 1, 128, false, output, 1));
  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(source, source, 8, 1, 1, 128, false, nullptr, 1));
  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(source, source, 0, 1, 1, 128, false, output, 1));
  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(source, source, 8, 0, 1, 128, false, output, 1));
  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(source, source, 8, 1, 0, 128, false, output, 1));
  TEST_ASSERT_FALSE(mouth::buildInterpolatedXbm(source, source, 8, 1, 2, 128, false, output, 1));
  TEST_ASSERT_EQUAL_HEX8(0xA5, output[0]);
}

void test_interpolation_clears_blank_pixels_and_preserves_output_guard(void)
{
  const std::uint8_t source[] = {0x80, 0xFF};
  std::uint8_t output[] = {0xFF, 0xFF, 0xA5};
  // Nine pixels require two bytes. Identical endpoints preserve lit pixels;
  // padding bits must stay clear and the extra output byte must be untouched.
  TEST_ASSERT_TRUE(mouth::buildInterpolatedXbm(source, source, 9, 1, 1, 128, false, output, sizeof(output)));
  const std::uint8_t expected[] = {0x80, 0x80, 0xA5};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, output, sizeof(expected));
}


void test_partial_openness_leaves_rows_above_jaw_blank(void)
{
  const std::uint8_t closed[] = {0, 0, 0, 0};
  const std::uint8_t open[] = {0xFF, 0xFF, 0xFF, 0xFF};
  std::uint8_t output[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xA5};
  // Half openness expands a one-row jaw to three bottom-aligned rows.
  // At 128 coverage, the Bayer pattern alternates lit pixels in each row.
  TEST_ASSERT_TRUE(mouth::buildInterpolatedXbm(
      closed, open, 8, 4, 1, 128, false, output, sizeof(output)));
  const std::uint8_t expected[] = {0x00, 0x55, 0xAA, 0x55, 0xA5};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, output, sizeof(expected));
}

void setup()
{
  UNITY_BEGIN();
  RUN_TEST(test_openness_quantization_preserves_endpoints);
  RUN_TEST(test_vertical_warp_keeps_lower_jaw_anchored);
  RUN_TEST(test_interpolation_preserves_source_endpoints);
  RUN_TEST(test_mirrored_halves_use_mirrored_dither);
  RUN_TEST(test_interpolation_rejects_undersized_output);
  RUN_TEST(test_frame_quantization_is_monotonic_and_round_trips);
  RUN_TEST(test_warp_rejects_invalid_geometry);
  RUN_TEST(test_interpolation_rejects_invalid_inputs_without_writing);
  RUN_TEST(test_interpolation_clears_blank_pixels_and_preserves_output_guard);
  RUN_TEST(test_partial_openness_leaves_rows_above_jaw_blank);
  UNITY_END();
}

int main()
{
  setup();
  return 0;
}

void loop()
{
}
