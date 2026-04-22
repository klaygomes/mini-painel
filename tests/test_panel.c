#include "vendor/unity.h"
#include "fake_serial.h"
#include "../src/panel.h"

#include <stdlib.h>
#include <string.h>

/*
 * Expected bytes the XuanFang Rev B device receives, per its specification.
 * These are device-protocol constants, not internal library constants.
 */
#define DEVICE_CMD_SET_ORIENTATION  0xCB
#define DEVICE_CMD_DISPLAY_BITMAP   0xCC
#define DEVICE_CMD_SET_LIGHTING     0xCD
#define DEVICE_CMD_SET_BRIGHTNESS   0xCE

#define DEVICE_ORIENT_PORTRAIT      0x00
#define DEVICE_ORIENT_LANDSCAPE     0x01

/* Sub-revision bytes returned by the device in its HELLO response. */
#define SUB_REV_A01 0x01   /* Rev B, binary brightness               */
#define SUB_REV_A02 0x02   /* Flagship, binary brightness + LED      */
#define SUB_REV_A11 0x11   /* Rev B, 0-255 brightness                */
#define SUB_REV_A12 0x12   /* Flagship, 0-255 brightness + LED       */

/* Build a device that has completed the HELLO handshake.
 * Clears the write buffer after open so tests start with an empty slate. */
static xf_device_t *open_as(uint8_t sub_byte)
{
    uint8_t resp[10] = {0xCA,'H','E','L','L','O',0x0A,sub_byte,0x00,0xCA};
    fake_serial_set_response(resp, 10);
    xf_device_t *dev = panel_open("fake");
    fake_serial_clear_writes();
    return dev;
}

void setUp(void)    { fake_serial_reset(); }
void tearDown(void) {}

/* ─────────────────────────────────────────────────────────────────────────── */
/* Capability detection                                                         */
/* ─────────────────────────────────────────────────────────────────────────── */

static void test_a01_is_not_flagship_and_not_range(void)
{
    xf_device_t *dev = open_as(SUB_REV_A01);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_FALSE(panel_is_flagship(dev));
    TEST_ASSERT_FALSE(panel_is_brightness_range(dev));
    panel_close(dev);
}

static void test_a02_is_flagship_and_not_range(void)
{
    xf_device_t *dev = open_as(SUB_REV_A02);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_TRUE(panel_is_flagship(dev));
    TEST_ASSERT_FALSE(panel_is_brightness_range(dev));
    panel_close(dev);
}

static void test_a11_is_not_flagship_and_is_range(void)
{
    xf_device_t *dev = open_as(SUB_REV_A11);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_FALSE(panel_is_flagship(dev));
    TEST_ASSERT_TRUE(panel_is_brightness_range(dev));
    panel_close(dev);
}

static void test_a12_is_flagship_and_is_range(void)
{
    xf_device_t *dev = open_as(SUB_REV_A12);
    TEST_ASSERT_NOT_NULL(dev);
    TEST_ASSERT_TRUE(panel_is_flagship(dev));
    TEST_ASSERT_TRUE(panel_is_brightness_range(dev));
    panel_close(dev);
}

static void test_open_returns_null_on_bad_hello(void)
{
    uint8_t garbage[10] = {0xAA, 0, 0, 0, 0, 0, 0, 0, 0, 0xAA};
    fake_serial_set_response(garbage, 10);
    TEST_ASSERT_NULL(panel_open("fake"));
}

/* ─────────────────────────────────────────────────────────────────────────── */
/* Brightness                                                                   */
/* ─────────────────────────────────────────────────────────────────────────── */

static void test_range_brightness_off_sends_zero(void)
{
    xf_device_t *dev = open_as(SUB_REV_A12);
    panel_set_brightness(dev, 0);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_BRIGHTNESS, b[0]);
    TEST_ASSERT_EQUAL_UINT8(0, b[1]);
    panel_close(dev);
}

static void test_range_brightness_50pct_sends_127(void)
{
    xf_device_t *dev = open_as(SUB_REV_A12);
    panel_set_brightness(dev, 50);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_BRIGHTNESS, b[0]);
    TEST_ASSERT_EQUAL_UINT8(127, b[1]);
    panel_close(dev);
}

static void test_range_brightness_full_sends_255(void)
{
    xf_device_t *dev = open_as(SUB_REV_A12);
    panel_set_brightness(dev, 100);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_BRIGHTNESS, b[0]);
    TEST_ASSERT_EQUAL_UINT8(255, b[1]);
    panel_close(dev);
}

static void test_binary_brightness_on_sends_zero(void)
{
    /* Binary devices use 0 for full brightness, 1 for off. */
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_brightness(dev, 75);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_BRIGHTNESS, b[0]);
    TEST_ASSERT_EQUAL_UINT8(0, b[1]);
    panel_close(dev);
}

static void test_binary_brightness_off_sends_one(void)
{
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_brightness(dev, 0);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_BRIGHTNESS, b[0]);
    TEST_ASSERT_EQUAL_UINT8(1, b[1]);
    panel_close(dev);
}

/* ─────────────────────────────────────────────────────────────────────────── */
/* Backplate LED                                                                */
/* ─────────────────────────────────────────────────────────────────────────── */

static void test_led_flagship_sends_rgb_to_device(void)
{
    xf_device_t *dev = open_as(SUB_REV_A12);
    xf_color_t color = {0x10, 0x20, 0x30};
    panel_set_led(dev, color);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_LIGHTING, b[0]);
    TEST_ASSERT_EQUAL_UINT8(0x10, b[1]);
    TEST_ASSERT_EQUAL_UINT8(0x20, b[2]);
    TEST_ASSERT_EQUAL_UINT8(0x30, b[3]);
    panel_close(dev);
}

static void test_led_non_flagship_sends_nothing(void)
{
    xf_device_t *dev = open_as(SUB_REV_A01);
    xf_color_t color = {0xFF, 0xFF, 0xFF};
    panel_set_led(dev, color);
    TEST_ASSERT_EQUAL_size_t(0, fake_serial_written_len());
    panel_close(dev);
}

/* ─────────────────────────────────────────────────────────────────────────── */
/* Orientation                                                                  */
/* ─────────────────────────────────────────────────────────────────────────── */

static void test_portrait_sends_hw_portrait(void)
{
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_orientation(dev, XF_ORIENT_PORTRAIT);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_ORIENTATION, b[0]);
    TEST_ASSERT_EQUAL_UINT8(DEVICE_ORIENT_PORTRAIT, b[1]);
    panel_close(dev);
}

static void test_landscape_sends_hw_landscape(void)
{
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_orientation(dev, XF_ORIENT_LANDSCAPE);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_ORIENTATION, b[0]);
    TEST_ASSERT_EQUAL_UINT8(DEVICE_ORIENT_LANDSCAPE, b[1]);
    panel_close(dev);
}

static void test_reverse_portrait_sends_hw_portrait(void)
{
    /* Reverse portrait uses hardware portrait + software pixel rotation. */
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_orientation(dev, XF_ORIENT_REVERSE_PORTRAIT);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_ORIENTATION, b[0]);
    TEST_ASSERT_EQUAL_UINT8(DEVICE_ORIENT_PORTRAIT, b[1]);
    panel_close(dev);
}

static void test_reverse_landscape_sends_hw_landscape(void)
{
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_orientation(dev, XF_ORIENT_REVERSE_LANDSCAPE);
    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_SET_ORIENTATION, b[0]);
    TEST_ASSERT_EQUAL_UINT8(DEVICE_ORIENT_LANDSCAPE, b[1]);
    panel_close(dev);
}

/* ─────────────────────────────────────────────────────────────────────────── */
/* Bitmap display                                                               */
/* ─────────────────────────────────────────────────────────────────────────── */

static void test_bitmap_portrait_sends_correct_region(void)
{
    /* panel_display_bitmap at (10,20) with 100×50 pixels in portrait.
     * Device should receive top-left=(10,20) bottom-right=(109,69). */
    xf_device_t *dev = open_as(SUB_REV_A01);
    uint8_t pixels[100 * 50 * 3];
    memset(pixels, 0, sizeof(pixels));
    panel_display_bitmap(dev, 10, 20, 100, 50, pixels);

    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_DISPLAY_BITMAP, b[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, b[1]); TEST_ASSERT_EQUAL_UINT8(0x0A, b[2]); /* x0=10  */
    TEST_ASSERT_EQUAL_UINT8(0x00, b[3]); TEST_ASSERT_EQUAL_UINT8(0x14, b[4]); /* y0=20  */
    TEST_ASSERT_EQUAL_UINT8(0x00, b[5]); TEST_ASSERT_EQUAL_UINT8(0x6D, b[6]); /* x1=109 */
    TEST_ASSERT_EQUAL_UINT8(0x00, b[7]); TEST_ASSERT_EQUAL_UINT8(0x45, b[8]); /* y1=69  */
    panel_close(dev);
}

static void test_bitmap_reverse_portrait_flips_region(void)
{
    /* Same call in reverse portrait: region is mirrored on the 320×480 screen.
     * x0=320-10-100=210, y0=480-20-50=410, x1=309, y1=459 */
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_orientation(dev, XF_ORIENT_REVERSE_PORTRAIT);
    fake_serial_clear_writes();

    uint8_t pixels[100 * 50 * 3];
    memset(pixels, 0, sizeof(pixels));
    panel_display_bitmap(dev, 10, 20, 100, 50, pixels);

    const uint8_t *b = fake_serial_written_bytes();
    TEST_ASSERT_EQUAL_HEX8(DEVICE_CMD_DISPLAY_BITMAP, b[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, b[1]); TEST_ASSERT_EQUAL_UINT8(0xD2, b[2]); /* x0=210 */
    TEST_ASSERT_EQUAL_UINT8(0x01, b[3]); TEST_ASSERT_EQUAL_UINT8(0x9A, b[4]); /* y0=410 */
    TEST_ASSERT_EQUAL_UINT8(0x01, b[5]); TEST_ASSERT_EQUAL_UINT8(0x35, b[6]); /* x1=309 */
    TEST_ASSERT_EQUAL_UINT8(0x01, b[7]); TEST_ASSERT_EQUAL_UINT8(0xCB, b[8]); /* y1=459 */
    panel_close(dev);
}

static void test_bitmap_sends_pixel_in_rgb565_big_endian(void)
{
    /* A red pixel (255,0,0) encodes to RGB565 0xF800 in big-endian: {0xF8, 0x00}. */
    xf_device_t *dev = open_as(SUB_REV_A01);
    uint8_t red[3] = {255, 0, 0};
    panel_display_bitmap(dev, 0, 0, 1, 1, red);

    const uint8_t *b = fake_serial_written_bytes();
    /* Pixel data follows the 10-byte command frame. */
    TEST_ASSERT_EQUAL_HEX8(0xF8, b[10]);
    TEST_ASSERT_EQUAL_HEX8(0x00, b[11]);
    panel_close(dev);
}

static void test_bitmap_reverse_portrait_rotates_pixels(void)
{
    /* Two pixels [red, blue] sent in reverse portrait must arrive as [blue, red]
     * because the library rotates them 180° so the device renders them correctly. */
    xf_device_t *dev = open_as(SUB_REV_A01);
    panel_set_orientation(dev, XF_ORIENT_REVERSE_PORTRAIT);
    fake_serial_clear_writes();

    uint8_t pixels[6] = {255, 0, 0,   0, 0, 255}; /* red, blue */
    panel_display_bitmap(dev, 0, 0, 2, 1, pixels);

    const uint8_t *b = fake_serial_written_bytes();
    /* blue = 0x001F, red = 0xF800 */
    TEST_ASSERT_EQUAL_HEX8(0x00, b[10]); TEST_ASSERT_EQUAL_HEX8(0x1F, b[11]); /* blue */
    TEST_ASSERT_EQUAL_HEX8(0xF8, b[12]); TEST_ASSERT_EQUAL_HEX8(0x00, b[13]); /* red  */
    panel_close(dev);
}

/* ─────────────────────────────────────────────────────────────────────────── */
/* Runner                                                                       */
/* ─────────────────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_a01_is_not_flagship_and_not_range);
    RUN_TEST(test_a02_is_flagship_and_not_range);
    RUN_TEST(test_a11_is_not_flagship_and_is_range);
    RUN_TEST(test_a12_is_flagship_and_is_range);
    RUN_TEST(test_open_returns_null_on_bad_hello);

    RUN_TEST(test_range_brightness_off_sends_zero);
    RUN_TEST(test_range_brightness_50pct_sends_127);
    RUN_TEST(test_range_brightness_full_sends_255);
    RUN_TEST(test_binary_brightness_on_sends_zero);
    RUN_TEST(test_binary_brightness_off_sends_one);

    RUN_TEST(test_led_flagship_sends_rgb_to_device);
    RUN_TEST(test_led_non_flagship_sends_nothing);

    RUN_TEST(test_portrait_sends_hw_portrait);
    RUN_TEST(test_landscape_sends_hw_landscape);
    RUN_TEST(test_reverse_portrait_sends_hw_portrait);
    RUN_TEST(test_reverse_landscape_sends_hw_landscape);

    RUN_TEST(test_bitmap_portrait_sends_correct_region);
    RUN_TEST(test_bitmap_reverse_portrait_flips_region);
    RUN_TEST(test_bitmap_sends_pixel_in_rgb565_big_endian);
    RUN_TEST(test_bitmap_reverse_portrait_rotates_pixels);

    return UNITY_END();
}
