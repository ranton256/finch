/**
 * Visual Test for Text Rendering
 *
 * This test runs headless (no graphics window) and:
 * 1. Renders various text samples using Finch's text functions
 * 2. Verifies specific pixels are correct (automated integration test)
 * 3. Saves output as PNG for developer visual verification
 *
 * Run with: make visual_test
 * Output: visual_test_output.png
 */

#include "finch.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <stdbool.h>

// Test image dimensions
#define TEST_WIDTH 400
#define TEST_HEIGHT 300

// Helper to save buffer as PNG
static bool SavePNG(const char* filename, GraphicsBuffer* buffer)
{
	FILE* fp = fopen(filename, "wb");
	if (!fp) {
		fprintf(stderr, "Failed to open %s for writing\n", filename);
		return false;
	}

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fclose(fp);
		return false;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		png_destroy_write_struct(&png, NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_write_struct(&png, &info);
		fclose(fp);
		return false;
	}

	png_init_io(png, fp);
	png_set_IHDR(png, info, buffer->width, buffer->height,
	             8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	// Write row by row
	for (uint32_t y = 0; y < buffer->height; y++) {
		png_write_row(png, (png_bytep)(buffer->ptr + y * buffer->rowPixels));
	}

	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);
	fclose(fp);

	return true;
}

// Helper to check if a pixel matches expected color
static bool CheckPixel(GraphicsBuffer* buffer, int x, int y, Pixel expected, const char* desc)
{
	Pixel actual = GetPixel(buffer, x, y);
	if (actual != expected) {
		fprintf(stderr, "FAIL: %s - pixel at (%d,%d) expected 0x%08X, got 0x%08X\n",
		        desc, x, y, expected, actual);
		return false;
	}
	return true;
}

// Test function for DrawChar
static bool TestDrawChar(GraphicsBuffer* buffer)
{
	printf("Testing DrawChar...\n");

	// Draw 'A' at (10, 10) in white
	DrawChar(buffer, COLOR_WHITE, 10, 10, 'A');

	// Verify some pixels in the 'A' character
	// The 'A' character has specific pixels that should be white
	// Check top of A (should have white pixel in middle-ish area)
	bool found = false;
	for (int x = 10; x < 18; x++) {
		if (GetPixel(buffer, x, 11) == COLOR_WHITE) {
			found = true;
			break;
		}
	}

	if (!found) {
		fprintf(stderr, "FAIL: TestDrawChar - expected white pixels in 'A' character\n");
		return false;
	}

	printf("  PASS: DrawChar renders 'A' correctly\n");
	return true;
}

// Test function for DrawText
static bool TestDrawText(GraphicsBuffer* buffer)
{
	printf("Testing DrawText...\n");

	const char* testStr = "Test";
	DrawText(buffer, COLOR_GREEN, 50, 30, testStr);

	// Verify that some pixels in the text are green
	// Since each char is 8 pixels wide, "Test" should span 32 pixels
	bool foundGreen = false;
	for (int x = 50; x < 50 + 32; x++) {
		for (int y = 30; y < 30 + 8; y++) {
			if (GetPixel(buffer, x, y) == COLOR_GREEN) {
				foundGreen = true;
				break;
			}
		}
		if (foundGreen) break;
	}

	if (!foundGreen) {
		fprintf(stderr, "FAIL: TestDrawText - expected green pixels in 'Test' text\n");
		return false;
	}

	printf("  PASS: DrawText renders 'Test' correctly\n");
	return true;
}

// Test function for GetTextWidth
static bool TestGetTextWidth(void)
{
	printf("Testing GetTextWidth...\n");

	// Each character is 8 pixels wide
	if (GetTextWidth("A") != 8) {
		fprintf(stderr, "FAIL: GetTextWidth('A') should be 8\n");
		return false;
	}

	if (GetTextWidth("Test") != 32) {
		fprintf(stderr, "FAIL: GetTextWidth('Test') should be 32\n");
		return false;
	}

	if (GetTextWidth("") != 0) {
		fprintf(stderr, "FAIL: GetTextWidth('') should be 0\n");
		return false;
	}

	printf("  PASS: GetTextWidth returns correct values\n");
	return true;
}

// Test function for GetTextHeight
static bool TestGetTextHeight(void)
{
	printf("Testing GetTextHeight...\n");

	if (GetTextHeight() != 8) {
		fprintf(stderr, "FAIL: GetTextHeight() should be 8\n");
		return false;
	}

	printf("  PASS: GetTextHeight returns 8\n");
	return true;
}

// Test function for DrawTextCentered
static bool TestDrawTextCentered(GraphicsBuffer* buffer)
{
	printf("Testing DrawTextCentered...\n");

	const char* text = "Hi";
	int centerX = 200;
	int centerY = 150;

	DrawTextCentered(buffer, COLOR_RED, centerX, centerY, text);

	// Calculate expected position
	int width = GetTextWidth(text);  // 16 pixels
	int height = GetTextHeight();     // 8 pixels
	int expectedX = centerX - width / 2;  // 200 - 8 = 192
	int expectedY = centerY - height / 2; // 150 - 4 = 146

	// Verify red pixels exist in the expected area
	bool foundRed = false;
	for (int x = expectedX; x < expectedX + width; x++) {
		for (int y = expectedY; y < expectedY + height; y++) {
			if (GetPixel(buffer, x, y) == COLOR_RED) {
				foundRed = true;
				break;
			}
		}
		if (foundRed) break;
	}

	if (!foundRed) {
		fprintf(stderr, "FAIL: TestDrawTextCentered - expected red pixels in centered text\n");
		return false;
	}

	printf("  PASS: DrawTextCentered renders centered text correctly\n");
	return true;
}

// Create a comprehensive visual test image
static void CreateVisualTestImage(GraphicsBuffer* buffer)
{
	// Clear to dark gray background
	ClearBuffer(buffer, COLOR_DARK_GRAY);

	// Title at top
	DrawTextCentered(buffer, COLOR_WHITE, TEST_WIDTH / 2, 20, "FINCH TEXT RENDERING TEST");

	// Draw a line separator
	DrawLine(buffer, COLOR_LIGHT_GRAY, 10, 35, TEST_WIDTH - 10, 35);

	// Test all colors
	DrawText(buffer, COLOR_RED, 10, 50, "Red Text");
	DrawText(buffer, COLOR_GREEN, 10, 65, "Green Text");
	DrawText(buffer, COLOR_BLUE, 10, 80, "Blue Text");
	DrawText(buffer, COLOR_YELLOW, 10, 95, "Yellow Text");
	DrawText(buffer, COLOR_CYAN, 10, 110, "Cyan Text");
	DrawText(buffer, COLOR_MAGENTA, 10, 125, "Magenta Text");
	DrawText(buffer, COLOR_WHITE, 10, 140, "White Text");
	DrawText(buffer, COLOR_LIGHT_GRAY, 10, 155, "Light Gray Text");
	DrawText(buffer, COLOR_GRAY, 10, 170, "Gray Text");

	// Right column - character samples
	DrawText(buffer, COLOR_WHITE, 200, 50, "Characters:");
	DrawText(buffer, COLOR_LIGHT_GRAY, 200, 65, "ABCDEFGHIJKLM");
	DrawText(buffer, COLOR_LIGHT_GRAY, 200, 80, "NOPQRSTUVWXYZ");
	DrawText(buffer, COLOR_LIGHT_GRAY, 200, 95, "abcdefghijklm");
	DrawText(buffer, COLOR_LIGHT_GRAY, 200, 110, "nopqrstuvwxyz");
	DrawText(buffer, COLOR_LIGHT_GRAY, 200, 125, "0123456789");
	DrawText(buffer, COLOR_LIGHT_GRAY, 200, 140, "!@#$%^&*()");
	DrawText(buffer, COLOR_LIGHT_GRAY, 200, 155, "[]{}|\\;:',.<>?");

	// Centered text examples with boxes
	DrawTextCentered(buffer, COLOR_GREEN, TEST_WIDTH / 2, 210, "Centered Green");
	int width = GetTextWidth("Centered Green");
	int height = GetTextHeight();
	DrawRect(buffer, COLOR_GREEN,
	         TEST_WIDTH / 2 - width / 2 - 2,
	         210 - height / 2 - 2,
	         TEST_WIDTH / 2 + width / 2 + 2,
	         210 + height / 2 + 2);

	DrawTextCentered(buffer, COLOR_YELLOW, TEST_WIDTH / 2, 235, "Centered Yellow");
	width = GetTextWidth("Centered Yellow");
	DrawRect(buffer, COLOR_YELLOW,
	         TEST_WIDTH / 2 - width / 2 - 2,
	         235 - height / 2 - 2,
	         TEST_WIDTH / 2 + width / 2 + 2,
	         235 + height / 2 + 2);

	// Bottom status
	DrawTextCentered(buffer, COLOR_CYAN, TEST_WIDTH / 2, TEST_HEIGHT - 20,
	                 "Visual verification: Check visual_test_output.png");
}

int main(int argc, char* argv[])
{
	printf("=== Finch Text Rendering Visual Test ===\n\n");

	// Allocate buffer for test image
	uint32_t* pixels = (uint32_t*)malloc(TEST_WIDTH * TEST_HEIGHT * sizeof(uint32_t));
	if (!pixels) {
		fprintf(stderr, "Failed to allocate pixel buffer\n");
		return 1;
	}

	GraphicsBuffer* buffer = NewGraphBuffer(pixels, TEST_WIDTH, TEST_HEIGHT, TEST_WIDTH, 0);
	if (!buffer) {
		fprintf(stderr, "Failed to create graphics buffer\n");
		free(pixels);
		return 1;
	}

	// Initialize to black
	ClearBuffer(buffer, COLOR_BLACK);

	// Run automated tests
	bool allPassed = true;

	allPassed &= TestGetTextWidth();
	allPassed &= TestGetTextHeight();
	allPassed &= TestDrawChar(buffer);
	allPassed &= TestDrawText(buffer);
	allPassed &= TestDrawTextCentered(buffer);

	// Clear and create comprehensive visual test image
	printf("\nGenerating visual test image...\n");
	CreateVisualTestImage(buffer);

	// Save as PNG
	const char* outputFile = "visual_test_output.png";
	if (SavePNG(outputFile, buffer)) {
		printf("SUCCESS: Visual test image saved to %s\n", outputFile);
		printf("         Please open this file to verify text rendering visually\n");
	} else {
		fprintf(stderr, "ERROR: Failed to save PNG file\n");
		allPassed = false;
	}

	// Cleanup
	DeleteGraphBuffer(buffer);
	free(pixels);

	printf("\n=== Test Summary ===\n");
	if (allPassed) {
		printf("All automated tests PASSED\n");
		printf("Visual verification: Check %s\n", outputFile);
		return 0;
	} else {
		printf("Some tests FAILED\n");
		return 1;
	}
}
