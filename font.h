#ifndef __FINCH_FONT__
#define __FINCH_FONT__

#include <stdint.h>

/**
 * Built-in 8x8 bitmap font for text rendering.
 * Covers printable ASCII characters 32-126.
 * Each character is 8 bytes, one byte per row.
 */

#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 8
#define FONT_FIRST_CHAR 32
#define FONT_LAST_CHAR 126
#define FONT_NUM_CHARS (FONT_LAST_CHAR - FONT_FIRST_CHAR + 1)

/**
 * Font bitmap data.
 * Each character is 8 bytes (8x8 pixels).
 * Bit set (1) = draw pixel, bit clear (0) = skip pixel.
 * Most significant bit is leftmost pixel.
 */
extern const uint8_t font8x8_basic[FONT_NUM_CHARS][FONT_CHAR_HEIGHT];

#endif // __FINCH_FONT__
