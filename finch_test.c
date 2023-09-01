#include "finch.h"
#include "blit.h"

#include <stdio.h>
#include <stdbool.h>

typedef bool (*ForegroundPredicate)(uint8_t *pptr, uint32_t x, uint32_t y);

const uint32_t kPixelX = 15, kPixelY = 10;
const uint32_t kLeft = 10, kRight = 50, kTop = 15, kBottom = 45;
const uint32_t kLineStart = 10, kLineStop = 50;
const int32_t kCenterX = 40, kCenterY = 30, kRadius = 16;

const RGBColor24 kWhite = {255, 255, 255};
const RGBColor24 kBlack = {0, 0, 0};
const RGBColor24 kRed = {255, 0, 0};
const RGBColor24 kGreen = {0, 255, 0};
const RGBColor24 kBlue = {0, 0, 255};

const uint32_t kDrawTestWidth = 80;
const uint32_t kDrawTestHeight = 60;

static bool PixelEqualNoMask(uint8_t *expected, uint8_t *actual)
{
	return (
		expected[0] == actual[0] &&
		expected[1] == actual[1] &&
		expected[2] == actual[2]);
}

// Compares an image for foreground/background match at each pixel
// based on a function that returns true if the pixel should be foreground color
static bool CompareBufferToPredicate(GraphicsBuffer *buffer, ForegroundPredicate predicate, RGBColor24 foreColor, RGBColor24 backColor)
{
	uint8_t ppVal[4] = {foreColor.red, foreColor.green, foreColor.blue, 255};
	uint8_t bgVal[4] = {backColor.red, backColor.green, backColor.blue, 255};

	for (uint32_t y = 0; y < buffer->height; y++)
	{
		for (uint32_t x = 0; x < buffer->width; x++)
		{

			Pixel* pixelPtr = buffer->ptr + y * buffer->rowPixels + x;
			uint8_t *pptr = (uint8_t*) pixelPtr;
			
			if (predicate(pptr, x, y))
			{
				if (!PixelEqualNoMask(ppVal, pptr))
				{
					fprintf(stderr, "Foreground pixel value mismatch at %u, %u\n", x, y);
					return false;
				}
			}
			else
			{
				if (!PixelEqualNoMask(bgVal, pptr))
				{
					fprintf(stderr, "Background pixel value mismatch at %u, %u\n", x, y);
					return false;
				}
			}
		}
	}
	return true;
}

static bool FillRectOpaquePredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return false; // all background.
}

static bool FillRectOpaqueTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);
	return CompareBufferToPredicate(buffer, FillRectOpaquePredicate, kWhite, kBlack);
}

static bool PutPixelPredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return (x == kPixelX && y == kPixelY);
}

static bool PutPixelTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	PutPixel(buffer, AsPixel(kBlue), kPixelX, kPixelY);

	return CompareBufferToPredicate(buffer, PutPixelPredicate, kBlue, kBlack);
}

static bool FillRectPredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return (x >= kLeft && x < kRight && y >= kTop && y < kBottom);
}

static bool FillRectTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	FillRectOpaque(buffer, AsPixel(kRed), kLeft, kTop, kRight, kBottom);

	return CompareBufferToPredicate(buffer, FillRectPredicate, kRed, kBlack);
}

static bool DrawRectPredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return ((x == kLeft || x == kRight - 1) && (y >= kTop && y < kBottom)) ||
		   ((y == kTop || y == kBottom - 1) && (x >= kLeft && x < kRight));
}

static bool DrawRectTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);
	DrawRect(buffer, AsPixel(kRed), kLeft, kTop, kRight, kBottom);
	return CompareBufferToPredicate(buffer, DrawRectPredicate, kRed, kBlack);
}

static bool DrawLinePredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return (x >= kLineStart && y >= kLineStart) &&
		   (x < kLineStop && y < kLineStop) &&
		   (x == y);
}

static bool DrawLineTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);
	// 45 degree line so predicate is simpler
	DrawLine(buffer, AsPixel(kRed), kLineStart, kLineStart, kLineStop, kLineStop);

	return CompareBufferToPredicate(buffer, DrawLinePredicate, kRed, kBlack);
}

static bool BlitBufferPredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	// just drawing a rect by different means
	return FillRectPredicate(pptr, x, y);
}

static bool BlitBufferTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	uint32_t width = kRight - kLeft;
	uint32_t height = kBottom - kTop;
	GraphicsBuffer *buffer2 = NewGraphBuffer(NULL, width, height, width, width * height * sizeof(Pixel));

	if (!buffer2)
		return false;

	FillRectOpaque(buffer2, AsPixel(kGreen), 0, 0, width, height);

	BlitGraphBuffer(buffer2, buffer, kLeft, kTop);

	bool result = CompareBufferToPredicate(buffer, BlitBufferPredicate, kGreen, kBlack);

	DeleteGraphBuffer(buffer2);
	return result;
}

static bool ColorTest()
{
    // void Color2Values(uint32_t color, uint8_t components[4])
    uint8_t inRed = 255, inGreen = 120, inBlue = 45, inAlpha = 222;
    uint8_t outValues[4];
    
    uint32_t color = MakeColorWithAlpha(inRed,inGreen,inBlue,inAlpha);
    
    outValues[0] = outValues[1] = outValues[2] = outValues[3] = 0;
    Color2Values(color, outValues);
    bool good = inRed == outValues[0] && inGreen  == outValues[1] && inBlue == outValues[2] && inAlpha == outValues[3];
    if(!good) {
        return false;
    }
    
    outValues[0] = outValues[1] = outValues[2] = outValues[3] = 0;
    color = MakeColor(inRed,inGreen,inBlue);
    Color2Values(color, outValues);
    good = inRed == outValues[0] && inGreen  == outValues[1] && inBlue == outValues[2];
    if(!good) {
        return false;
    }
    return true;
}

static int CircleStatus(uint32_t x, uint32_t y)
{
	// Do some trig!!!

	// f_circle(x,y) = x*x + y*y - r*r
	// negative if interior, positive if outside, 0 if on boundary.
	int ix = (int)x - kCenterX;
	int iy = (int)y - kCenterY;
	int ir = (int)kRadius;
	int status = ix * ix + iy * iy - ir * ir;
	return status;
}

static bool CircleTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	DrawCircle(buffer, AsPixel(kGreen), kCenterX, kCenterY, kRadius);

	RGBColor24 foreColor = kGreen;
	RGBColor24 backColor = kBlack;

	uint8_t ppVal[4] = {foreColor.red, foreColor.green, foreColor.blue, 255};
	uint8_t bgVal[4] = {backColor.red, backColor.green, backColor.blue, 255};
	uint32_t rowBytes = buffer->rowPixels * 4;
	uint32_t bytesPerPixel = 4;

	for (uint32_t y = 0; y < buffer->height; y++)
	{
		for (uint32_t x = 0; x < buffer->width; x++)
		{

			uint8_t *pptr = ((uint8_t *)buffer->ptr) + y * rowBytes + x * bytesPerPixel;

			int status = CircleStatus(x, y);
			if (status * status < 9)
			{
				if (!PixelEqualNoMask(ppVal, pptr))
				{
					fprintf(stderr, "Circle foreground pixel value mismatch at %u, %u\n", x, y);
					return false;
				}
			}
			else if (status * status <= 256)
			{
				// give it a pass, indeterminate
			}
			else
			{
				if (!PixelEqualNoMask(bgVal, pptr))
				{
					fprintf(stderr, "Circle background pixel value mismatch at %u, %u\n", x, y);
					return false;
				}
			}
		}
	}
	return true;
}

typedef bool (*FinchUnitTestFunc)(GraphicsBuffer *buffer);

struct FinchUnitTest_t
{
	FinchUnitTestFunc func;
	const char *name;
};
typedef struct FinchUnitTest_t FinchUnitTest;

// tests should have NULL pointer for func of last entry.
// returns 1 if all pass, 0 if any fail.
static int RunTests(FinchUnitTest tests[])
{
	int good = true;

	for (int i = 0; tests[i].func != NULL; i++)
	{

		GraphicsBuffer *buffer = NewGraphBuffer(NULL, kDrawTestWidth, kDrawTestHeight, kDrawTestWidth, kDrawTestWidth * kDrawTestHeight * sizeof(Pixel));
		if (!buffer)
		{
			fprintf(stderr, "Graphics buffer creation failed during test.\n");
			good = false;
			break;
		}
		bool result = tests[i].func(buffer);
		DeleteGraphBuffer(buffer);

		printf("%s %s\n", tests[i].name, result ? "passed" : "FAILED");
		if (!result)
		{
			good = false;
		}
	}

	return good;
}

static bool FinchTests()
{
	bool good = true;

	FinchUnitTest tests[] = {
        {ColorTest, "ColorTest"},
		{PutPixelTest, "PutPixelTest"},
		{FillRectTest, "FillRectTest"},
		{FillRectOpaqueTest, "FillRectOpaqueTest"},
		{DrawRectTest, "DrawRectTest"},
		{DrawLineTest, "DrawLineTest"},
		{CircleTest, "CircleTest"},
		{BlitBufferTest, "BlitBufferTest"},
		{NULL, "marker"}
	};

	int allPassed = RunTests(tests);
	if (allPassed)
	{
		printf("All tests passed.\n");
	}
	else
	{
		printf("There were failed tests.\n");
	}

	return good;
}

int main()
{
	if (!FinchTests())
	{
		fprintf(stderr, "Some tests failed!\n");
		return 1;
	}
	return 0;
}
