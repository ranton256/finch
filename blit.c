#include "blit.h"

// colors are 0xAARRGGBB
uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b)
{
        return 0xFF000000 | (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | b;
}

uint32_t MakeColorWithAlpha(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
        return (((uint32_t)a) << 24) | (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | b;
}

void Color2Values(uint32_t color, uint8_t components[4])
{
    // TODO: return (((uint32_t)a) << 24) | (((uint32_t)r) << 16) | (((uint32_t)g) << 8) | b;
    uint32_t tmp = color;
    components[2] = tmp & 0xff; // blue
    tmp >>= 8;
    components[1] = tmp & 0xFF; // green
    tmp >>= 8;
    components[0] = tmp & 0xFF; // red
    tmp >>= 8;
    components[3] = tmp & 0xFF; // alpha
}


void Blit32Bit(Pixel* dst, uint8_t* src, uint32_t width, uint32_t height)
{
  for(int row = 0; row < height; row++) {
        Pixel* dstPixelsPtr = dst + row * width;
        uint8_t* srcPixelsPtr = src + 4 * row * width;
        for(int col = 0; col < width; col++) {
            uint8_t r, g, b, a;
            r = *srcPixelsPtr++;
            g = *srcPixelsPtr++;
            b = *srcPixelsPtr++;
            a = *srcPixelsPtr++;
            // r = 55; g = 0; b = 255;
            // a = col < 24 ? 255 : 30;
            *dstPixelsPtr++ = MakeColorWithAlpha(r, g, b, a);
        }
    }
}

void Blit24To32Bit(Pixel* dst, uint8_t* src, uint32_t width, uint32_t height)
{
    for(int row = 0; row < height; row++) {
        Pixel* dstPixelsPtr = dst + row * width;
        uint8_t* srcPixelsPtr = src + 3 * row * width;
        for(int col = 0; col < width; col++) {
            uint8_t r, g, b;
            r = *srcPixelsPtr++;
            g = *srcPixelsPtr++;
            b = *srcPixelsPtr++;
            *dstPixelsPtr++ = MakeColor(r, g, b);
        }
    }
}

