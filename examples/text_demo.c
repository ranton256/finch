/**
 * Text Rendering Demo
 *
 * Demonstrates Finch's text rendering capabilities:
 * - Drawing text at specific positions
 * - Centered text
 * - Displaying FPS counter
 * - Showing mouse coordinates
 * - Multiple colors
 *
 * Controls:
 * - Move mouse to see coordinates update
 * - Press 'q' or ESC to quit
 */

#include "finch.h"
#include "input_events.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

const int WIN_WIDTH = 800;
const int WIN_HEIGHT = 600;

typedef struct {
    int mouseX, mouseY;
    double totalTime;
    int frameCount;
    double fps;
} DemoState;

bool gDone = false;

bool FinchMain(int argc, const char* argv[], void** userData) {
    DemoState* state = malloc(sizeof(DemoState));
    state->mouseX = WIN_WIDTH / 2;
    state->mouseY = WIN_HEIGHT / 2;
    state->totalTime = 0;
    state->frameCount = 0;
    state->fps = 0;
    *userData = state;
    return FinchStartGraphics(WIN_WIDTH, WIN_HEIGHT);
}

bool FinchInit(int width, int height, void* userData) {
    return true;
}

void FinchUpdate(void* userData, double elapsedTicks) {
    DemoState* state = (DemoState*)userData;

    state->totalTime += elapsedTicks;
    state->frameCount++;

    // Update FPS every second
    if (state->totalTime >= 1000.0) {
        state->fps = state->frameCount / (state->totalTime / 1000.0);
        state->frameCount = 0;
        state->totalTime = 0;
    }
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    DemoState* state = (DemoState*)userData;
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    // Clear screen
    ClearBuffer(screen, COLOR_BLACK);

    // Title - centered at top
    DrawTextCentered(screen, COLOR_WHITE, WIN_WIDTH / 2, 30, "FINCH TEXT RENDERING DEMO");

    // Draw some example text in different colors
    DrawText(screen, COLOR_RED, 50, 80, "Red text at fixed position");
    DrawText(screen, COLOR_GREEN, 50, 100, "Green text - 8x8 bitmap font");
    DrawText(screen, COLOR_BLUE, 50, 120, "Blue text - ASCII 32-126 supported");
    DrawText(screen, COLOR_YELLOW, 50, 140, "Yellow text");
    DrawText(screen, COLOR_CYAN, 50, 160, "Cyan text");
    DrawText(screen, COLOR_MAGENTA, 50, 180, "Magenta text");

    // FPS counter - top right
    char fpsText[32];
    snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", state->fps);
    int fpsWidth = GetTextWidth(fpsText);
    DrawText(screen, COLOR_LIGHT_GRAY, WIN_WIDTH - fpsWidth - 10, 10, fpsText);

    // Mouse coordinates - follow mouse
    char mouseText[64];
    snprintf(mouseText, sizeof(mouseText), "Mouse: (%d, %d)", state->mouseX, state->mouseY);
    DrawText(screen, COLOR_WHITE, state->mouseX + 20, state->mouseY - 10, mouseText);

    // Draw crosshair at mouse position
    DrawLine(screen, COLOR_GRAY, state->mouseX - 10, state->mouseY,
             state->mouseX + 10, state->mouseY);
    DrawLine(screen, COLOR_GRAY, state->mouseX, state->mouseY - 10,
             state->mouseX, state->mouseY + 10);

    // Character set demo - bottom half
    DrawText(screen, COLOR_WHITE, 50, 250, "Character Set:");

    // Draw all printable ASCII characters
    int x = 50;
    int y = 280;
    for (char c = 32; c <= 126; c++) {
        DrawChar(screen, COLOR_LIGHT_GRAY, x, y, c);
        x += 8;
        if (x > WIN_WIDTH - 100) {
            x = 50;
            y += 10;
        }
    }

    // Instructions - bottom
    DrawTextCentered(screen, COLOR_DARK_GRAY, WIN_WIDTH / 2, WIN_HEIGHT - 30,
                     "Move mouse around - Press 'q' or ESC to quit");

    // Centered message example
    DrawTextCentered(screen, COLOR_GREEN, WIN_WIDTH / 2, WIN_HEIGHT / 2,
                     "Centered Text Example");

    // Box around centered text
    int boxTextWidth = GetTextWidth("Centered Text Example");
    int boxTextHeight = GetTextHeight();
    DrawRect(screen, COLOR_GREEN,
             WIN_WIDTH / 2 - boxTextWidth / 2 - 5,
             WIN_HEIGHT / 2 - boxTextHeight / 2 - 3,
             WIN_WIDTH / 2 + boxTextWidth / 2 + 5,
             WIN_HEIGHT / 2 + boxTextHeight / 2 + 3);

    DeleteGraphBuffer(screen);
}

void FinchHandleEvent(InputEvent* event, void* userData) {
    DemoState* state = (DemoState*)userData;

    if (event->eventType == kInputEventType_MouseMove) {
        state->mouseX = event->x;
        state->mouseY = event->y;
    }

    if (event->eventType == kInputEventType_KeyDown) {
        if (event->keyCode == 'q' || event->keyCode == 27) { // 'q' or ESC
            gDone = true;
        }
    }

    if (event->eventType == kInputEventType_Quit) {
        gDone = true;
    }
}

bool FinchDone(void* userData) {
    return gDone;
}

void FinchCleanup(void* userData) {
    free(userData);
}
