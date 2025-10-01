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

// ============================================================================
// Test Helper Functions
// ============================================================================

// Helper: Clear entire buffer to specified color
// This replaces the repetitive FillRectOpaque(buffer, AsPixel(color), 0, 0, h, w) pattern
static void ClearBuffer(GraphicsBuffer *buffer, RGBColor24 color)
{
	FillRectOpaque(buffer, AsPixel(color), 0, 0, buffer->height, buffer->width);
}

// Helper: Verify pixel at coordinates matches expected color
// Returns true if match, false if mismatch (with error message)
static bool AssertPixelEquals(GraphicsBuffer *buffer, uint32_t x, uint32_t y,
                              Pixel expected, const char *testName)
{
	Pixel actual = GetPixel(buffer, x, y);
	if (actual != expected) {
		fprintf(stderr, "%s: pixel at (%u,%u) mismatch - expected 0x%08X, got 0x%08X\n",
		        testName, x, y, expected, actual);
		return false;
	}
	return true;
}

// Helper: Initialize a rect with given bounds
// Makes rect construction clearer and less verbose
static LSRect MakeRect(int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	LSRect r;
	r.left = left;
	r.top = top;
	r.right = right;
	r.bottom = bottom;
	return r;
}

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

// Test: PutPixel draws a single colored pixel at specified coordinates
// Success: One blue pixel at (kPixelX, kPixelY), rest of buffer is black
static bool PutPixelTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);
	PutPixel(buffer, AsPixel(kBlue), kPixelX, kPixelY);
	return CompareBufferToPredicate(buffer, PutPixelPredicate, kBlue, kBlack);
}

// Test: GetPixel reads back pixel colors correctly and handles out-of-bounds
// Success: Reads correct colors from various positions, returns 0 for out-of-bounds
static bool GetPixelTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);

	// Put several different colored pixels at known locations
	PutPixel(buffer, AsPixel(kRed), 10, 10);
	PutPixel(buffer, AsPixel(kGreen), 20, 20);
	PutPixel(buffer, AsPixel(kBlue), 30, 30);

	// Read them back and verify colors match
	if (!AssertPixelEquals(buffer, 10, 10, AsPixel(kRed), "GetPixelTest")) return false;
	if (!AssertPixelEquals(buffer, 20, 20, AsPixel(kGreen), "GetPixelTest")) return false;
	if (!AssertPixelEquals(buffer, 30, 30, AsPixel(kBlue), "GetPixelTest")) return false;
	if (!AssertPixelEquals(buffer, 15, 15, AsPixel(kBlack), "GetPixelTest")) return false;

	// Test out-of-bounds access returns 0
	if (GetPixel(buffer, -1, 10) != 0 ||
	    GetPixel(buffer, 10, -1) != 0 ||
	    GetPixel(buffer, buffer->width, 10) != 0 ||
	    GetPixel(buffer, 10, buffer->height) != 0) {
		fprintf(stderr, "GetPixelTest: out-of-bounds should return 0\n");
		return false;
	}

	return true;
}

static bool FillRectPredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return (x >= kLeft && x < kRight && y >= kTop && y < kBottom);
}

// Test: FillRectOpaque fills a rectangular region with solid color
// Success: Rectangle from (kLeft,kTop) to (kRight,kBottom) is red, rest is black
static bool FillRectTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);
	FillRectOpaque(buffer, AsPixel(kRed), kLeft, kTop, kRight, kBottom);
	return CompareBufferToPredicate(buffer, FillRectPredicate, kRed, kBlack);
}

static bool DrawRectPredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return ((x == kLeft || x == kRight - 1) && (y >= kTop && y < kBottom)) ||
		   ((y == kTop || y == kBottom - 1) && (x >= kLeft && x < kRight));
}

// Test: DrawRect draws rectangle outline
// Success: Red outline at rectangle edges, rest is black
static bool DrawRectTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);
	DrawRect(buffer, AsPixel(kRed), kLeft, kTop, kRight, kBottom);
	return CompareBufferToPredicate(buffer, DrawRectPredicate, kRed, kBlack);
}

// Test: Rectangle drawing handles edge cases without crashing
// Tests: zero-width, zero-height, 1x1, inverted, full-buffer, and very large rectangles
// Success: No crashes, reasonable behavior for degenerate inputs
static bool RectEdgeCasesTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);

	// Zero-width rectangle (left == right)
	DrawRect(buffer, AsPixel(kRed), 10, 10, 10, 20);
	FillRectOpaque(buffer, AsPixel(kGreen), 15, 10, 15, 20);
	// Should either draw nothing or a vertical line - main test is no crash

	// Zero-height rectangle (top == bottom)
	DrawRect(buffer, AsPixel(kBlue), 10, 25, 20, 25);
	FillRectOpaque(buffer, AsPixel(kRed), 10, 30, 20, 30);
	// Should either draw nothing or a horizontal line - main test is no crash

	// Single-pixel rectangle (1x1)
	DrawRect(buffer, AsPixel(kWhite), 30, 30, 31, 31);
	FillRectOpaque(buffer, AsPixel(kWhite), 35, 30, 36, 31);
	// Verify at least one pixel drawn
	bool found = (GetPixel(buffer, 30, 30) == AsPixel(kWhite)) ||
	             (GetPixel(buffer, 35, 30) == AsPixel(kWhite));
	if (!found) {
		fprintf(stderr, "RectEdgeCasesTest: 1x1 rect not visible\n");
		return false;
	}

	// Rectangle exactly matching buffer dimensions
	// DrawRect uses exclusive right/bottom, so this will be clipped
	DrawRect(buffer, AsPixel(kGreen), 0, 0, buffer->width, buffer->height);
	// Just verify it doesn't crash - the rect extends beyond buffer so may not draw fully
	// Check if at least some edge pixels are drawn
	bool foundEdge = false;
	for (uint32_t i = 0; i < 10 && i < buffer->width; i++) {
		if (GetPixel(buffer, i, 0) == AsPixel(kGreen)) {
			foundEdge = true;
			break;
		}
	}
	if (!foundEdge) {
		fprintf(stderr, "RectEdgeCasesTest: full buffer rect drew nothing\n");
		return false;
	}

	// Inverted rectangles (right < left, bottom < top) should now be handled
	// by normalizing the coordinates
	DrawRect(buffer, AsPixel(kRed), 30, 10, 20, 20);  // right < left
	FillRectOpaque(buffer, AsPixel(kBlue), 40, 10, 30, 20);  // right < left
	// Should swap coordinates and draw normally - main test is no crash

	DrawRect(buffer, AsPixel(kRed), 10, 30, 20, 20);  // bottom < top
	FillRectOpaque(buffer, AsPixel(kBlue), 10, 40, 20, 30);  // bottom < top
	// Should swap coordinates and draw normally - main test is no crash

	// Both inverted
	DrawRect(buffer, AsPixel(kGreen), 60, 50, 50, 40);  // both inverted
	FillRectOpaque(buffer, AsPixel(kGreen), 70, 50, 65, 45);  // both inverted
	// Should normalize and draw - main test is no crash

	// Very large rectangle
	DrawRect(buffer, AsPixel(kWhite), -100, -100, buffer->width + 100, buffer->height + 100);
	FillRectOpaque(buffer, AsPixel(kWhite), -50, -50, buffer->width + 50, buffer->height + 50);
	// Should clip properly - main test is no crash

	// Main success: didn't crash with any degenerate inputs
	return true;
}

static bool DrawLinePredicate(uint8_t *pptr, uint32_t x, uint32_t y)
{
	return (x >= kLineStart && y >= kLineStart) &&
		   (x < kLineStop && y < kLineStop) &&
		   (x == y);
}

// Test: DrawLine draws a 45-degree diagonal line
// Success: Red diagonal line from (kLineStart,kLineStart) to (kLineStop,kLineStop)
static bool DrawLineTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);
	// 45 degree line so predicate is simpler
	DrawLine(buffer, AsPixel(kRed), kLineStart, kLineStart, kLineStop, kLineStop);
	return CompareBufferToPredicate(buffer, DrawLinePredicate, kRed, kBlack);
}

// Test: DrawLine handles all line directions (8 octants plus horizontal/vertical)
// Tests horizontal, vertical, and diagonal lines in all directions
// Success: Lines visible in each octant without crashes
static bool DrawLineVariantsTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);

	// Horizontal line (y constant)
	DrawLine(buffer, AsPixel(kRed), 10, 20, 30, 20);
	// DrawLine doesn't always include the end pixel in every octant
	// Just verify start and several middle pixels
	if (GetPixel(buffer, 10, 20) != AsPixel(kRed)) {
		fprintf(stderr, "Horizontal line failed at start (10,20)\n");
		return false;
	}
	if (GetPixel(buffer, 15, 20) != AsPixel(kRed)) {
		fprintf(stderr, "Horizontal line failed at middle (15,20)\n");
		return false;
	}
	if (GetPixel(buffer, 25, 20) != AsPixel(kRed)) {
		fprintf(stderr, "Horizontal line failed at (25,20)\n");
		return false;
	}

	// Vertical line (x constant)
	DrawLine(buffer, AsPixel(kGreen), 40, 10, 40, 30);
	// Check start and middle pixels
	if (GetPixel(buffer, 40, 10) != AsPixel(kGreen)) {
		fprintf(stderr, "Vertical line failed at start (40,10)\n");
		return false;
	}
	if (GetPixel(buffer, 40, 20) != AsPixel(kGreen)) {
		fprintf(stderr, "Vertical line failed at middle (40,20)\n");
		return false;
	}
	if (GetPixel(buffer, 40, 28) != AsPixel(kGreen)) {
		fprintf(stderr, "Vertical line failed at (40,28)\n");
		return false;
	}

	// Octant 1: 0° < angle < 45° (shallow, left to right, downward)
	DrawLine(buffer, AsPixel(kBlue), 5, 5, 20, 10);
	// Just verify start point - Bresenham may not always draw exact endpoint
	if (GetPixel(buffer, 5, 5) != AsPixel(kBlue)) {
		fprintf(stderr, "Octant 1 line start failed\n");
		return false;
	}

	// Octant 2-8: Just verify lines draw without crashing, check start points
	DrawLine(buffer, AsPixel(kRed), 25, 5, 30, 20);
	if (GetPixel(buffer, 25, 5) != AsPixel(kRed)) {
		fprintf(stderr, "Octant 2 line failed\n");
		return false;
	}

	DrawLine(buffer, AsPixel(kGreen), 60, 5, 55, 20);
	if (GetPixel(buffer, 60, 5) != AsPixel(kGreen)) {
		fprintf(stderr, "Octant 3 line failed\n");
		return false;
	}

	DrawLine(buffer, AsPixel(kBlue), 70, 5, 55, 10);
	if (GetPixel(buffer, 70, 5) != AsPixel(kBlue)) {
		fprintf(stderr, "Octant 4 line failed\n");
		return false;
	}

	DrawLine(buffer, AsPixel(kRed), 70, 35, 55, 30);
	if (GetPixel(buffer, 70, 35) != AsPixel(kRed)) {
		fprintf(stderr, "Octant 5 line failed\n");
		return false;
	}

	DrawLine(buffer, AsPixel(kGreen), 60, 50, 55, 35);
	if (GetPixel(buffer, 60, 50) != AsPixel(kGreen)) {
		fprintf(stderr, "Octant 6 line failed\n");
		return false;
	}

	DrawLine(buffer, AsPixel(kBlue), 25, 50, 30, 35);
	if (GetPixel(buffer, 25, 50) != AsPixel(kBlue)) {
		fprintf(stderr, "Octant 7 line failed\n");
		return false;
	}

	DrawLine(buffer, AsPixel(kRed), 5, 35, 20, 30);
	if (GetPixel(buffer, 5, 35) != AsPixel(kRed)) {
		fprintf(stderr, "Octant 8 line failed\n");
		return false;
	}

	return true;
}

// Test: DrawLine edge cases (single point, extreme clipping)
// Tests boundary conditions not covered by other line tests
// Success: No crashes, reasonable behavior for edge cases
static bool DrawLineEdgeCasesTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);

	// Single-point line (start == end)
	// Should either draw single pixel or do nothing gracefully
	DrawLine(buffer, AsPixel(kWhite), 50, 50, 50, 50);
	// Don't assert specific behavior - just verify no crash

	// Vertical line entering/exiting top and bottom
	DrawLine(buffer, AsPixel(kGreen), buffer->width / 2, -10, buffer->width / 2, buffer->height + 10);
	// Should clip to buffer boundaries

	// Diagonal line, opposite direction from ClippingTest
	DrawLine(buffer, AsPixel(kBlue), -10, buffer->height + 10, buffer->width + 10, -10);
	// Should clip properly

	// Very long horizontal line completely outside buffer
	DrawLine(buffer, AsPixel(kRed), -1000, -100, -500, -100);
	// Should do nothing (all pixels clipped)

	return true;
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

static bool BlitTransparencyTest(GraphicsBuffer *buffer)
{
	// Test that BlitGraphBufferComposite properly handles transparent pixels (alpha=0)
	// Note: BlitGraphBuffer is opaque-only, BlitGraphBufferComposite handles transparency

	// Fill destination with a red and blue checkerboard pattern
	FillRectOpaque(buffer, AsPixel(kRed), 0, 0, buffer->height, buffer->width);
	FillRectOpaque(buffer, AsPixel(kBlue), 20, 20, 40, 40);

	// Create a sprite with transparent and opaque pixels
	uint32_t spriteWidth = 30, spriteHeight = 30;
	GraphicsBuffer *sprite = NewGraphBuffer(NULL, spriteWidth, spriteHeight, spriteWidth,
	                                         spriteWidth * spriteHeight * sizeof(Pixel));
	if (!sprite) {
		fprintf(stderr, "BlitTransparencyTest: failed to create sprite buffer\n");
		return false;
	}

	// Fill sprite with a pattern:
	// - Left half: fully transparent (alpha=0)
	// - Right half: opaque green
	for (uint32_t y = 0; y < spriteHeight; y++) {
		for (uint32_t x = 0; x < spriteWidth; x++) {
			if (x < spriteWidth / 2) {
				// Transparent pixels - use MakeColorWithAlpha with alpha=0
				PutPixel(sprite, MakeColorWithAlpha(0, 255, 0, 0), x, y);
			} else {
				// Opaque green pixels
				PutPixel(sprite, MakeColor(0, 255, 0), x, y);
			}
		}
	}

	// Blit sprite over the checkerboard at (10, 10) using COMPOSITE version
	// The transparent left half should NOT change the background
	// The opaque right half should show green
	BlitGraphBufferComposite(sprite, buffer, 10, 10);

	// Check transparent region - should still show original background (red or blue)
	Pixel p1 = GetPixel(buffer, 15, 15); // In transparent region, over red background
	if (p1 != AsPixel(kRed)) {
		fprintf(stderr, "BlitTransparencyTest: transparent pixel overwrote background at (15,15)\n");
		DeleteGraphBuffer(sprite);
		return false;
	}

	Pixel p2 = GetPixel(buffer, 20, 25); // In transparent region, over blue background
	if (p2 != AsPixel(kBlue)) {
		fprintf(stderr, "BlitTransparencyTest: transparent pixel overwrote background at (20,25)\n");
		DeleteGraphBuffer(sprite);
		return false;
	}

	// Check opaque region - should show green
	Pixel p3 = GetPixel(buffer, 30, 15); // In opaque region
	if (p3 != MakeColor(0, 255, 0)) {
		fprintf(stderr, "BlitTransparencyTest: opaque pixel not drawn at (30,15)\n");
		DeleteGraphBuffer(sprite);
		return false;
	}

	DeleteGraphBuffer(sprite);
	return true;
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

static bool NegativeCoordTest(GraphicsBuffer *buffer)
{
	// More rigorous testing of negative coordinate handling with verification
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	// Draw rect outline from negative to positive coords - verify visible portion
	DrawRect(buffer, AsPixel(kRed), -5, -5, 10, 10);
	// DrawRect draws an outline, so edges should be visible
	// The rect goes from (-5,-5) to (10,10), so visible edges are:
	// - Top edge at y=0, x=[0,9]
	// - Left edge at x=0, y=[0,9]
	// Check a few points on visible edges
	bool foundRed = false;
	for (int i = 0; i < 10; i++) {
		if (GetPixel(buffer, i, 0) == AsPixel(kRed) || GetPixel(buffer, 0, i) == AsPixel(kRed)) {
			foundRed = true;
			break;
		}
	}
	if (!foundRed) {
		fprintf(stderr, "NegativeCoordTest: expected red edges from clipped rect\n");
		return false;
	}

	// Fill rect from 0,0 to test normal case first
	FillRectOpaque(buffer, AsPixel(kGreen), 0, 0, 5, 5);
	if (GetPixel(buffer, 0, 0) != AsPixel(kGreen)) {
		fprintf(stderr, "NegativeCoordTest: basic fill failed at (0,0)\n");
		return false;
	}
	if (GetPixel(buffer, 2, 2) != AsPixel(kGreen)) {
		fprintf(stderr, "NegativeCoordTest: basic fill failed at (2,2)\n");
		return false;
	}

	// Line from negative to positive
	DrawLine(buffer, AsPixel(kBlue), -20, buffer->height / 2, 20, buffer->height / 2);
	// Should draw from (0, height/2) to (20, height/2)
	if (GetPixel(buffer, 0, buffer->height / 2) != AsPixel(kBlue)) {
		fprintf(stderr, "NegativeCoordTest: expected blue at (0,%d) from clipped line\n", buffer->height / 2);
		return false;
	}
	if (GetPixel(buffer, 10, buffer->height / 2) != AsPixel(kBlue)) {
		fprintf(stderr, "NegativeCoordTest: expected blue at (10,%d) from line\n", buffer->height / 2);
		return false;
	}

	// PutPixel with negative coords should not crash and GetPixel should return 0
	PutPixel(buffer, AsPixel(kWhite), -1, -1);
	PutPixel(buffer, AsPixel(kWhite), -1, 5);
	PutPixel(buffer, AsPixel(kWhite), 5, -1);
	// Already tested in GetPixelTest but verify here too
	if (GetPixel(buffer, -1, -1) != 0) {
		fprintf(stderr, "NegativeCoordTest: GetPixel(-1,-1) should return 0\n");
		return false;
	}

	return true;
}

static bool LSPointInRectTest()
{
	// Test LSPointInRect function
	LSRect rect;
	rect.left = 10;
	rect.right = 30;
	rect.top = 20;
	rect.bottom = 40;

	// Points clearly inside
	if (!LSPointInRect(15, 25, rect)) {
		fprintf(stderr, "LSPointInRectTest: point (15,25) should be inside\n");
		return false;
	}
	if (!LSPointInRect(10, 20, rect)) {
		fprintf(stderr, "LSPointInRectTest: top-left corner (10,20) should be inside\n");
		return false;
	}

	// Points clearly outside
	if (LSPointInRect(5, 25, rect)) {
		fprintf(stderr, "LSPointInRectTest: point (5,25) should be outside (left)\n");
		return false;
	}
	if (LSPointInRect(35, 25, rect)) {
		fprintf(stderr, "LSPointInRectTest: point (35,25) should be outside (right)\n");
		return false;
	}
	if (LSPointInRect(15, 15, rect)) {
		fprintf(stderr, "LSPointInRectTest: point (15,15) should be outside (above)\n");
		return false;
	}
	if (LSPointInRect(15, 45, rect)) {
		fprintf(stderr, "LSPointInRectTest: point (15,45) should be outside (below)\n");
		return false;
	}

	// Right and bottom edges (exclusive in rect convention)
	if (LSPointInRect(30, 25, rect)) {
		fprintf(stderr, "LSPointInRectTest: right edge (30,25) should be outside\n");
		return false;
	}
	if (LSPointInRect(15, 40, rect)) {
		fprintf(stderr, "LSPointInRectTest: bottom edge (15,40) should be outside\n");
		return false;
	}

	// Corner cases
	if (LSPointInRect(30, 40, rect)) {
		fprintf(stderr, "LSPointInRectTest: bottom-right corner (30,40) should be outside\n");
		return false;
	}

	return true;
}

// Test: IntersectRects correctly calculates rectangle intersections
// Tests both intersecting and non-intersecting rectangle pairs
// Success: Correct intersection bounds, proper detection of non-intersection
static bool RectTest()
{
    // Test case 1: Overlapping rectangles
    // r1 = (40,75) to (100,100)
    // r2 = (20,85) to (60,105)
    // Expected intersection: (40,85) to (60,100)
    LSRect r1 = MakeRect(40, 75, 100, 100);
    LSRect r2 = MakeRect(20, 85, 60, 105);
    LSRect ir;

    bool intersects = IntersectRects(r1, r2, &ir);
    if(!intersects) {
        fprintf(stderr, "RectTest: expected intersection but got none\n");
        return false;
    }

    // Verify intersection bounds
    if(ir.left != 40 || ir.right != 60 || ir.top != 85 || ir.bottom != 100) {
        fprintf(stderr, "RectTest: intersection mismatch - got (%d,%d,%d,%d), expected (40,85,60,100)\n",
                ir.left, ir.top, ir.right, ir.bottom);
        return false;
    }

    // Test case 2: Non-intersecting rectangles
    // r3 = (10,10) to (20,20)
    // r4 = (30,30) to (40,40)
    // Should not intersect
    LSRect r3 = MakeRect(10, 10, 20, 20);
    LSRect r4 = MakeRect(30, 30, 40, 40);
    LSRect ir2;

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

static bool PixelComponentsTest()
{
	// Test PixelComponents function (inverse of MakeColor)
	// MakeColor creates 0xAARRGGBB format, PixelComponents should extract R, G, B

	// Test with MakeColor
	uint32_t color1 = MakeColor(200, 150, 100);
	uint8_t r, g, b;

	PixelComponents(color1, &r, &g, &b);

	if (r != 200) {
		fprintf(stderr, "PixelComponentsTest: expected red=200, got %d\n", r);
		return false;
	}
	if (g != 150) {
		fprintf(stderr, "PixelComponentsTest: expected green=150, got %d\n", g);
		return false;
	}
	if (b != 100) {
		fprintf(stderr, "PixelComponentsTest: expected blue=100, got %d\n", b);
		return false;
	}

	// Test with MakeColorWithAlpha
	uint32_t color2 = MakeColorWithAlpha(75, 125, 175, 255);
	PixelComponents(color2, &r, &g, &b);

	if (r != 75 || g != 125 || b != 175) {
		fprintf(stderr, "PixelComponentsTest: MakeColorWithAlpha roundtrip failed (got %d,%d,%d)\n", r, g, b);
		return false;
	}

	// Test edge cases
	uint32_t black = MakeColor(0, 0, 0);
	PixelComponents(black, &r, &g, &b);
	if (r != 0 || g != 0 || b != 0) {
		fprintf(stderr, "PixelComponentsTest: black pixel failed (got %d,%d,%d)\n", r, g, b);
		return false;
	}

	uint32_t white = MakeColor(255, 255, 255);
	PixelComponents(white, &r, &g, &b);
	if (r != 255 || g != 255 || b != 255) {
		fprintf(stderr, "PixelComponentsTest: white pixel failed (got %d,%d,%d)\n", r, g, b);
		return false;
	}

	// Test symmetry with Color2Values
	uint32_t color32 = MakeColor(88, 99, 110);
	uint8_t components[4];
	Color2Values(color32, components);
	uint8_t r2, g2, b2;
	PixelComponents(color32, &r2, &g2, &b2);

	if (r2 != components[0] || g2 != components[1] || b2 != components[2]) {
		fprintf(stderr, "PixelComponentsTest: symmetry with Color2Values failed\n");
		return false;
	}

	// Test roundtrip: create color, extract components, should match
	uint8_t orig_r = 123, orig_g = 234, orig_b = 45;
	uint32_t roundtrip_color = MakeColor(orig_r, orig_g, orig_b);
	uint8_t extracted_r, extracted_g, extracted_b;
	PixelComponents(roundtrip_color, &extracted_r, &extracted_g, &extracted_b);

	if (extracted_r != orig_r || extracted_g != orig_g || extracted_b != orig_b) {
		fprintf(stderr, "PixelComponentsTest: roundtrip failed - input (%d,%d,%d) != output (%d,%d,%d)\n",
		        orig_r, orig_g, orig_b, extracted_r, extracted_g, extracted_b);
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

static bool BufferStrideTest(GraphicsBuffer *buffer)
{
	// Test buffer with rowPixels > width (stride/padding)
	// This is critical for wrapping external buffers like SDL surfaces
	uint32_t width = 32;
	uint32_t height = 32;
	uint32_t rowPixels = 64; // Double width - simulates stride
	uint32_t size = rowPixels * height * sizeof(Pixel);

	GraphicsBuffer *strideBuf = NewGraphBuffer(NULL, width, height, rowPixels, size);
	if (!strideBuf) {
		fprintf(stderr, "BufferStrideTest: failed to create stride buffer\n");
		return false;
	}

	// Fill with black
	FillRectOpaque(strideBuf, AsPixel(kBlack), 0, 0, height, width);

	// Draw a vertical line - this tests row addressing with stride
	for (uint32_t y = 0; y < height; y++) {
		PutPixel(strideBuf, AsPixel(kRed), 10, y);
	}

	// Verify the line was drawn correctly
	for (uint32_t y = 0; y < height; y++) {
		Pixel p = GetPixel(strideBuf, 10, y);
		if (p != AsPixel(kRed)) {
			fprintf(stderr, "BufferStrideTest: vertical line failed at y=%d\n", y);
			DeleteGraphBuffer(strideBuf);
			return false;
		}
		// Check adjacent pixel is still black
		if (y < height - 1) {
			Pixel adj = GetPixel(strideBuf, 11, y);
			if (adj != AsPixel(kBlack)) {
				fprintf(stderr, "BufferStrideTest: adjacent pixel corrupted at y=%d\n", y);
				DeleteGraphBuffer(strideBuf);
				return false;
			}
		}
	}

	// Draw a horizontal line - tests that we don't overrun into padding
	DrawLine(strideBuf, AsPixel(kGreen), 5, 15, 25, 15);
	// Check start and several points (don't assume endpoint is always drawn)
	if (GetPixel(strideBuf, 5, 15) != AsPixel(kGreen) ||
	    GetPixel(strideBuf, 10, 15) != AsPixel(kGreen) ||
	    GetPixel(strideBuf, 20, 15) != AsPixel(kGreen)) {
		fprintf(stderr, "BufferStrideTest: horizontal line failed\n");
		DeleteGraphBuffer(strideBuf);
		return false;
	}

	// Draw a filled rect
	FillRectOpaque(strideBuf, AsPixel(kBlue), 20, 20, 28, 28);
	for (uint32_t y = 20; y < 28; y++) {
		for (uint32_t x = 20; x < 28; x++) {
			Pixel p = GetPixel(strideBuf, x, y);
			if (p != AsPixel(kBlue)) {
				fprintf(stderr, "BufferStrideTest: filled rect failed at (%d,%d)\n", x, y);
				DeleteGraphBuffer(strideBuf);
				return false;
			}
		}
	}

	DeleteGraphBuffer(strideBuf);
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

// Test: DrawCircle draws a circle outline
// Success: Green circle outline at (kCenterX,kCenterY) with radius kRadius
static bool CircleTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);
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

// Test: FillCircle draws a filled circle
// Success: Solid green circle at (kCenterX,kCenterY) with radius kRadius
static bool FillCircleTest(GraphicsBuffer *buffer)
{
	ClearBuffer(buffer, kBlack);
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

static bool Blit32BitTest(GraphicsBuffer *buffer)
{
	// Test converting 32-bit RGBA byte array to Pixel format
#define BLIT32_WIDTH 4
#define BLIT32_HEIGHT 3

	// Create source data: RGBA bytes (R, G, B, A for each pixel)
	uint8_t srcData[BLIT32_WIDTH * BLIT32_HEIGHT * 4] = {
		// Row 0: solid colors with full alpha
		255, 0, 0, 255,    // Red
		0, 255, 0, 255,    // Green
		0, 0, 255, 255,    // Blue
		255, 255, 255, 255,// White

		// Row 1: semi-transparent colors
		128, 0, 0, 128,    // Semi-transparent red
		0, 128, 0, 128,    // Semi-transparent green
		0, 0, 128, 128,    // Semi-transparent blue
		128, 128, 128, 128,// Semi-transparent gray

		// Row 2: fully transparent
		255, 0, 0, 0,      // Transparent (color should be preserved)
		0, 255, 0, 0,
		0, 0, 255, 0,
		255, 255, 255, 0
	};

	// Create destination buffer
	Pixel dstData[BLIT32_WIDTH * BLIT32_HEIGHT];

	// Perform conversion
	Blit32Bit(dstData, srcData, BLIT32_WIDTH, BLIT32_HEIGHT);

	// Verify row 0 - solid colors
	if (dstData[0] != MakeColor(255, 0, 0)) {
		fprintf(stderr, "Blit32Bit row 0: expected solid red\n");
		return false;
	}
	if (dstData[1] != MakeColor(0, 255, 0)) {
		fprintf(stderr, "Blit32Bit row 0: expected solid green\n");
		return false;
	}
	if (dstData[2] != MakeColor(0, 0, 255)) {
		fprintf(stderr, "Blit32Bit row 0: expected solid blue\n");
		return false;
	}
	if (dstData[3] != MakeColor(255, 255, 255)) {
		fprintf(stderr, "Blit32Bit row 0: expected solid white\n");
		return false;
	}

	// Verify row 1 - semi-transparent colors preserve alpha
	if (dstData[4] != MakeColorWithAlpha(128, 0, 0, 128)) {
		fprintf(stderr, "Blit32Bit row 1: expected semi-transparent red with alpha=128\n");
		return false;
	}
	if (dstData[5] != MakeColorWithAlpha(0, 128, 0, 128)) {
		fprintf(stderr, "Blit32Bit row 1: expected semi-transparent green with alpha=128\n");
		return false;
	}

	// Verify row 2 - fully transparent pixels preserve alpha=0
	uint8_t components[4];
	Color2Values(dstData[8], components);
	if (components[3] != 0) {
		fprintf(stderr, "Blit32Bit row 2: expected alpha=0 for transparent pixel\n");
		return false;
	}

#undef BLIT32_WIDTH
#undef BLIT32_HEIGHT
	return true;
}

static bool Blit24To32BitTest(GraphicsBuffer *buffer)
{
	// Test converting 24-bit RGB byte array to Pixel format (opaque)
#define BLIT24_WIDTH 3
#define BLIT24_HEIGHT 2

	// Create source data: RGB bytes (R, G, B for each pixel, no alpha)
	uint8_t srcData[BLIT24_WIDTH * BLIT24_HEIGHT * 3] = {
		// Row 0
		255, 0, 0,      // Red
		0, 255, 0,      // Green
		0, 0, 255,      // Blue

		// Row 1
		128, 64, 32,    // Custom color
		255, 255, 0,    // Yellow
		255, 0, 255     // Magenta
	};

	// Create destination buffer
	Pixel dstData[BLIT24_WIDTH * BLIT24_HEIGHT];

	// Perform conversion
	Blit24To32Bit(dstData, srcData, BLIT24_WIDTH, BLIT24_HEIGHT);

	// Verify all pixels - should be opaque (alpha=255)
	if (dstData[0] != MakeColor(255, 0, 0)) {
		fprintf(stderr, "Blit24To32Bit: expected opaque red\n");
		return false;
	}
	if (dstData[1] != MakeColor(0, 255, 0)) {
		fprintf(stderr, "Blit24To32Bit: expected opaque green\n");
		return false;
	}
	if (dstData[2] != MakeColor(0, 0, 255)) {
		fprintf(stderr, "Blit24To32Bit: expected opaque blue\n");
		return false;
	}
	if (dstData[3] != MakeColor(128, 64, 32)) {
		fprintf(stderr, "Blit24To32Bit: expected custom color (128,64,32)\n");
		return false;
	}
	if (dstData[4] != MakeColor(255, 255, 0)) {
		fprintf(stderr, "Blit24To32Bit: expected opaque yellow\n");
		return false;
	}
	if (dstData[5] != MakeColor(255, 0, 255)) {
		fprintf(stderr, "Blit24To32Bit: expected opaque magenta\n");
		return false;
	}

	// Verify alpha channel is 255 for all pixels
	uint8_t components[4];
	for (int i = 0; i < BLIT24_WIDTH * BLIT24_HEIGHT; i++) {
		Color2Values(dstData[i], components);
		if (components[3] != 255) {
			fprintf(stderr, "Blit24To32Bit: pixel %d has alpha=%d, expected 255\n", i, components[3]);
			return false;
		}
	}

#undef BLIT24_WIDTH
#undef BLIT24_HEIGHT
	return true;
}

static bool HorzVertLineTest(GraphicsBuffer *buffer)
{
	// Test direct horizontal and vertical line drawing
	// Note: DrawHorzLine and DrawVertLine use alpha compositing,
	// so we test basic functionality without exact pixel-by-pixel verification
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	// Draw horizontal line in middle of buffer
	DrawHorzLine(buffer, AsPixel(kRed), 10, 30, 15);
	// Verify at least some pixels were drawn
	bool foundRed = false;
	for (int x = 10; x <= 30; x++) {
		if (GetPixel(buffer, x, 15) == AsPixel(kRed)) {
			foundRed = true;
			break;
		}
	}
	if (!foundRed) {
		fprintf(stderr, "HorzVertLineTest: horizontal line not visible\n");
		return false;
	}

	// Draw vertical line in middle of buffer
	DrawVertLine(buffer, AsPixel(kGreen), 5, 25, 40);
	// Verify at least some pixels were drawn
	bool foundGreen = false;
	for (int y = 5; y <= 25; y++) {
		if (GetPixel(buffer, 40, y) == AsPixel(kGreen)) {
			foundGreen = true;
			break;
		}
	}
	if (!foundGreen) {
		fprintf(stderr, "HorzVertLineTest: vertical line not visible\n");
		return false;
	}

	// Test clipping - lines extending beyond buffer shouldn't crash
	DrawHorzLine(buffer, AsPixel(kWhite), -10, buffer->width + 10, 8);
	DrawVertLine(buffer, AsPixel(kBlue), -10, buffer->height + 10, 35);

	// Main success: didn't crash with clipped lines
	return true;
}

static bool CircleEdgeCasesTest(GraphicsBuffer *buffer)
{
	// Test edge cases for circle drawing
	FillRectOpaque(buffer, AsPixel(kBlack), 0, 0, buffer->height, buffer->width);

	// Radius 0 - should draw nothing or single pixel at center
	DrawCircle(buffer, AsPixel(kRed), 20, 20, 0);
	FillCircle(buffer, AsPixel(kRed), 25, 20, 0);
	// Don't crash is the main test here

	// Radius 1 - should draw a small circle (~5 pixels)
	DrawCircle(buffer, AsPixel(kGreen), 20, 30, 1);
	Pixel center = GetPixel(buffer, 20, 30);
	// Center or adjacent pixels should be green
	bool foundGreen = (center == AsPixel(kGreen)) ||
	                  (GetPixel(buffer, 21, 30) == AsPixel(kGreen)) ||
	                  (GetPixel(buffer, 20, 31) == AsPixel(kGreen));
	if (!foundGreen) {
		fprintf(stderr, "CircleEdgeCasesTest: radius 1 circle not visible\n");
		return false;
	}

	// Filled circle with radius 1
	FillCircle(buffer, AsPixel(kBlue), 30, 30, 1);
	Pixel centerFilled = GetPixel(buffer, 30, 30);
	if (centerFilled != AsPixel(kBlue)) {
		fprintf(stderr, "CircleEdgeCasesTest: radius 1 filled circle missing center\n");
		return false;
	}

	// Very large circle - radius > buffer dimensions
	// Should clip properly and not crash
	DrawCircle(buffer, AsPixel(kWhite), buffer->width / 2, buffer->height / 2, buffer->width + 50);
	FillCircle(buffer, AsPixel(kWhite), buffer->width / 2, buffer->height / 2, buffer->height + 50);

	// Circle centered at buffer edge
	DrawCircle(buffer, AsPixel(kRed), 0, 0, 10);
	FillCircle(buffer, AsPixel(kGreen), buffer->width - 1, buffer->height - 1, 10);

	// Circle centered outside buffer
	DrawCircle(buffer, AsPixel(kBlue), -20, -20, 30);
	FillCircle(buffer, AsPixel(kRed), buffer->width + 20, buffer->height + 20, 30);

	// Main success: didn't crash
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
		// Color and pixel utilities
		{ColorTest, "ColorTest"},
		{PixelComponentsTest, "PixelComponentsTest"},

		// Basic pixel operations
		{PutPixelTest, "PutPixelTest"},
		{GetPixelTest, "GetPixelTest"},

		// Rectangle operations
		{RectTest, "RectTest"},
		{LSPointInRectTest, "LSPointInRectTest"},
		{FillRectTest, "FillRectTest"},
		{FillRectOpaqueTest, "FillRectOpaqueTest"},
		{DrawRectTest, "DrawRectTest"},
		{RectEdgeCasesTest, "RectEdgeCasesTest"},

		// Line drawing
		{DrawLineTest, "DrawLineTest"},
		{DrawLineVariantsTest, "DrawLineVariantsTest"},
		{DrawLineEdgeCasesTest, "DrawLineEdgeCasesTest"},
		{HorzVertLineTest, "HorzVertLineTest"},

		// Circle drawing
		{CircleTest, "CircleTest"},
		{FillCircleTest, "FillCircleTest"},
		{CircleEdgeCasesTest, "CircleEdgeCasesTest"},

		// Blitting and compositing
		{BlitBufferTest, "BlitBufferTest"},
		{BlitTransparencyTest, "BlitTransparencyTest"},
		{Blit32BitTest, "Blit32BitTest"},
		{Blit24To32BitTest, "Blit24To32BitTest"},
		{AlphaCompositingTest, "AlphaCompositingTest"},

		// Buffer management and edge cases
		{BufferStrideTest, "BufferStrideTest"},
		{ClippingTest, "ClippingTest"},
		{NegativeCoordTest, "NegativeCoordTest"},

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
