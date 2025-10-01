/**
 * Barnsley Fern Demo
 *
 * Demonstrates:
 * - Mathematical visualization using chaos theory
 * - Iterated function systems (IFS)
 * - Point plotting
 * - Gradient coloring
 *
 * Controls:
 * - Press 'q' or ESC to quit
 *
 * This example uses the Barnsley Fern algorithm to create a beautiful
 * fractal that resembles a natural fern leaf. The algorithm uses four
 * affine transformations chosen randomly to plot each point.
 *
 * The fern is created by repeatedly applying one of four transformations
 * to a point (x,y):
 *
 * 1% of time:  Stem (moves to origin)
 * 85% of time: Main frond (largest part of fern)
 * 7% of time:  Left leaflet
 * 7% of time:  Right leaflet
 *
 * Over thousands of iterations, this creates a detailed fern shape.
 */

#include "finch.h"
#include "input_events.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

const int WIN_WIDTH = 1280;
const int WIN_HEIGHT = 1024;

#define FERN_SEED 789234
#define MAX_ITER 100000  // Increased for more detail

bool gDone = false;
bool gNeedsRedraw = true;

// Draw the Barnsley Fern using iterated function system
static void DrawFern(GraphicsBuffer *screen)
{
    ClearBuffer(screen, COLOR_BLACK);

    srand(FERN_SEED);

    double x = 0.0, y = 0.0;

    for (int i = 0; i < MAX_ITER; ++i) {
        double r = (double)rand() / RAND_MAX;
        double xn, yn;

        // Choose transformation based on probability
        if (r < 0.01) {
            // Stem - 1% probability
            xn = 0.0;
            yn = 0.16 * y;
        } else if (r < 0.86) {
            // Main frond - 85% probability
            xn = 0.85 * x + 0.04 * y;
            yn = -0.04 * x + 0.85 * y + 1.6;
        } else if (r < 0.93) {
            // Left leaflet - 7% probability
            xn = 0.2 * x - 0.26 * y;
            yn = 0.23 * x + 0.22 * y + 1.6;
        } else {
            // Right leaflet - 7% probability
            xn = -0.15 * x + 0.28 * y;
            yn = 0.26 * x + 0.24 * y + 0.44;
        }

        // Create color gradient - darker green to lighter as fern grows
        RGBColor24 rgb = {
            120 + 90 * i / MAX_ITER,   // Slight red tint
            255 - 50 * i / MAX_ITER,   // Green (high, slightly decreasing)
            90 + 160 * i / MAX_ITER    // Blue (increasing for lighter shade)
        };
        Pixel color = AsPixel(rgb);

        // Transform mathematical coordinates to screen coordinates
        // The fern is centered and scaled to fit nicely on screen
        int px = (int)(480 + xn * 60);
        int py = (int)(720 - yn * 60);  // Flip Y axis (screen Y grows down)

        PutPixel(screen, color, px, py);

        x = xn;
        y = yn;
    }
}

// Finch engine callbacks

bool FinchMain(int argc, const char* argv[], void** userData) {
    *userData = NULL;
    return FinchStartGraphics(WIN_WIDTH, WIN_HEIGHT);
}

bool FinchInit(int width, int height, void* userData) {
    return true;
}

void FinchCleanup(void *userData) {
    // Nothing to clean up
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void *userData)
{
    GraphicsBuffer *screen = NewGraphBuffer(pixels, width, height, width, 0);
    if (!screen) {
        fprintf(stderr, "Error creating graphics buffer\n");
        return;
    }

    // Only draw once - it's computationally expensive
    if (gNeedsRedraw) {
        gNeedsRedraw = false;
        printf("Rendering Barnsley Fern (%d iterations)...\n", MAX_ITER);
        DrawFern(screen);
        printf("Done!\n");
    }

    DeleteGraphBuffer(screen);
}

void FinchUpdate(void* userData, double elapsedTicks)
{
    // No animation - static image
}

bool FinchDone(void* userData)
{
    return gDone;
}

void FinchHandleEvent(InputEvent* event, void* userData)
{
    if (event->eventType == kInputEventType_Quit ||
        (event->eventType == kInputEventType_KeyDown &&
         (event->keyCode == 'q' || event->keyCode == 27))) {
        gDone = true;
    }
}
