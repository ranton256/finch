#ifndef __BLIT__
#define __BLIT__

#include "finch.h"

// These are for blitting pixel data to SDL buffers.

uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b);
uint32_t MakeColorWithAlpha(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void Blit32Bit(Pixel* dst, uint8_t* src, uint32_t width, uint32_t height);
void Blit24To32Bit(Pixel* dst, uint8_t* src, uint32_t width, uint32_t height);

#endif
