# Finch Graphics Library Tutorial

Welcome! This tutorial will teach you how to create 2D graphics programs and games using the Finch library. Finch is a simple C library that wraps SDL2, making it easy to draw shapes, handle input, and create interactive graphics applications.

## Table of Contents

1. [Your First Finch Program](#1-your-first-finch-program)
2. [Understanding the Callback Model](#2-understanding-the-callback-model)
3. [Drawing Basics](#3-drawing-basics)
4. [Color and Transparency](#4-color-and-transparency)
5. [Interactive Graphics](#5-interactive-graphics)
6. [Animation](#6-animation)
7. [Building Your First Game](#7-building-your-first-game)

---

## 1. Your First Finch Program

Let's start with the simplest possible Finch program that displays a single pixel.

### Minimal Program Structure

Every Finch program needs to implement these callback functions:

```c
#include "finch.h"
#include <stdbool.h>

// Global flag to control when to quit
bool gDone = false;

// Entry point - called first
bool FinchMain(int argc, const char* argv[], void** userData) {
    // Start graphics with 800x600 window
    return FinchStartGraphics(800, 600);
}

// Initialization - called once after window is created
bool FinchInit(int width, int height, void* userData) {
    return true; // Return true for success
}

// Rendering - called every frame to draw
void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    // Wrap the screen pixels in a GraphicsBuffer
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    // Clear screen to black
    ClearBuffer(screen, COLOR_BLACK);

    // Draw a single red pixel at (400, 300)
    PutPixel(screen, COLOR_RED, 400, 300);

    // Clean up (only the wrapper, not the pixels!)
    DeleteGraphBuffer(screen);
}

// Update - called every frame for game logic
void FinchUpdate(void* userData, double elapsedTicks) {
    // Nothing to update yet
}

// Event handling - called for keyboard/mouse events
void FinchHandleEvent(InputEvent* event, void* userData) {
    if (event->eventType == kInputEventType_KeyDown && event->keyCode == 'q') {
        gDone = true;
    }
    if (event->eventType == kInputEventType_Quit) {
        gDone = true;
    }
}

// Check if we should quit
bool FinchDone(void* userData) {
    return gDone;
}

// Cleanup - called once before exit
void FinchCleanup(void* userData) {
    // Nothing to clean up yet
}
```

### Compiling and Running

```bash
# Compile your program
gcc -o myprogram myprogram.c -lfinch $(sdl2-config --cflags --libs) -lSDL2_mixer -lpng

# Run it
./myprogram
```

Press 'q' to quit the program.

**What You Learned:**
- The basic structure of a Finch program
- How to create a window
- How to draw a single pixel
- How to handle keyboard input to quit

---

## 2. Understanding the Callback Model

Finch uses a **callback-based architecture**. You implement specific functions that Finch calls at the right times. Understanding when each callback is called is crucial.

### Callback Execution Order

```
1. FinchMain()
   ↓
   Call FinchStartGraphics() to create window
   ↓
2. FinchInit() - called once after window creation
   ↓
3. MAIN LOOP (repeats until FinchDone() returns true)
   │
   ├─→ FinchHandleEvent() - called for each input event
   │
   ├─→ FinchUpdate() - called once per frame
   │
   ├─→ FinchRenderProc() - called once per frame
   │
   └─→ FinchDone() - checked to see if we should quit
   ↓
4. FinchCleanup() - called once before exit
```

### GraphicsBuffer Management

**IMPORTANT:** In `FinchRenderProc()`, the `pixels` parameter is the screen buffer owned by SDL2.

**Do:**
- ✅ Create a GraphicsBuffer wrapper: `GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);`
- ✅ Use the wrapper for drawing: `PutPixel(screen, COLOR_RED, x, y);`
- ✅ Delete the wrapper when done: `DeleteGraphBuffer(screen);`

**Don't:**
- ❌ Don't free the `pixels` array (SDL2 owns it)
- ❌ Don't keep the GraphicsBuffer across frames (recreate each frame)

### User Data Pattern

The `userData` parameter lets you pass custom data through all callbacks:

```c
typedef struct {
    int playerX, playerY;
    int score;
} GameState;

bool FinchMain(int argc, const char* argv[], void** userData) {
    // Allocate game state
    GameState* state = malloc(sizeof(GameState));
    state->playerX = 400;
    state->playerY = 300;
    state->score = 0;
    *userData = state; // Store for later callbacks

    return FinchStartGraphics(800, 600);
}

void FinchUpdate(void* userData, double elapsedTicks) {
    GameState* state = (GameState*)userData;
    state->playerX += 1; // Move player
}

void FinchCleanup(void* userData) {
    GameState* state = (GameState*)userData;
    free(state); // Don't forget to free!
}
```

---

## 3. Drawing Basics

Now let's draw various shapes and understand the coordinate system.

### Coordinate System

Finch uses standard screen coordinates:
```
(0,0) ────────→ X increases
  │
  │
  ↓
Y increases
```

- Origin (0,0) is at the **top-left** corner
- X axis increases to the **right**
- Y axis increases **downward**

### Drawing Shapes

```c
void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    // Clear to black background
    ClearBuffer(screen, COLOR_BLACK);

    // Draw individual pixels
    PutPixel(screen, COLOR_RED, 100, 100);
    PutPixel(screen, COLOR_GREEN, 200, 100);
    PutPixel(screen, COLOR_BLUE, 300, 100);

    // Draw lines
    DrawLine(screen, COLOR_WHITE, 50, 150, 350, 150);      // Horizontal
    DrawLine(screen, COLOR_YELLOW, 200, 50, 200, 250);     // Vertical
    DrawLine(screen, COLOR_CYAN, 50, 50, 350, 250);        // Diagonal

    // Draw rectangles
    DrawRect(screen, COLOR_MAGENTA, 400, 50, 550, 150);    // Outline
    FillRectOpaque(screen, COLOR_RED, 400, 200, 550, 300); // Filled

    // Draw circles
    DrawCircle(screen, COLOR_GREEN, 100, 400, 50);         // Outline
    FillCircle(screen, COLOR_BLUE, 250, 400, 50);          // Filled

    DeleteGraphBuffer(screen);
}
```

### Reading Pixels

You can also read pixel values:

```c
// Get the color at a specific location
Pixel color = GetPixel(screen, 100, 100);

// Extract color components
uint8_t red, green, blue;
PixelComponents(color, &red, &green, &blue);
printf("Color at (100,100): R=%d G=%d B=%d\n", red, green, blue);
```

---

## 4. Color and Transparency

Understanding colors and alpha blending is essential for creating rich graphics.

### Color Format

Finch uses 32-bit RGBA colors in `0xAARRGGBB` hexadecimal format:
- **AA** = Alpha (transparency): 0=transparent, 255=opaque
- **RR** = Red channel (0-255)
- **GG** = Green channel (0-255)
- **BB** = Blue channel (0-255)

### Creating Colors

```c
// Method 1: Use predefined constants
Pixel red = COLOR_RED;
Pixel blue = COLOR_BLUE;

// Method 2: Create from RGB components (opaque)
Pixel MakeColor(uint8_t r, uint8_t g, uint8_t b);
Pixel orange = MakeColor(255, 165, 0);

// Method 3: Create with custom alpha
Pixel MakeColorWithAlpha(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
Pixel semiRed = MakeColorWithAlpha(255, 0, 0, 128); // 50% transparent red

// Method 4: Direct RGBA
Pixel LSRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
Pixel semiBlue = LSRGBA(0, 0, 255, 128);
```

### Available Color Constants

```c
COLOR_WHITE
COLOR_BLACK
COLOR_RED
COLOR_GREEN
COLOR_BLUE
COLOR_YELLOW      // Red + Green
COLOR_CYAN        // Green + Blue
COLOR_MAGENTA     // Red + Blue
COLOR_GRAY
COLOR_DARK_GRAY
COLOR_LIGHT_GRAY
```

### Alpha Blending (Transparency)

Some functions support alpha blending:

```c
void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    // Draw solid blue circle
    FillCircle(screen, COLOR_BLUE, 300, 300, 100);

    // Draw semi-transparent red circle overlapping it
    Pixel semiRed = MakeColorWithAlpha(255, 0, 0, 128);

    // DrawLineComposite blends instead of overwriting
    DrawLineComposite(screen, semiRed, 200, 200, 400, 400);

    // You can also blit transparent sprites
    GraphicsBuffer* sprite = NewGraphBuffer(NULL, 64, 64, 64, 64*64*sizeof(Pixel));
    // ... draw into sprite ...
    BlitGraphBufferComposite(sprite, screen, 100, 100); // Blends sprite onto screen
    DeleteGraphBuffer(sprite);

    DeleteGraphBuffer(screen);
}
```

**Functions that support transparency:**
- `DrawLineComposite()`
- `DrawHorzLine()` and `DrawVertLine()` (use compositing)
- `BlitGraphBufferComposite()`

**Functions that are opaque (ignore alpha):**
- `DrawLine()` (unless you use `DrawLineComposite`)
- `FillRectOpaque()`
- `BlitGraphBuffer()`

---

## 5. Interactive Graphics

Now let's make programs respond to user input.

### Handling Keyboard Input

```c
void FinchHandleEvent(InputEvent* event, void* userData) {
    if (event->eventType == kInputEventType_KeyDown) {
        // Check which key was pressed
        if (event->keyCode == 'w') {
            // W key pressed
        }
        if (event->keyCode == 's') {
            // S key pressed
        }
        if (event->keyCode == 'a') {
            // A key pressed
        }
        if (event->keyCode == 'd') {
            // D key pressed
        }

        // Special keys
        if (event->keyCode == 27) {  // ESC key
            gDone = true;
        }
    }

    if (event->eventType == kInputEventType_KeyUp) {
        // Key was released
    }

    if (event->eventType == kInputEventType_Quit) {
        // Window close button clicked
        gDone = true;
    }
}
```

### Handling Mouse Input

```c
typedef struct {
    int mouseX, mouseY;
    bool isDrawing;
} PaintState;

void FinchHandleEvent(InputEvent* event, void* userData) {
    PaintState* state = (PaintState*)userData;

    if (event->eventType == kInputEventType_MouseMove) {
        state->mouseX = event->x;
        state->mouseY = event->y;
    }

    if (event->eventType == kInputEventType_MouseButtonDown) {
        if (event->button == 1) {  // Left button
            state->isDrawing = true;
        }
    }

    if (event->eventType == kInputEventType_MouseButtonUp) {
        if (event->button == 1) {  // Left button
            state->isDrawing = false;
        }
    }
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    PaintState* state = (PaintState*)userData;
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    // Draw crosshair at mouse position
    DrawLine(screen, COLOR_WHITE, state->mouseX - 10, state->mouseY,
             state->mouseX + 10, state->mouseY);
    DrawLine(screen, COLOR_WHITE, state->mouseX, state->mouseY - 10,
             state->mouseX, state->mouseY + 10);

    // If mouse button is down, draw
    if (state->isDrawing) {
        FillCircle(screen, COLOR_RED, state->mouseX, state->mouseY, 5);
    }

    DeleteGraphBuffer(screen);
}
```

### Complete Interactive Example

```c
#include "finch.h"
#include "input_events.h"
#include <stdbool.h>

typedef struct {
    int circleX, circleY;
} GameState;

bool gDone = false;

bool FinchMain(int argc, const char* argv[], void** userData) {
    GameState* state = malloc(sizeof(GameState));
    state->circleX = 400;
    state->circleY = 300;
    *userData = state;
    return FinchStartGraphics(800, 600);
}

bool FinchInit(int width, int height, void* userData) {
    return true;
}

void FinchHandleEvent(InputEvent* event, void* userData) {
    GameState* state = (GameState*)userData;

    if (event->eventType == kInputEventType_KeyDown) {
        // Move circle with arrow keys (or WASD)
        if (event->keyCode == 'w') state->circleY -= 10;
        if (event->keyCode == 's') state->circleY += 10;
        if (event->keyCode == 'a') state->circleX -= 10;
        if (event->keyCode == 'd') state->circleX += 10;

        if (event->keyCode == 'q' || event->keyCode == 27) {
            gDone = true;
        }
    }

    if (event->eventType == kInputEventType_Quit) {
        gDone = true;
    }
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    GameState* state = (GameState*)userData;
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    ClearBuffer(screen, COLOR_BLACK);

    // Draw the movable circle
    FillCircle(screen, COLOR_CYAN, state->circleX, state->circleY, 30);

    DeleteGraphBuffer(screen);
}

void FinchUpdate(void* userData, double elapsedTicks) {
    // Nothing to update for this simple example
}

bool FinchDone(void* userData) {
    return gDone;
}

void FinchCleanup(void* userData) {
    free(userData);
}
```

---

## 6. Animation

Animation is achieved by updating positions in `FinchUpdate()` and drawing in `FinchRenderProc()`.

### Basic Animation Pattern

```c
typedef struct {
    double ballX, ballY;      // Position (use double for smooth movement)
    double ballDX, ballDY;    // Velocity
    double ballRadius;
} AnimationState;

void FinchUpdate(void* userData, double elapsedTicks) {
    AnimationState* state = (AnimationState*)userData;

    // Convert ticks to seconds (elapsedTicks is in milliseconds)
    double dt = elapsedTicks / 1000.0;

    // Update position based on velocity
    state->ballX += state->ballDX * dt;
    state->ballY += state->ballDY * dt;

    // Bounce off walls
    if (state->ballX - state->ballRadius < 0 ||
        state->ballX + state->ballRadius > 800) {
        state->ballDX = -state->ballDX;  // Reverse direction
    }

    if (state->ballY - state->ballRadius < 0 ||
        state->ballY + state->ballRadius > 600) {
        state->ballDY = -state->ballDY;
    }
}
```

### Bouncing Ball Example

```c
#include "finch.h"
#include "input_events.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    double x, y;          // Position
    double dx, dy;        // Velocity (pixels per second)
    int radius;
    int windowWidth, windowHeight;
} Ball;

bool gDone = false;

bool FinchMain(int argc, const char* argv[], void** userData) {
    Ball* ball = malloc(sizeof(Ball));
    ball->x = 400;
    ball->y = 300;
    ball->dx = 200;   // 200 pixels/second to the right
    ball->dy = 150;   // 150 pixels/second downward
    ball->radius = 20;
    *userData = ball;
    return FinchStartGraphics(800, 600);
}

bool FinchInit(int width, int height, void* userData) {
    Ball* ball = (Ball*)userData;
    ball->windowWidth = width;
    ball->windowHeight = height;
    return true;
}

void FinchUpdate(void* userData, double elapsedTicks) {
    Ball* ball = (Ball*)userData;

    // Time step in seconds
    double dt = elapsedTicks / 1000.0;

    // Update position
    ball->x += ball->dx * dt;
    ball->y += ball->dy * dt;

    // Bounce off left/right walls
    if (ball->x - ball->radius <= 0) {
        ball->x = ball->radius;
        ball->dx = -ball->dx;
    }
    if (ball->x + ball->radius >= ball->windowWidth) {
        ball->x = ball->windowWidth - ball->radius;
        ball->dx = -ball->dx;
    }

    // Bounce off top/bottom walls
    if (ball->y - ball->radius <= 0) {
        ball->y = ball->radius;
        ball->dy = -ball->dy;
    }
    if (ball->y + ball->radius >= ball->windowHeight) {
        ball->y = ball->windowHeight - ball->radius;
        ball->dy = -ball->dy;
    }
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    Ball* ball = (Ball*)userData;
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    ClearBuffer(screen, COLOR_BLACK);

    // Draw the ball
    FillCircle(screen, COLOR_YELLOW, (int)ball->x, (int)ball->y, ball->radius);

    DeleteGraphBuffer(screen);
}

void FinchHandleEvent(InputEvent* event, void* userData) {
    if (event->eventType == kInputEventType_Quit ||
        (event->eventType == kInputEventType_KeyDown && event->keyCode == 'q')) {
        gDone = true;
    }
}

bool FinchDone(void* userData) {
    return gDone;
}

void FinchCleanup(void* userData) {
    free(userData);
}
```

### Adding Gravity

```c
void FinchUpdate(void* userData, double elapsedTicks) {
    Ball* ball = (Ball*)userData;
    double dt = elapsedTicks / 1000.0;

    const double GRAVITY = 500.0;  // Pixels per second squared

    // Apply gravity to vertical velocity
    ball->dy += GRAVITY * dt;

    // Update position
    ball->x += ball->dx * dt;
    ball->y += ball->dy * dt;

    // Bounce off floor with energy loss
    if (ball->y + ball->radius >= ball->windowHeight) {
        ball->y = ball->windowHeight - ball->radius;
        ball->dy = -ball->dy * 0.8;  // Lose 20% energy on bounce
    }

    // Bounce off walls
    if (ball->x - ball->radius <= 0 || ball->x + ball->radius >= ball->windowWidth) {
        ball->dx = -ball->dx;
    }
}
```

---

## 7. Building Your First Game

Let's build a simple paddle game (simplified Pong).

### Game Design

- Ball bounces around the screen
- Player controls paddle with mouse
- Ball bounces off paddle
- Game over if ball reaches bottom

### Complete Game Code

```c
#include "finch.h"
#include "input_events.h"
#include <stdbool.h>
#include <stdlib.h>

const int WIN_WIDTH = 800;
const int WIN_HEIGHT = 600;

typedef struct {
    // Ball
    double ballX, ballY;
    double ballDX, ballDY;
    int ballRadius;

    // Paddle
    int paddleX;
    int paddleWidth;
    int paddleHeight;
    int paddleY;

    // Game state
    bool gameOver;
    int score;
} GameState;

bool gDone = false;

bool FinchMain(int argc, const char* argv[], void** userData) {
    GameState* state = malloc(sizeof(GameState));

    // Initialize ball
    state->ballX = WIN_WIDTH / 2;
    state->ballY = WIN_HEIGHT / 2;
    state->ballDX = 200;
    state->ballDY = 200;
    state->ballRadius = 10;

    // Initialize paddle
    state->paddleX = WIN_WIDTH / 2;
    state->paddleWidth = 100;
    state->paddleHeight = 20;
    state->paddleY = WIN_HEIGHT - 50;

    // Initialize game state
    state->gameOver = false;
    state->score = 0;

    *userData = state;
    return FinchStartGraphics(WIN_WIDTH, WIN_HEIGHT);
}

bool FinchInit(int width, int height, void* userData) {
    return true;
}

void FinchUpdate(void* userData, double elapsedTicks) {
    GameState* state = (GameState*)userData;

    if (state->gameOver) return;  // Don't update if game over

    double dt = elapsedTicks / 1000.0;

    // Update ball position
    state->ballX += state->ballDX * dt;
    state->ballY += state->ballDY * dt;

    // Bounce off left/right walls
    if (state->ballX - state->ballRadius <= 0 ||
        state->ballX + state->ballRadius >= WIN_WIDTH) {
        state->ballDX = -state->ballDX;
    }

    // Bounce off top wall
    if (state->ballY - state->ballRadius <= 0) {
        state->ballDY = -state->ballDY;
    }

    // Check collision with paddle
    if (state->ballY + state->ballRadius >= state->paddleY &&
        state->ballX >= state->paddleX - state->paddleWidth/2 &&
        state->ballX <= state->paddleX + state->paddleWidth/2) {

        state->ballDY = -state->ballDY;
        state->score++;
    }

    // Check if ball went off bottom (game over)
    if (state->ballY - state->ballRadius > WIN_HEIGHT) {
        state->gameOver = true;
    }
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    GameState* state = (GameState*)userData;
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);

    // Clear screen
    ClearBuffer(screen, COLOR_BLACK);

    if (!state->gameOver) {
        // Draw ball
        FillCircle(screen, COLOR_WHITE, (int)state->ballX, (int)state->ballY,
                   state->ballRadius);

        // Draw paddle
        FillRectOpaque(screen, COLOR_CYAN,
                       state->paddleX - state->paddleWidth/2,
                       state->paddleY,
                       state->paddleX + state->paddleWidth/2,
                       state->paddleY + state->paddleHeight);
    } else {
        // Draw "GAME OVER" (using rectangles to form letters - simple example)
        // In a real game, you'd use text rendering
        FillRectOpaque(screen, COLOR_RED, 350, 250, 450, 350);
    }

    DeleteGraphBuffer(screen);
}

void FinchHandleEvent(InputEvent* event, void* userData) {
    GameState* state = (GameState*)userData;

    if (event->eventType == kInputEventType_MouseMove) {
        // Move paddle with mouse
        state->paddleX = event->x;

        // Clamp to screen bounds
        if (state->paddleX - state->paddleWidth/2 < 0) {
            state->paddleX = state->paddleWidth/2;
        }
        if (state->paddleX + state->paddleWidth/2 > WIN_WIDTH) {
            state->paddleX = WIN_WIDTH - state->paddleWidth/2;
        }
    }

    if (event->eventType == kInputEventType_KeyDown) {
        // Press R to restart
        if (event->keyCode == 'r' && state->gameOver) {
            state->ballX = WIN_WIDTH / 2;
            state->ballY = WIN_HEIGHT / 2;
            state->ballDX = 200;
            state->ballDY = 200;
            state->gameOver = false;
            state->score = 0;
        }

        if (event->keyCode == 'q') {
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
```

---

## Next Steps

Congratulations! You now know the fundamentals of Finch graphics programming. Here are some ideas for expanding your skills:

### Beginner Projects
1. **Color Picker** - Click to change colors
2. **Simple Paint** - Draw with mouse, multiple colors
3. **Pattern Generator** - Create interesting visual patterns
4. **Screensaver** - Animated geometric shapes

### Intermediate Projects
5. **Breakout Clone** - Ball, paddle, and bricks
6. **Snake Game** - Growing snake that eats food
7. **Particle System** - Explosion effects
8. **Maze Generator** - Procedural maze drawing

### Advanced Projects
9. **Platformer** - Jump and run game
10. **Physics Simulation** - Realistic collisions
11. **Ray Tracer** - 3D-style rendering
12. **Cellular Automata** - Conway's Game of Life

### Learning Resources

- **finch.h** - Read the header file for complete API documentation
- **Example Programs** - Check the `examples/` directory (if available)
- **SDL2 Documentation** - For advanced features: https://wiki.libsdl.org/
- **Graphics Algorithms** - Learn about Bresenham's line algorithm, scanline fill, etc.

### Common Patterns

**Pattern 1: Drawing to Off-Screen Buffer**
```c
// Create sprite buffer
GraphicsBuffer* sprite = NewGraphBuffer(NULL, 64, 64, 64, 64*64*sizeof(Pixel));

// Draw into sprite
FillCircle(sprite, COLOR_RED, 32, 32, 30);

// Draw sprite onto screen
BlitGraphBuffer(sprite, screen, 100, 100);

// Clean up
DeleteGraphBuffer(sprite);
```

**Pattern 2: Double-Buffered Drawing**
```c
// Finch automatically double-buffers, so you don't see tearing
// Just draw normally in FinchRenderProc()
```

**Pattern 3: Frame-Rate Independent Movement**
```c
void FinchUpdate(void* userData, double elapsedTicks) {
    // Always use elapsedTicks for smooth movement
    double dt = elapsedTicks / 1000.0;
    playerX += playerVelocity * dt;  // Pixels per second
}
```

### Troubleshooting

**Problem: Nothing appears on screen**
- Did you call `ClearBuffer()` or draw anything?
- Did you create the GraphicsBuffer wrapper correctly?
- Did you forget to call `DeleteGraphBuffer()` at the end of FinchRenderProc()?

**Problem: Program crashes**
- Are you freeing the pixels array? (Don't! SDL2 owns it)
- Are you accessing userData before initializing it?
- Are you drawing outside buffer bounds? (Should be auto-clipped, but check)

**Problem: Movement is jerky**
- Are you using `elapsedTicks` for movement calculations?
- Convert to seconds: `dt = elapsedTicks / 1000.0`

**Problem: Colors look wrong**
- Remember: 0xAARRGGBB format (alpha, red, green, blue)
- Use `MakeColor()` or constants for simplicity

---

## Conclusion

You've learned:
- ✅ How to structure a Finch program
- ✅ The callback execution model
- ✅ Drawing shapes and handling colors
- ✅ Processing user input
- ✅ Creating animations
- ✅ Building a complete game

Finch gives you a simple foundation for 2D graphics programming in C. The best way to learn is by building projects. Start small, experiment often, and have fun!

Happy coding! 🎨🎮
