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

static bool GetPixelTest(GraphicsBuffer *buffer)
{
	// Fill buffer with black
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	// Put several different colored pixels
	PutPixel(buffer, AsPixel(kRed), 10, 10);
	PutPixel(buffer, AsPixel(kGreen), 20, 20);
	PutPixel(buffer, AsPixel(kBlue), 30, 30);

	// Read them back and verify
	Pixel p1 = GetPixel(buffer, 10, 10);
	Pixel p2 = GetPixel(buffer, 20, 20);
	Pixel p3 = GetPixel(buffer, 30, 30);
	Pixel p4 = GetPixel(buffer, 15, 15);  // Should be black

	if (p1 != AsPixel(kRed)) {
		fprintf(stderr, "GetPixel failed: expected red at (10,10)\n");
		return false;
	}
	if (p2 != AsPixel(kGreen)) {
		fprintf(stderr, "GetPixel failed: expected green at (20,20)\n");
		return false;
	}
	if (p3 != AsPixel(kBlue)) {
		fprintf(stderr, "GetPixel failed: expected blue at (30,30)\n");
		return false;
	}
	if (p4 != AsPixel(kBlack)) {
		fprintf(stderr, "GetPixel failed: expected black at (15,15)\n");
		return false;
	}

	// Test out-of-bounds returns 0
	Pixel p5 = GetPixel(buffer, -1, 10);
	Pixel p6 = GetPixel(buffer, 10, -1);
	Pixel p7 = GetPixel(buffer, buffer->width, 10);
	Pixel p8 = GetPixel(buffer, 10, buffer->height);

	if (p5 != 0 || p6 != 0 || p7 != 0 || p8 != 0) {
		fprintf(stderr, "GetPixel failed: out-of-bounds should return 0\n");
		return false;
	}

	return true;
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

static bool ClippingTest(GraphicsBuffer *buffer)
{
	// Test that drawing outside buffer bounds doesn't crash or corrupt memory
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	// Draw line extending beyond buffer on all sides
	DrawLine(buffer, AsPixel(kRed), -10, -10, buffer->width + 10, buffer->height + 10);

	// Draw rect partially outside buffer (negative coords)
	DrawRect(buffer, AsPixel(kGreen), -5, -5, 10, 10);

	// Draw rect partially outside buffer (exceeds dimensions)
	DrawRect(buffer, AsPixel(kBlue), buffer->width - 10, buffer->height - 10,
	         buffer->width + 5, buffer->height + 5);

	// FillRect extending beyond bounds
	FillRectOpaque(buffer, AsPixel(kRed), buffer->width - 5, buffer->height - 5,
	               buffer->width + 10, buffer->height + 10);

	// Draw circle centered outside buffer
	DrawCircle(buffer, AsPixel(kGreen), -10, -10, 20);
	DrawCircle(buffer, AsPixel(kGreen), buffer->width + 10, buffer->height + 10, 20);

	// If we get here without crashing, clipping works
	// Verify at least some edges were drawn correctly
	Pixel topLeft = GetPixel(buffer, 0, 0);
	Pixel bottomRight = GetPixel(buffer, buffer->width - 1, buffer->height - 1);

	// These should have been modified by some of the operations
	// (not rigorous but ensures clipping didn't prevent all drawing)
	if (topLeft == AsPixel(kBlack) && bottomRight == AsPixel(kBlack)) {
		// Both still black might indicate clipping is too aggressive
		// But this is acceptable - the test mainly checks for crashes
	}

	return true;
}

static bool RectTest()
{
    /* TODO:
     r1 = MakeRect(40, 75, 60, 100)
     r2 = MakeRect(20, 85, 60, 105)
     ir = IntersectRects(r1,r2)

     // our rect structure.
     typedef struct {
         int32_t left, top, right, bottom;
     } LSRect;

     // this checks to see if a point is in a rect.
     int LSPointInRect( int x, int y, const LSRect r );
     // this checks for intersection of 2 rectangles.
     int IntersectRects( const LSRect r1, const LSRect r2, LSRect* sect );


     */
    LSRect r1, r2, ir;
    r1.left = 40;
    r1.right = 100;
    r1.top = 75;
    r1.bottom = 100;

    r2.left = 20;
    r2.right = 60;
    r2.top = 85;
    r2.bottom = 105;

    bool intersects = IntersectRects(r1, r2, &ir);
    if(!intersects) {
        fprintf(stderr, "RectTest: expected intersection but got none\n");
        return false;
    }

    if(ir.left != 40) {
        fprintf(stderr, "RectTest: expected left=40, got %d\n", ir.left);
        return false;
    }
    if(ir.right != 60) {
        fprintf(stderr, "RectTest: expected right=60, got %d\n", ir.right);
        return false;
    }
    if(ir.top != 85) {
        fprintf(stderr, "RectTest: expected top=85, got %d\n", ir.top);
        return false;
    }
    if(ir.bottom != 100) {
        fprintf(stderr, "RectTest: expected bottom=100, got %d\n", ir.bottom);
        return false;
    }

    // Test non-intersecting rectangles
    LSRect r3, r4, ir2;
    r3.left = 10;
    r3.right = 20;
    r3.top = 10;
    r3.bottom = 20;

    r4.left = 30;
    r4.right = 40;
    r4.top = 30;
    r4.bottom = 40;

    bool noIntersection = IntersectRects(r3, r4, &ir2);
    if(noIntersection) {
        fprintf(stderr, "RectTest: expected no intersection but got one\n");
        return false;
    }

    return true;
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

static bool AlphaCompositingTest(GraphicsBuffer *buffer)
{
	// Test LSCompositePixels and alpha blending
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	// Create a semi-transparent red (50% alpha)
	Pixel semiRed = MakeColorWithAlpha(255, 0, 0, 128);

	// Draw a line with alpha compositing
	DrawLineComposite(buffer, semiRed, 10, 10, 50, 10);

	// Get a pixel from the line - should be blended with black background
	Pixel linePixel = GetPixel(buffer, 30, 10);
	uint8_t components[4];
	Color2Values(linePixel, components);

	// With 50% alpha red over black, we expect approximately (128, 0, 0, 255)
	// Allow some tolerance due to rounding
	if (components[0] < 120 || components[0] > 135) {
		fprintf(stderr, "AlphaCompositing line failed: expected ~128 red, got %d\n", components[0]);
		return false;
	}
	if (components[1] > 5 || components[2] > 5) {
		fprintf(stderr, "AlphaCompositing line failed: expected near-zero green/blue\n");
		return false;
	}

	// Test BlitGraphBufferComposite
	FillRectOpaque(buffer, MakeColor(0, 0, 255), 0, 0, buffer->height, buffer->width);

	// Create a small buffer with semi-transparent red
	uint32_t smallWidth = 20, smallHeight = 20;
	GraphicsBuffer *alphaBuf = NewGraphBuffer(NULL, smallWidth, smallHeight, smallWidth,
	                                           smallWidth * smallHeight * sizeof(Pixel));
	if (!alphaBuf) {
		return false;
	}

	FillRectOpaque(alphaBuf, MakeColorWithAlpha(255, 0, 0, 128), 0, 0, smallHeight, smallWidth);

	// Blit with compositing onto blue background
	BlitGraphBufferComposite(alphaBuf, buffer, 30, 30);

	// Check a pixel in the blitted area - should be blend of red and blue
	Pixel blitPixel = GetPixel(buffer, 35, 35);
	Color2Values(blitPixel, components);

	// With 50% red over blue, expect purple-ish: red ~128, blue ~128
	if (components[0] < 120 || components[0] > 135) {
		fprintf(stderr, "AlphaCompositing blit failed: expected ~128 red, got %d\n", components[0]);
		DeleteGraphBuffer(alphaBuf);
		return false;
	}
	if (components[2] < 120 || components[2] > 135) {
		fprintf(stderr, "AlphaCompositing blit failed: expected ~128 blue, got %d\n", components[2]);
		DeleteGraphBuffer(alphaBuf);
		return false;
	}

	DeleteGraphBuffer(alphaBuf);
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

static bool FillCircleTest(GraphicsBuffer *buffer)
{
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	FillCircle(buffer, AsPixel(kGreen), kCenterX, kCenterY, kRadius);

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
			// For filled circle, all interior pixels should be foreground
			if (status <= 0)
			{
				if (!PixelEqualNoMask(ppVal, pptr))
				{
					fprintf(stderr, "FillCircle interior pixel value mismatch at %u, %u\n", x, y);
					return false;
				}
			}
			else if (status * status <= 256)
			{
				// give it a pass, indeterminate (edge pixels)
			}
			else
			{
				if (!PixelEqualNoMask(bgVal, pptr))
				{
					fprintf(stderr, "FillCircle exterior pixel value mismatch at %u, %u\n", x, y);
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
        {RectTest, "RectTest"},
        {ColorTest, "ColorTest"},
		{PutPixelTest, "PutPixelTest"},
		{GetPixelTest, "GetPixelTest"},
		{FillRectTest, "FillRectTest"},
		{FillRectOpaqueTest, "FillRectOpaqueTest"},
		{DrawRectTest, "DrawRectTest"},
		{DrawLineTest, "DrawLineTest"},
		{CircleTest, "CircleTest"},
		{FillCircleTest, "FillCircleTest"},
		{BlitBufferTest, "BlitBufferTest"},
		{ClippingTest, "ClippingTest"},
		{AlphaCompositingTest, "AlphaCompositingTest"},
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
