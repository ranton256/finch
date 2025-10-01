/*
 * Screenshot Generator for Example Programs
 * Generates PNG screenshots for documentation
 */

#include "finch.h"
#include "blit.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <png.h>

// Forward declarations for example render functions
void RenderBounceExample(GraphicsBuffer* buffer);
void RenderFernExample(GraphicsBuffer* buffer);
void RenderTextExample(GraphicsBuffer* buffer);

// PNG saving function
static bool SavePNG(const char* filename, GraphicsBuffer* buffer)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Could not open %s for writing\n", filename);
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
    png_set_IHDR(png, info, buffer->width, buffer->height, 8,
                 PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    for (int y = 0; y < buffer->height; y++) {
        png_write_row(png, (png_bytep)(buffer->ptr + y * buffer->rowPixels));
    }

    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);

    return true;
}

// Bounce example rendering
void RenderBounceExample(GraphicsBuffer* buffer)
{
    ClearBuffer(buffer, MakeColor(135, 206, 235)); // Sky blue background

    // Draw title
    DrawText(buffer, COLOR_WHITE, 10, 10, "BOUNCE - Press Q to Quit, R to Restart");

    // Draw multiple bouncing balls at different positions
    static const struct {
        int x, y, r;
        Pixel color;
    } balls[] = {
        {150, 200, 25, 0xFFFF0000}, // Red
        {400, 300, 30, 0xFF00FF00}, // Green
        {250, 150, 20, 0xFF0000FF}, // Blue
        {550, 250, 35, 0xFFFFFF00}, // Yellow
        {350, 400, 28, 0xFFFF00FF}, // Magenta
    };

    for (int i = 0; i < 5; i++) {
        // Draw shadow
        Pixel shadow = MakeColorWithAlpha(0, 0, 0, 64);
        FillCircle(buffer, shadow, balls[i].x + 5, balls[i].y + 5, balls[i].r);

        // Draw ball
        FillCircle(buffer, balls[i].color, balls[i].x, balls[i].y, balls[i].r);

        // Draw highlight
        FillCircle(buffer, MakeColorWithAlpha(255, 255, 255, 128),
                   balls[i].x - balls[i].r/3, balls[i].y - balls[i].r/3, balls[i].r/4);
    }

    // Draw ground line
    DrawLine(buffer, MakeColor(100, 100, 100), 0, 500, buffer->width, 500);
}

// Fern example rendering
void RenderFernExample(GraphicsBuffer* buffer)
{
    ClearBuffer(buffer, COLOR_BLACK);

    DrawText(buffer, COLOR_WHITE, 10, 10, "BARNSLEY FERN - Press Q to Quit");

    // Barnsley fern using iterated function system
    const int MAX_ITER = 100000;
    double x = 0.0, y = 0.0;

    srand(42); // Fixed seed for consistent output

    for (int i = 0; i < MAX_ITER; ++i) {
        double r = (double)rand() / RAND_MAX;
        double xn, yn;

        if (r < 0.01) {
            // Stem
            xn = 0.0;
            yn = 0.16 * y;
        } else if (r < 0.86) {
            // Successively smaller leaflets
            xn = 0.85 * x + 0.04 * y;
            yn = -0.04 * x + 0.85 * y + 1.6;
        } else if (r < 0.93) {
            // Largest left-hand leaflet
            xn = 0.2 * x - 0.26 * y;
            yn = 0.23 * x + 0.22 * y + 1.6;
        } else {
            // Largest right-hand leaflet
            xn = -0.15 * x + 0.28 * y;
            yn = 0.26 * x + 0.24 * y + 0.44;
        }

        x = xn;
        y = yn;

        // Map to screen coordinates
        int px = (int)(buffer->width / 2 + x * 60);
        int py = (int)(buffer->height - 50 - y * 60);

        if (px >= 0 && px < buffer->width && py >= 0 && py < buffer->height) {
            // Gradient color based on iteration
            RGBColor24 rgb = {
                120 + 90 * i / MAX_ITER,
                255 - 50 * i / MAX_ITER,
                90 + 160 * i / MAX_ITER
            };
            Pixel color = AsPixel(rgb);
            PutPixel(buffer, color, px, py);
        }
    }
}

// Text demo example rendering
void RenderTextExample(GraphicsBuffer* buffer)
{
    ClearBuffer(buffer, MakeColor(40, 40, 60));

    DrawText(buffer, COLOR_WHITE, 10, 10, "FINCH TEXT RENDERING DEMO");
    DrawText(buffer, COLOR_CYAN, 10, 30, "Press Start 2P Font - 8x8 Arcade Style");

    DrawText(buffer, COLOR_YELLOW, 10, 70, "Color Examples:");
    DrawText(buffer, COLOR_RED, 30, 90, "Red Text");
    DrawText(buffer, COLOR_GREEN, 30, 110, "Green Text");
    DrawText(buffer, COLOR_BLUE, 30, 130, "Blue Text");
    DrawText(buffer, COLOR_MAGENTA, 30, 150, "Magenta Text");

    DrawText(buffer, COLOR_YELLOW, 10, 190, "Character Set:");
    DrawText(buffer, COLOR_WHITE, 30, 210, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    DrawText(buffer, COLOR_WHITE, 30, 230, "abcdefghijklmnopqrstuvwxyz");
    DrawText(buffer, COLOR_WHITE, 30, 250, "0123456789");
    DrawText(buffer, COLOR_WHITE, 30, 270, "!@#$%^&*()_+-=[]{}\\|;:'\",.<>?/");

    DrawText(buffer, COLOR_YELLOW, 10, 310, "Multiline Text:");
    DrawText(buffer, COLOR_LIGHT_GRAY, 30, 330, "This is line 1");
    DrawText(buffer, COLOR_LIGHT_GRAY, 30, 350, "This is line 2");
    DrawText(buffer, COLOR_LIGHT_GRAY, 30, 370, "This is line 3");

    DrawText(buffer, COLOR_GREEN, 10, 420, "Perfect for retro games!");
    DrawText(buffer, COLOR_CYAN, 10, 460, "Score: 12345  Lives: 3  Level: 8");
}

int main(void)
{
    const int width = 800;
    const int height = 600;

    // Generate bounce example screenshot
    GraphicsBuffer* buffer = NewGraphBuffer(NULL, width, height, width, width * height * sizeof(Pixel));
    if (buffer) {
        printf("Generating bounce.png...\n");
        RenderBounceExample(buffer);
        if (SavePNG("docs/images/example_bounce.png", buffer)) {
            printf("✓ Saved docs/images/example_bounce.png\n");
        }
        DeleteGraphBuffer(buffer);
    }

    // Generate fern example screenshot
    buffer = NewGraphBuffer(NULL, width, height, width, width * height * sizeof(Pixel));
    if (buffer) {
        printf("Generating fern.png...\n");
        RenderFernExample(buffer);
        if (SavePNG("docs/images/example_fern.png", buffer)) {
            printf("✓ Saved docs/images/example_fern.png\n");
        }
        DeleteGraphBuffer(buffer);
    }

    // Generate text demo screenshot
    buffer = NewGraphBuffer(NULL, width, height, width, width * height * sizeof(Pixel));
    if (buffer) {
        printf("Generating text_demo.png...\n");
        RenderTextExample(buffer);
        if (SavePNG("docs/images/example_text_demo.png", buffer)) {
            printf("✓ Saved docs/images/example_text_demo.png\n");
        }
        DeleteGraphBuffer(buffer);
    }

    printf("\n✓ All screenshots generated successfully!\n");
    return 0;
}
