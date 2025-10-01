/**
 * Visual Integration Tests for Finch Graphics Library
 *
 * This test suite creates PNG images demonstrating all major graphics
 * operations. These serve dual purposes:
 * 1. Automated visual regression testing
 * 2. Visual documentation of what each function does
 *
 * Each test renders a specific graphics operation with labels and
 * saves a PNG file for inspection.
 *
 * Run with: make run_visual_integration_test
 */

#include "finch.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <stdbool.h>
#include <math.h>

// Test image dimensions
#define TEST_WIDTH 800
#define TEST_HEIGHT 600

// Reference images directory
#define REFERENCE_DIR "test_references/"

// Pixel comparison tolerance (allow small differences for rounding/AA)
#define PIXEL_TOLERANCE 2
#define MAX_DIFFERENT_PIXELS_PERCENT 0.1  // Allow 0.1% of pixels to differ

// Load PNG file into a buffer
static GraphicsBuffer* LoadPNG(const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        return NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return NULL;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(fp);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return NULL;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    // Convert to RGBA if needed
    if (bit_depth == 16)
        png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    // Allocate buffer
    uint32_t* pixels = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    if (!pixels) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return NULL;
    }

    // Read image
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_bytep)(pixels + y * width);
    }

    png_read_image(png, row_pointers);
    free(row_pointers);

    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);

    return NewGraphBuffer(pixels, width, height, width, 0);
}

// Compare two pixels with tolerance
static bool PixelsMatch(Pixel p1, Pixel p2, int tolerance)
{
    uint8_t* c1 = (uint8_t*)&p1;
    uint8_t* c2 = (uint8_t*)&p2;

    for (int i = 0; i < 4; i++) {
        int diff = abs(c1[i] - c2[i]);
        if (diff > tolerance) {
            return false;
        }
    }
    return true;
}

// Compare two buffers and return difference statistics
static bool CompareBuffers(GraphicsBuffer* actual, GraphicsBuffer* expected,
                          int* outDifferentPixels, int* outMaxDiff)
{
    if (!actual || !expected) {
        return false;
    }

    if (actual->width != expected->width || actual->height != expected->height) {
        fprintf(stderr, "Size mismatch: actual %dx%d vs expected %dx%d\n",
                actual->width, actual->height, expected->width, expected->height);
        return false;
    }

    int differentPixels = 0;
    int maxDiff = 0;

    for (uint32_t y = 0; y < actual->height; y++) {
        for (uint32_t x = 0; x < actual->width; x++) {
            Pixel actualPixel = GetPixel(actual, x, y);
            Pixel expectedPixel = GetPixel(expected, x, y);

            if (!PixelsMatch(actualPixel, expectedPixel, PIXEL_TOLERANCE)) {
                differentPixels++;

                // Calculate max difference for diagnostics
                uint8_t* c1 = (uint8_t*)&actualPixel;
                uint8_t* c2 = (uint8_t*)&expectedPixel;
                for (int i = 0; i < 4; i++) {
                    int diff = abs(c1[i] - c2[i]);
                    if (diff > maxDiff) maxDiff = diff;
                }
            }
        }
    }

    *outDifferentPixels = differentPixels;
    *outMaxDiff = maxDiff;

    int totalPixels = actual->width * actual->height;
    double diffPercent = (100.0 * differentPixels) / totalPixels;

    return diffPercent <= MAX_DIFFERENT_PIXELS_PERCENT;
}

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

    for (uint32_t y = 0; y < buffer->height; y++) {
        png_write_row(png, (png_bytep)(buffer->ptr + y * buffer->rowPixels));
    }

    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);

    return true;
}

// Create buffer for testing
static GraphicsBuffer* CreateTestBuffer(void)
{
    uint32_t* pixels = (uint32_t*)malloc(TEST_WIDTH * TEST_HEIGHT * sizeof(uint32_t));
    if (!pixels) return NULL;

    GraphicsBuffer* buffer = NewGraphBuffer(pixels, TEST_WIDTH, TEST_HEIGHT, TEST_WIDTH, 0);
    if (!buffer) {
        free(pixels);
        return NULL;
    }

    return buffer;
}

// Clean up test buffer
static void DeleteTestBuffer(GraphicsBuffer* buffer)
{
    if (buffer) {
        free(buffer->ptr);
        DeleteGraphBuffer(buffer);
    }
}

// Helper to run test with verification
static bool RunTestWithVerification(const char* testName,
                                    void (*renderFunc)(GraphicsBuffer*),
                                    const char* outputFilename)
{
    printf("Testing %s...\n", testName);

    // Create buffer and render
    GraphicsBuffer* buffer = CreateTestBuffer();
    if (!buffer) return false;

    renderFunc(buffer);

    // Save output
    bool saved = SavePNG(outputFilename, buffer);
    if (!saved) {
        DeleteTestBuffer(buffer);
        return false;
    }

    printf("  ✓ Saved %s\n", outputFilename);

    // Try to load reference image
    char refPath[256];
    snprintf(refPath, sizeof(refPath), "%s%s", REFERENCE_DIR, outputFilename);

    GraphicsBuffer* reference = LoadPNG(refPath);
    if (!reference) {
        printf("  ⚠ No reference image found at %s\n", refPath);
        printf("    Run 'make generate_reference_images' to create references\n");
        DeleteTestBuffer(buffer);
        return true; // Pass if no reference exists yet
    }

    // Compare with reference
    int differentPixels = 0;
    int maxDiff = 0;
    bool matches = CompareBuffers(buffer, reference, &differentPixels, &maxDiff);

    if (matches) {
        printf("  ✓ Matches reference image\n");
    } else {
        int totalPixels = buffer->width * buffer->height;
        double diffPercent = (100.0 * differentPixels) / totalPixels;
        printf("  ✗ Does NOT match reference:\n");
        printf("    Different pixels: %d/%d (%.2f%%)\n",
               differentPixels, totalPixels, diffPercent);
        printf("    Max channel diff: %d\n", maxDiff);
    }

    DeleteTestBuffer(buffer);
    DeleteTestBuffer(reference);

    return matches;
}

// Render function for basic primitives test
static void RenderBasicPrimitives(GraphicsBuffer* buffer)
{

    // Dark background
    ClearBuffer(buffer, 0xFF202020);

    // Title
    DrawTextCentered(buffer, COLOR_WHITE, TEST_WIDTH / 2, 20, "BASIC DRAWING PRIMITIVES");

    // Individual pixels
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 60, "PutPixel:");
    for (int i = 0; i < 50; i++) {
        PutPixel(buffer, COLOR_GREEN, 20 + i, 80 + (i % 5));
    }

    // Horizontal line
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 110, "Horizontal Line:");
    DrawLine(buffer, COLOR_RED, 20, 130, 300, 130);

    // Vertical line
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 160, "Vertical Line:");
    DrawLine(buffer, COLOR_BLUE, 50, 180, 50, 280);

    // Diagonal lines
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 310, "Diagonal Lines:");
    DrawLine(buffer, COLOR_YELLOW, 20, 330, 150, 400);
    DrawLine(buffer, COLOR_CYAN, 150, 330, 20, 400);

    // Rectangle outline
    DrawText(buffer, COLOR_LIGHT_GRAY, 320, 60, "Rectangle Outline:");
    DrawRect(buffer, COLOR_MAGENTA, 320, 80, 480, 150);

    // Filled rectangle
    DrawText(buffer, COLOR_LIGHT_GRAY, 320, 180, "Filled Rectangle:");
    FillRectOpaque(buffer, COLOR_GREEN, 320, 200, 480, 270);
    DrawRect(buffer, COLOR_WHITE, 320, 200, 480, 270);

    // Opaque filled rectangle (RGB color)
    DrawText(buffer, COLOR_LIGHT_GRAY, 320, 300, "Opaque Fill (RGB):");
    RGBColor24 rgb = {255, 128, 0}; // Orange
    FillRectOpaque(buffer, AsPixel(rgb), 320, 320, 480, 390);

    // Color test - gradients
    DrawText(buffer, COLOR_LIGHT_GRAY, 520, 60, "Color Gradient:");
    for (int i = 0; i < 100; i++) {
        RGBColor24 c = {i * 255 / 100, 0, 255 - i * 255 / 100};
        Pixel p = AsPixel(c);
        DrawLine(buffer, p, 520 + i, 80, 520 + i, 150);
    }

    // Alpha blending test
    DrawText(buffer, COLOR_LIGHT_GRAY, 520, 180, "Alpha Blending:");
    FillRectOpaque(buffer, COLOR_RED, 520, 200, 620, 250);
    RGBColor24 semiBlue = {0, 0, 255};
    Pixel semiTransBlue = AsPixelWithAlpha(semiBlue, 128);
    FillRectOpaque(buffer, semiTransBlue, 570, 215, 670, 265);

}

// Test wrapper for basic primitives
static bool TestBasicPrimitives(void)
{
    return RunTestWithVerification("basic drawing primitives",
                                  RenderBasicPrimitives,
                                  "visual_test_primitives.png");
}

// Render function for circles test
static void RenderCircles(GraphicsBuffer* buffer)
{
    ClearBuffer(buffer, 0xFF202020);

    DrawTextCentered(buffer, COLOR_WHITE, TEST_WIDTH / 2, 20, "CIRCLE DRAWING");

    // Circle outlines
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 60, "Circle Outlines:");
    DrawCircle(buffer, COLOR_RED, 80, 130, 40);
    DrawCircle(buffer, COLOR_GREEN, 180, 130, 30);
    DrawCircle(buffer, COLOR_BLUE, 260, 130, 20);
    DrawCircle(buffer, COLOR_YELLOW, 320, 130, 10);

    // Filled circles
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 220, "Filled Circles:");
    FillCircle(buffer, COLOR_RED, 80, 300, 40);
    FillCircle(buffer, COLOR_GREEN, 180, 300, 30);
    FillCircle(buffer, COLOR_BLUE, 260, 300, 20);
    FillCircle(buffer, COLOR_YELLOW, 320, 300, 10);

    // Filled with outline
    DrawText(buffer, COLOR_LIGHT_GRAY, 400, 60, "Filled + Outline:");
    FillCircle(buffer, COLOR_CYAN, 480, 130, 50);
    DrawCircle(buffer, COLOR_WHITE, 480, 130, 50);

    FillCircle(buffer, COLOR_MAGENTA, 600, 130, 40);
    DrawCircle(buffer, COLOR_WHITE, 600, 130, 40);

    // Overlapping circles with alpha
    DrawText(buffer, COLOR_LIGHT_GRAY, 400, 220, "Alpha Blending:");
    RGBColor24 red = {255, 0, 0};
    RGBColor24 green = {0, 255, 0};
    RGBColor24 blue = {0, 0, 255};

    Pixel semiRed = AsPixelWithAlpha(red, 180);
    Pixel semiGreen = AsPixelWithAlpha(green, 180);
    Pixel semiBlue = AsPixelWithAlpha(blue, 180);

    FillCircle(buffer, semiRed, 480, 300, 45);
    FillCircle(buffer, semiGreen, 520, 330, 45);
    FillCircle(buffer, semiBlue, 550, 300, 45);

    // Concentric circles
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 420, "Concentric:");
    for (int r = 10; r <= 50; r += 10) {
        DrawCircle(buffer, COLOR_LIGHT_GRAY, 100, 510, r);
    }
}

// Test wrapper for circles
static bool TestCircles(void)
{
    return RunTestWithVerification("circle drawing",
                                  RenderCircles,
                                  "visual_test_circles.png");
}

// Render function for rectangles test
static void RenderRectangles(GraphicsBuffer* buffer)
{
    ClearBuffer(buffer, 0xFF202020);

    DrawTextCentered(buffer, COLOR_WHITE, TEST_WIDTH / 2, 20, "RECTANGLES AND CLIPPING");

    // Various rectangles
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 60, "Various Sizes:");

    DrawRect(buffer, COLOR_RED, 20, 80, 120, 180);
    DrawRect(buffer, COLOR_GREEN, 140, 80, 200, 180);
    DrawRect(buffer, COLOR_BLUE, 220, 80, 340, 180);

    // Filled rectangles
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 210, "Filled:");
    FillRectOpaque(buffer, COLOR_RED, 20, 230, 120, 280);
    FillRectOpaque(buffer, COLOR_GREEN, 140, 230, 200, 280);
    FillRectOpaque(buffer, COLOR_BLUE, 220, 230, 340, 280);

    // Edge cases - partial clipping
    DrawText(buffer, COLOR_LIGHT_GRAY, 400, 60, "Clipping Test:");
    DrawText(buffer, COLOR_GRAY, 400, 80, "(Partial offscreen)");

    // Rectangles partially off screen
    FillRectOpaque(buffer, COLOR_YELLOW, TEST_WIDTH - 50, 100, TEST_WIDTH + 50, 200);
    FillRectOpaque(buffer, COLOR_CYAN, 400, TEST_HEIGHT - 50, 500, TEST_HEIGHT + 50);

    DrawRect(buffer, COLOR_MAGENTA, TEST_WIDTH - 60, 220, TEST_WIDTH + 40, 280);

    // Overlapping rectangles
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 310, "Overlapping:");
    FillRectOpaque(buffer, COLOR_RED, 20, 330, 150, 420);
    FillRectOpaque(buffer, COLOR_GREEN, 80, 360, 210, 450);
    FillRectOpaque(buffer, COLOR_BLUE, 140, 390, 270, 480);

    // Checkerboard pattern
    DrawText(buffer, COLOR_LIGHT_GRAY, 400, 310, "Checkerboard:");
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Pixel color = ((x + y) % 2) ? COLOR_WHITE : COLOR_BLACK;
            FillRectOpaque(buffer, color, 400 + x * 20, 330 + y * 20,
                    420 + x * 20, 350 + y * 20);
        }
    }
}

// Test wrapper for rectangles
static bool TestRectangles(void)
{
    return RunTestWithVerification("rectangles and clipping",
                                  RenderRectangles,
                                  "visual_test_rectangles.png");
}

// Render function for lines test
static void RenderLines(GraphicsBuffer* buffer)
{
    ClearBuffer(buffer, 0xFF202020);

    DrawTextCentered(buffer, COLOR_WHITE, TEST_WIDTH / 2, 20, "LINE DRAWING - ALL ANGLES");

    // Star pattern - lines from center
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 60, "Radial Lines:");
    int cx = 150, cy = 200;
    for (int angle = 0; angle < 360; angle += 15) {
        double rad = angle * 3.14159 / 180.0;
        int x = cx + 100 * cos(rad);
        int y = cy + 100 * sin(rad);

        // Rainbow colors
        RGBColor24 rgb = {
            (uint8_t)(128 + 127 * sin(rad)),
            (uint8_t)(128 + 127 * sin(rad + 2.094)),
            (uint8_t)(128 + 127 * sin(rad + 4.189))
        };
        DrawLine(buffer, AsPixel(rgb), cx, cy, x, y);
    }

    // Grid pattern
    DrawText(buffer, COLOR_LIGHT_GRAY, 400, 60, "Grid Pattern:");
    for (int i = 0; i <= 10; i++) {
        // Vertical lines
        DrawLine(buffer, COLOR_GRAY, 400 + i * 30, 80, 400 + i * 30, 380);
        // Horizontal lines
        DrawLine(buffer, COLOR_GRAY, 400, 80 + i * 30, 700, 80 + i * 30);
    }

    // Thick lines using multiple parallel lines
    DrawText(buffer, COLOR_LIGHT_GRAY, 20, 380, "Thick Lines:");
    for (int i = -2; i <= 2; i++) {
        DrawLine(buffer, COLOR_RED, 20, 410 + i, 200, 410 + i);
        DrawLine(buffer, COLOR_GREEN, 220 + i, 400, 220 + i, 500);
    }

    // Diagonal lines - all octants
    DrawText(buffer, COLOR_LIGHT_GRAY, 400, 420, "All Octants:");
    int ox = 550, oy = 510;
    DrawLine(buffer, COLOR_RED, ox, oy, ox + 60, oy);         // 0°
    DrawLine(buffer, COLOR_YELLOW, ox, oy, ox + 50, oy - 30); // 45°
    DrawLine(buffer, COLOR_GREEN, ox, oy, ox, oy - 60);       // 90°
    DrawLine(buffer, COLOR_CYAN, ox, oy, ox - 50, oy - 30);   // 135°
    DrawLine(buffer, COLOR_BLUE, ox, oy, ox - 60, oy);        // 180°
    DrawLine(buffer, COLOR_MAGENTA, ox, oy, ox - 50, oy + 30);// 225°
    DrawLine(buffer, COLOR_WHITE, ox, oy, ox, oy + 60);       // 270°
    DrawLine(buffer, COLOR_LIGHT_GRAY, ox, oy, ox + 50, oy + 30); // 315°
}

// Test wrapper for lines
static bool TestLines(void)
{
    return RunTestWithVerification("line drawing at various angles",
                                  RenderLines,
                                  "visual_test_lines.png");
}

// Render function for complex scene test
static void RenderComplexScene(GraphicsBuffer* buffer)
{
    // Sky gradient
    for (int y = 0; y < TEST_HEIGHT / 2; y++) {
        RGBColor24 sky = {
            100,
            150 + y * 105 / (TEST_HEIGHT / 2),
            255
        };
        DrawLine(buffer, AsPixel(sky), 0, y, TEST_WIDTH, y);
    }

    // Ground
    FillRectOpaque(buffer, 0xFF228B22, 0, TEST_HEIGHT / 2, TEST_WIDTH, TEST_HEIGHT);

    DrawTextCentered(buffer, COLOR_WHITE, TEST_WIDTH / 2, 20, "COMPLEX SCENE TEST");

    // Sun
    FillCircle(buffer, COLOR_YELLOW, 700, 80, 40);
    DrawCircle(buffer, 0xFFFFAA00, 700, 80, 40);

    // House
    FillRectOpaque(buffer, 0xFF8B4513, 100, 250, 300, 450); // Brown walls
    FillRectOpaque(buffer, 0xFF4169E1, 150, 300, 210, 400); // Blue door
    FillRectOpaque(buffer, 0xFF87CEEB, 220, 300, 270, 350); // Light blue window

    // Roof
    DrawLine(buffer, COLOR_RED, 100, 250, 200, 180);
    DrawLine(buffer, COLOR_RED, 200, 180, 300, 250);
    DrawLine(buffer, COLOR_RED, 100, 250, 300, 250);

    // Fill roof (triangle)
    for (int y = 180; y < 250; y++) {
        int width = (y - 180) * 100 / 70;
        DrawLine(buffer, 0xFF8B0000, 200 - width, y, 200 + width, y);
    }

    // Tree
    FillRectOpaque(buffer, 0xFF8B4513, 450, 350, 480, 450); // Trunk
    FillCircle(buffer, 0xFF228B22, 465, 320, 50);     // Leaves
    DrawCircle(buffer, 0xFF006400, 465, 320, 50);

    // Flowers
    for (int i = 0; i < 5; i++) {
        int x = 350 + i * 40;
        DrawLine(buffer, 0xFF228B22, x, 460, x, 440); // Stem
        FillCircle(buffer, COLOR_MAGENTA, x, 435, 8); // Flower
    }

    // Cloud using multiple circles
    int cloud_x = 150, cloud_y = 100;
    FillCircle(buffer, COLOR_WHITE, cloud_x, cloud_y, 20);
    FillCircle(buffer, COLOR_WHITE, cloud_x + 25, cloud_y, 25);
    FillCircle(buffer, COLOR_WHITE, cloud_x + 50, cloud_y, 20);
    FillCircle(buffer, COLOR_WHITE, cloud_x + 70, cloud_y + 5, 18);

    // Birds (V shapes)
    for (int i = 0; i < 3; i++) {
        int bx = 500 + i * 80;
        int by = 120 + i * 20;
        DrawLine(buffer, COLOR_BLACK, bx, by, bx + 10, by - 8);
        DrawLine(buffer, COLOR_BLACK, bx + 10, by - 8, bx + 20, by);
    }
}

// Test wrapper for complex scene
static bool TestComplexScene(void)
{
    return RunTestWithVerification("complex scene with all primitives",
                                  RenderComplexScene,
                                  "visual_test_complex_scene.png");
}

int main(int argc, char* argv[])
{
    printf("=== Finch Visual Integration Tests ===\n\n");

    int passed = 0;
    int total = 5;

    if (TestBasicPrimitives()) passed++;
    if (TestCircles()) passed++;
    if (TestRectangles()) passed++;
    if (TestLines()) passed++;
    if (TestComplexScene()) passed++;

    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", passed, total);

    if (passed == total) {
        printf("✓ All visual integration tests completed successfully!\n");
        printf("\nGenerated test images:\n");
        printf("  - visual_test_primitives.png\n");
        printf("  - visual_test_circles.png\n");
        printf("  - visual_test_rectangles.png\n");
        printf("  - visual_test_lines.png\n");
        printf("  - visual_test_complex_scene.png\n");
        printf("\nPlease visually inspect these images to verify correctness.\n");
        return 0;
    } else {
        printf("✗ Some tests failed\n");
        return 1;
    }
}
