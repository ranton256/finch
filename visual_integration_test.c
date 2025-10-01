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

// Test 1: Basic drawing primitives
static bool TestBasicPrimitives(void)
{
    printf("Testing basic drawing primitives...\n");

    GraphicsBuffer* buffer = CreateTestBuffer();
    if (!buffer) return false;

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

    bool result = SavePNG("visual_test_primitives.png", buffer);
    DeleteTestBuffer(buffer);

    if (result) {
        printf("  ✓ Saved visual_test_primitives.png\n");
    }
    return result;
}

// Test 2: Circles
static bool TestCircles(void)
{
    printf("Testing circle drawing...\n");

    GraphicsBuffer* buffer = CreateTestBuffer();
    if (!buffer) return false;

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

    bool result = SavePNG("visual_test_circles.png", buffer);
    DeleteTestBuffer(buffer);

    if (result) {
        printf("  ✓ Saved visual_test_circles.png\n");
    }
    return result;
}

// Test 3: Rectangles and clipping
static bool TestRectangles(void)
{
    printf("Testing rectangles and clipping...\n");

    GraphicsBuffer* buffer = CreateTestBuffer();
    if (!buffer) return false;

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

    bool result = SavePNG("visual_test_rectangles.png", buffer);
    DeleteTestBuffer(buffer);

    if (result) {
        printf("  ✓ Saved visual_test_rectangles.png\n");
    }
    return result;
}

// Test 4: Lines at various angles
static bool TestLines(void)
{
    printf("Testing line drawing at various angles...\n");

    GraphicsBuffer* buffer = CreateTestBuffer();
    if (!buffer) return false;

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

    bool result = SavePNG("visual_test_lines.png", buffer);
    DeleteTestBuffer(buffer);

    if (result) {
        printf("  ✓ Saved visual_test_lines.png\n");
    }
    return result;
}

// Test 5: Complex scene combining all primitives
static bool TestComplexScene(void)
{
    printf("Testing complex scene with all primitives...\n");

    GraphicsBuffer* buffer = CreateTestBuffer();
    if (!buffer) return false;

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

    bool result = SavePNG("visual_test_complex_scene.png", buffer);
    DeleteTestBuffer(buffer);

    if (result) {
        printf("  ✓ Saved visual_test_complex_scene.png\n");
    }
    return result;
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
