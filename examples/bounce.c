/**
 * Bouncing Balls Demo
 *
 * Demonstrates:
 * - Physics simulation (velocity, collision detection)
 * - Drawing filled circles
 * - Animation with frame timing
 * - Random number generation
 * - Event handling (keyboard input)
 *
 * Controls:
 * - Press 'r' to restart with new random balls
 * - Press 'q' or ESC to quit
 *
 * This example shows how to create a simple physics simulation
 * with multiple objects bouncing around the screen.
 */

#include "finch.h"
#include "input_events.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Window and simulation constants
const int WIN_WIDTH = 800;
const int WIN_HEIGHT = 600;

const int NUM_BALLS = 7;
const int MAX_SPEED = 5;
const int MIN_R = 25;
const int MAX_R = 60;
const double FRAME_DELAY_TICKS = 15;

const int RAND_SEED = 789034;

// Ball structure - represents a bouncing ball
typedef struct {
    double r;      // radius
    double x, y;   // position
    double dx, dy; // velocity
    Pixel color;   // color
} Ball;

// Game state - all data needed for the simulation
typedef struct {
    bool done;
    Ball balls[NUM_BALLS];
    double currentTime, lastFrameTime;
    uint32_t frameCount;
} GameState;

// Color palette for balls
static const Pixel kColors[] = {
    0xFFFF0000, // Red
    0xFF00FF00, // Green
    0xFF0000FF, // Blue
    0xFFFFFF00, // Yellow
    0xFF00FFFF, // Cyan
    0xFFFF00FF, // Magenta
    0xFFFFFFFF, // White
    0xFF00A5E2, // Light blue
    0xFFFF6B35, // Orange
    0xFF7FB800, // Lime
};

#define NUM_COLORS (sizeof(kColors) / sizeof(kColors[0]))

// Random number in range [min, max)
static int RandRange(int min, int max) {
    int rv = rand();
    int result = min + rv % (max - min);
    return result;
}

// Initialize a ball with random position, velocity, and color
void NewBall(Ball* ball, int32_t screenWidth, int32_t screenHeight)
{
    ball->r = RandRange(MIN_R, MAX_R + 1);

    // Position - ensure ball starts fully on screen
    ball->x = RandRange(ball->r, screenWidth - ball->r);
    ball->y = RandRange(ball->r, screenHeight - ball->r);

    // Velocity - ensure non-zero movement
    int cnt = 0;
    do {
        ball->dx = RandRange(-MAX_SPEED, MAX_SPEED);
        ball->dy = RandRange(-MAX_SPEED, MAX_SPEED);
        cnt++;
    } while (ball->dx == 0 && ball->dy == 0 && cnt < 10);

    // Random color from palette
    ball->color = kColors[RandRange(0, NUM_COLORS)];
}

// Reset all balls to new random positions
void RestartBalls(GameState* state) {
    for (int i = 0; i < NUM_BALLS; i++) {
        NewBall(&state->balls[i], WIN_WIDTH, WIN_HEIGHT);
    }
}

// Draw a single ball
void DrawBall(GraphicsBuffer *screen, Ball* ball)
{
    // Draw filled circle with white outline
    FillCircle(screen, ball->color, ball->x, ball->y, ball->r);
    DrawCircle(screen, COLOR_WHITE, ball->x, ball->y, ball->r);
}

// Update ball position and handle wall collisions
void MoveBall(Ball* ball, int32_t screenWidth, int32_t screenHeight)
{
    int32_t oldx = ball->x;
    int32_t oldy = ball->y;

    // Bounce off walls by reversing velocity
    if (oldx + ball->r >= screenWidth || oldx - ball->r < 0) {
        ball->dx = -ball->dx;
    }
    if (oldy + ball->r >= screenHeight || oldy - ball->r < 0) {
        ball->dy = -ball->dy;
    }

    // Update position
    ball->x = oldx + ball->dx;
    ball->y = oldy + ball->dy;
}

// Draw all balls on screen
static void DrawBalls(GraphicsBuffer *screen, GameState* state) {
    // Clear to black
    ClearBuffer(screen, COLOR_BLACK);

    // Draw each ball
    for (int j = 0; j < NUM_BALLS; j++) {
        DrawBall(screen, &state->balls[j]);
    }
}

// Finch engine callbacks

bool FinchMain(int argc, const char* argv[], void** userData) {
    srand(RAND_SEED);

    GameState* state = malloc(sizeof(GameState));
    if (!state) return false;

    state->done = false;
    state->currentTime = 0;
    state->lastFrameTime = 0;
    state->frameCount = 0;

    RestartBalls(state);

    *userData = state;
    return FinchStartGraphics(WIN_WIDTH, WIN_HEIGHT);
}

bool FinchInit(int width, int height, void* userData) {
    return true;
}

void FinchCleanup(void *userData) {
    free(userData);
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void *userData) {
    GameState* state = (GameState*)userData;
    if (!state) return;

    GraphicsBuffer *screen = NewGraphBuffer(pixels, width, height, width, 0);
    if (!screen) {
        fprintf(stderr, "Error creating graphics buffer\n");
        return;
    }

    DrawBalls(screen, state);
    DeleteGraphBuffer(screen);
}

void FinchUpdate(void* userData, double elapsedTicks) {
    GameState* state = (GameState*)userData;
    if (!state) return;

    state->currentTime += elapsedTicks;

    // Update physics at fixed rate
    if (state->currentTime - state->lastFrameTime >= FRAME_DELAY_TICKS) {
        for (int j = 0; j < NUM_BALLS; j++) {
            MoveBall(&state->balls[j], WIN_WIDTH, WIN_HEIGHT);
        }

        ++state->frameCount;
        state->lastFrameTime = state->currentTime;
    }
}

bool FinchDone(void* userData) {
    GameState* state = (GameState*)userData;
    return state ? state->done : false;
}

void FinchHandleEvent(InputEvent* event, void* userData) {
    GameState* state = (GameState*)userData;
    if (!state) return;

    // Quit on 'q' or ESC or window close
    if (event->eventType == kInputEventType_Quit ||
        (event->eventType == kInputEventType_KeyDown &&
         (event->keyCode == 'q' || event->keyCode == 27))) {
        state->done = true;
        return;
    }

    // Restart on 'r'
    if (event->eventType == kInputEventType_KeyDown && event->keyCode == 'r') {
        RestartBalls(state);
    }
}
