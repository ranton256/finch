# Finch Graphics Library - API Reference

A simple C library for 2D graphics and game development built on SDL2. Provides an immediate-mode API for drawing primitives and managing pixel buffers.

## Table of Contents

- [Coordinate System](#coordinate-system)
- [Pixel Format](#pixel-format)
- [Core Types](#core-types)
- [Color Constants](#color-constants)
- [Application Lifecycle](#application-lifecycle)
- [Core Functions](#core-functions)
- [Buffer Management](#buffer-management)
- [Color Functions](#color-functions)
- [Basic Drawing](#basic-drawing)
- [Line Drawing](#line-drawing)
- [Rectangle Drawing](#rectangle-drawing)
- [Circle Drawing](#circle-drawing)
- [Text Rendering](#text-rendering)
- [Blitting](#blitting)
- [Utility Functions](#utility-functions)
- [Input Events](#input-events)

---

## Coordinate System

Origin (0,0) is at the top-left corner:
- X axis increases to the right
- Y axis increases downward

```
(0,0) -------> X increases
  |
  |
  v
Y increases
```

---

## Pixel Format

32-bit RGBA format: `0xAARRGGBB`
- **AA** = Alpha channel (0=transparent, 255=opaque)
- **RR** = Red channel (0-255)
- **GG** = Green channel (0-255)
- **BB** = Blue channel (0-255)

---

## Core Types

### Pixel

```c
typedef uint32_t Pixel;
```

32-bit RGBA value in `0xAARRGGBB` format. Use `LSRGBA()` or color constants to create pixels.

### RGBColor24

```c
typedef struct {
    uint8_t red, green, blue;
} RGBColor24;
```

RGB color structure (24-bit, no alpha). Used as intermediate representation for color conversion.

### LSRect

```c
typedef struct {
    int32_t left, top, right, bottom;
} LSRect;
```

Rectangle structure defining a rectangular region. Coordinates are inclusive: pixels from [left,top] to [right,bottom] are inside. Functions automatically handle inverted rectangles (right < left).

### GraphicsBuffer

```c
typedef struct {
    uint32_t id;
    Pixel *ptr;
    uint32_t width, height;
    uint32_t rowPixels;
    uint32_t size;
    LSBool ownsPtr;
} GraphicsBuffer;
```

Graphics buffer structure wrapping a pixel array with metadata.

**Fields:**
- `id` - Unique identifier for this buffer
- `ptr` - Pointer to pixel data (array of Pixel values)
- `width` - Width in pixels
- `height` - Height in pixels
- `rowPixels` - Number of pixels per row (may be >= width for stride/padding)
- `size` - Size of buffer in bytes (may be 0 if not relevant)
- `ownsPtr` - If true, ptr will be freed when DeleteGraphBuffer() is called

**Important:** Always use `NewGraphBuffer()` and `DeleteGraphBuffer()` to manage buffers.

---

## Color Constants

All color constants are fully opaque (alpha = 255).

```c
extern const Pixel COLOR_WHITE;
extern const Pixel COLOR_BLACK;
extern const Pixel COLOR_RED;
extern const Pixel COLOR_GREEN;
extern const Pixel COLOR_BLUE;
extern const Pixel COLOR_YELLOW;
extern const Pixel COLOR_CYAN;
extern const Pixel COLOR_MAGENTA;
extern const Pixel COLOR_GRAY;
extern const Pixel COLOR_DARK_GRAY;
extern const Pixel COLOR_LIGHT_GRAY;
```

**Example:**
```c
DrawCircle(buffer, COLOR_RED, 100, 100, 50);
```

---

## Application Lifecycle

### Callback Execution Order

1. **FinchMain()** - Entry point, call FinchStartGraphics() to begin
2. **FinchInit()** - Called once after window creation
3. **Main loop** (repeats until FinchDone() returns true):
   - **FinchHandleEvent()** - Called for each input event
   - **FinchUpdate()** - Called once per frame for game logic
   - **FinchRenderProc()** - Called once per frame to draw
   - **FinchDone()** - Checked to see if app should quit
4. **FinchCleanup()** - Called once before exit

### FinchMain

```c
bool FinchMain(int argc, const char* argv[], void** userData);
```

Application entry point. Called first when the program starts. Should call `FinchStartGraphics()` to create the window and begin the main loop.

**Parameters:**
- `argc` - Command line argument count
- `argv` - Command line arguments
- `userData` - Pointer to store application-specific data (optional)

**Returns:** `true` on success, `false` on error

**Example:**
```c
bool FinchMain(int argc, const char* argv[], void** userData) {
    return FinchStartGraphics(800, 600);
}
```

### FinchInit

```c
bool FinchInit(int width, int height, void* userData);
```

Initialization callback. Called once after the window is created, before the main loop begins. Use this to initialize game state, load resources, etc.

**Parameters:**
- `width` - Window width in pixels
- `height` - Window height in pixels
- `userData` - Application data pointer from FinchMain

**Returns:** `true` on success, `false` to abort startup

### FinchRenderProc

```c
void FinchRenderProc(int width, int height, uint32_t* pixels, void *userData);
```

Rendering callback. Called once per frame to draw graphics. The pixels array is the screen buffer.

**Important:** Create a GraphicsBuffer wrapper using `NewGraphBuffer()` and remember to call `DeleteGraphBuffer()` when done (the wrapper, not the pixel array).

**Parameters:**
- `width` - Screen width in pixels
- `height` - Screen height in pixels
- `pixels` - Pointer to screen pixel buffer (do not free this!)
- `userData` - Application data pointer

**Example:**
```c
void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);
    ClearBuffer(screen, COLOR_BLACK);
    DrawCircle(screen, COLOR_WHITE, width/2, height/2, 100);
    DeleteGraphBuffer(screen); // Only delete wrapper, not pixels!
}
```

### FinchUpdate

```c
void FinchUpdate(void* userData, double elapsedTicks);
```

Update callback. Called once per frame for game logic updates (physics, AI, etc.). Called before FinchRenderProc() each frame.

**Parameters:**
- `userData` - Application data pointer
- `elapsedTicks` - Time elapsed since last update (in SDL ticks, ~milliseconds)

**Example:**
```c
void FinchUpdate(void* userData, double elapsedTicks) {
    playerX += playerVelocityX * (elapsedTicks / 1000.0);
}
```

### FinchHandleEvent

```c
void FinchHandleEvent(InputEvent* event, void* userData);
```

Event handling callback. Called for each input event (keyboard, mouse, window close, etc.). May be called multiple times per frame if multiple events occurred.

**Parameters:**
- `event` - Pointer to event data (see [Input Events](#input-events))
- `userData` - Application data pointer

**Example:**
```c
void FinchHandleEvent(InputEvent* event, void* userData) {
    if (event->eventType == kInputEventType_KeyDown && event->keyCode == 'q') {
        gQuitFlag = true;
    }
}
```

### FinchDone

```c
bool FinchDone(void* userData);
```

Quit check callback. Called once per frame to check if the application should exit.

**Parameters:**
- `userData` - Application data pointer

**Returns:** `true` to quit the application, `false` to continue

### FinchCleanup

```c
void FinchCleanup(void *userData);
```

Cleanup callback. Called once before the program exits. Use this to free resources, save state, etc.

**Parameters:**
- `userData` - Application data pointer

---

## Core Functions

### FinchStartGraphics

```c
bool FinchStartGraphics(int width, int height);
```

Start the graphics system and create the window. This begins the main event loop and will not return until FinchDone() returns true.

Call this from FinchMain() to start your application.

**Parameters:**
- `width` - Window width in pixels
- `height` - Window height in pixels

**Returns:** `true` on success, `false` on error

**Example:**
```c
bool FinchMain(int argc, const char* argv[], void** userData) {
    return FinchStartGraphics(800, 600);
}
```

---

## Buffer Management

### NewGraphBuffer

```c
GraphicsBuffer *NewGraphBuffer(
    Pixel *memory,
    uint32_t width,
    uint32_t height,
    uint32_t rowPixels,
    uint32_t size);
```

Create a new graphics buffer.

If memory is NULL, allocates pixel array automatically (will be freed in DeleteGraphBuffer). If memory is provided, wraps existing pixel array (you must manage its lifetime).

**Parameters:**
- `memory` - Pointer to existing pixel array, or NULL to auto-allocate
- `width` - Width in pixels
- `height` - Height in pixels
- `rowPixels` - Pixels per row (usually equals width, may be larger for stride/padding)
- `size` - Size in bytes (use width*height*sizeof(Pixel) if unsure, or 0)

**Returns:** Pointer to new GraphicsBuffer, or NULL on error

**Example (auto-allocate):**
```c
GraphicsBuffer* sprite = NewGraphBuffer(NULL, 64, 64, 64, 64*64*sizeof(Pixel));
// ... use sprite ...
DeleteGraphBuffer(sprite); // Frees pixel array and wrapper
```

**Example (wrap existing array):**
```c
GraphicsBuffer* screen = NewGraphBuffer(pixels, 800, 600, 800, 0);
// ... use screen ...
DeleteGraphBuffer(screen); // Only frees wrapper, not pixels!
```

### DeleteGraphBuffer

```c
void DeleteGraphBuffer(GraphicsBuffer *buffer);
```

Delete a graphics buffer. Frees the GraphicsBuffer structure and pixel array (if owned by buffer).

**Parameters:**
- `buffer` - Buffer to delete

**Important:** If buffer was created with memory=NULL, the pixel array is freed. If buffer was created wrapping existing memory, only the wrapper is freed.

---

## Color Functions

### LSRGBA

```c
Pixel LSRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
```

Create a Pixel from RGBA components.

**Parameters:**
- `r` - Red component (0-255)
- `g` - Green component (0-255)
- `b` - Blue component (0-255)
- `a` - Alpha component (0=transparent, 255=opaque)

**Returns:** Pixel value in `0xAARRGGBB` format

**Example:**
```c
Pixel semiTransparentRed = LSRGBA(255, 0, 0, 128); // 50% transparent red
```

### MakeColor

```c
uint32_t MakeColor(uint8_t r, uint8_t g, uint8_t b);
```

Create a fully opaque color from RGB components.

**Parameters:**
- `r` - Red component (0-255)
- `g` - Green component (0-255)
- `b` - Blue component (0-255)

**Returns:** Pixel value with alpha=255

### MakeColorWithAlpha

```c
uint32_t MakeColorWithAlpha(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
```

Create a color from RGBA components.

**Parameters:**
- `r` - Red component (0-255)
- `g` - Green component (0-255)
- `b` - Blue component (0-255)
- `a` - Alpha component (0-255)

**Returns:** Pixel value in `0xAARRGGBB` format

### AsPixel

```c
Pixel AsPixel(RGBColor24 c);
```

Convert RGBColor24 structure to opaque Pixel (alpha=255).

**Parameters:**
- `c` - RGB color structure

**Returns:** Fully opaque pixel

**Example:**
```c
RGBColor24 color = {255, 0, 0}; // red
Pixel pixel = AsPixel(color);
```

### AsPixelWithAlpha

```c
Pixel AsPixelWithAlpha(RGBColor24 c, uint8_t alpha);
```

Convert RGBColor24 structure to Pixel with specified alpha.

**Parameters:**
- `c` - RGB color structure
- `alpha` - Transparency (0=transparent, 255=opaque)

**Returns:** Pixel with specified alpha

### PixelComponents

```c
void PixelComponents(Pixel pixel, uint8_t* rp, uint8_t* gp, uint8_t* bp);
```

Extract RGB components from a pixel. Does not extract alpha channel.

**Parameters:**
- `pixel` - Pixel value
- `rp` - Pointer to store red component (0-255)
- `gp` - Pointer to store green component (0-255)
- `bp` - Pointer to store blue component (0-255)

**Example:**
```c
uint8_t r, g, b;
PixelComponents(COLOR_YELLOW, &r, &g, &b);
// r=255, g=255, b=0
```

### Color2Values

```c
void Color2Values(uint32_t color, uint8_t components[4]);
```

Extract all color components (RGBA) into an array.

**Parameters:**
- `color` - Pixel value
- `components` - Array of 4 uint8_t to store [R, G, B, A]

### LSCompositeValues

```c
extern inline uint32_t LSCompositeValues(uint32_t a, uint32_t b, uint32_t m);
```

Composite two 8-bit color values using alpha blending.

Formula: `result = (a * m + b * (255 - m)) / 255`

**Parameters:**
- `a` - Source value
- `b` - Destination value
- `m` - Mask/alpha value (0-255)

**Returns:** Blended value

### LSCompositePixels

```c
extern inline uint32_t LSCompositePixels(uint32_t sp, uint32_t dp);
```

Composite two pixels using alpha blending. Blends source pixel over destination pixel based on source alpha.

**Parameters:**
- `sp` - Source pixel
- `dp` - Destination pixel

**Returns:** Composited pixel

### LSCompositePixelsOpaque

```c
extern inline uint32_t LSCompositePixelsOpaque(uint32_t sp, uint32_t dp);
```

Opaque pixel composite (no blending). Simply returns the source pixel, ignoring destination.

**Parameters:**
- `sp` - Source pixel
- `dp` - Destination pixel (ignored)

**Returns:** Source pixel

---

## Basic Drawing

### ClearBuffer

```c
void ClearBuffer(GraphicsBuffer* buffer, Pixel color);
```

Clear or fill entire buffer with a solid color. This is a convenience function equivalent to FillRectOpaque() for the entire buffer.

**Parameters:**
- `buffer` - Graphics buffer to clear
- `color` - Fill color

**Example:**
```c
ClearBuffer(screen, COLOR_BLACK); // Clear screen to black
```

### PutPixel

```c
void PutPixel(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y);
```

Set a single pixel at (x,y) to the specified color. Does nothing if coordinates are out of bounds (automatic clipping).

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Pixel color
- `x` - X coordinate (0 = left edge)
- `y` - Y coordinate (0 = top edge)

**Example:**
```c
PutPixel(screen, COLOR_RED, 100, 100);
```

### GetPixel

```c
Pixel GetPixel(GraphicsBuffer* buffer, int32_t x, int32_t y);
```

Get the color of the pixel at (x,y). Returns 0 (transparent black) if coordinates are out of bounds.

**Parameters:**
- `buffer` - Graphics buffer to read from
- `x` - X coordinate
- `y` - Y coordinate

**Returns:** Pixel color at (x,y), or 0 if out of bounds

**Example:**
```c
Pixel color = GetPixel(screen, 100, 100);
```

---

## Line Drawing

### DrawHorzLine

```c
void DrawHorzLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t x2, int32_t y);
```

Draw a horizontal line from (x1,y) to (x2,y). Automatically clips to buffer bounds.

**Note:** This function uses alpha compositing, not opaque drawing.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Line color
- `x1` - Starting X coordinate
- `x2` - Ending X coordinate (can be less than x1)
- `y` - Y coordinate of the line

**Example:**
```c
DrawHorzLine(screen, COLOR_RED, 10, 100, 50); // Horizontal red line
```

### DrawVertLine

```c
void DrawVertLine(GraphicsBuffer *buffer, Pixel color, int32_t y1, int32_t y2, int32_t x);
```

Draw a vertical line from (x,y1) to (x,y2). Automatically clips to buffer bounds.

**Note:** This function uses alpha compositing, not opaque drawing.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Line color
- `y1` - Starting Y coordinate
- `y2` - Ending Y coordinate (can be less than y1)
- `x` - X coordinate of the line

**Example:**
```c
DrawVertLine(screen, COLOR_GREEN, 10, 100, 50); // Vertical green line
```

### DrawLine

```c
void DrawLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
```

Draw a line from (x1,y1) to (x2,y2) using Bresenham's algorithm. Handles all angles and directions. Automatically clips to buffer bounds.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Line color
- `x1` - Starting X coordinate
- `y1` - Starting Y coordinate
- `x2` - Ending X coordinate
- `y2` - Ending Y coordinate

**Example:**
```c
DrawLine(screen, COLOR_BLUE, 10, 10, 100, 100); // Diagonal blue line
```

### DrawLineComposite

```c
void DrawLineComposite(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
```

Draw a line with alpha compositing (blends with existing pixels). Same as DrawLine() but uses alpha blending instead of overwriting pixels.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Line color (alpha channel is used for transparency)
- `x1` - Starting X coordinate
- `y1` - Starting Y coordinate
- `x2` - Ending X coordinate
- `y2` - Ending Y coordinate

**Example:**
```c
Pixel semiTransparent = LSRGBA(255, 0, 0, 128); // 50% red
DrawLineComposite(screen, semiTransparent, 10, 10, 100, 100);
```

---

## Rectangle Drawing

### DrawRect

```c
void DrawRect(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom);
```

Draw a rectangle outline. Coordinates define the rectangle bounds (inclusive). Automatically handles inverted coordinates and clips to buffer bounds.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Rectangle color
- `left` - Left edge X coordinate
- `top` - Top edge Y coordinate
- `right` - Right edge X coordinate
- `bottom` - Bottom edge Y coordinate

**Example:**
```c
DrawRect(screen, COLOR_YELLOW, 50, 50, 150, 100); // Yellow rectangle outline
```

### FillRectOpaque

```c
void FillRectOpaque(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom);
```

Fill a rectangle with solid color (no alpha blending). Coordinates define the rectangle bounds (inclusive). Automatically handles inverted coordinates and clips to buffer bounds.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Fill color
- `left` - Left edge X coordinate
- `top` - Top edge Y coordinate
- `right` - Right edge X coordinate
- `bottom` - Bottom edge Y coordinate

**Example:**
```c
FillRectOpaque(screen, COLOR_RED, 50, 50, 150, 100); // Solid red rectangle
```

---

## Circle Drawing

### DrawCircle

```c
void DrawCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius);
```

Draw a circle outline using midpoint circle algorithm. Automatically clips to buffer bounds.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Circle color
- `xCenter` - Center X coordinate
- `yCenter` - Center Y coordinate
- `radius` - Radius in pixels

**Example:**
```c
DrawCircle(screen, COLOR_CYAN, 200, 200, 50); // Cyan circle outline
```

### FillCircle

```c
void FillCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius);
```

Draw a filled circle using scanline fill algorithm. Automatically clips to buffer bounds.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Circle color
- `xCenter` - Center X coordinate
- `yCenter` - Center Y coordinate
- `radius` - Radius in pixels

**Example:**
```c
FillCircle(screen, COLOR_MAGENTA, 200, 200, 50); // Solid magenta circle
```

---

## Text Rendering

All text rendering uses a built-in 8x8 bitmap font supporting ASCII characters 32-126.

### Font Constants

```c
#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 8
#define FONT_FIRST_CHAR 32
#define FONT_LAST_CHAR 126
```

### DrawChar

```c
void DrawChar(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y, char c);
```

Draw a single character at the specified position.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Text color
- `x` - Left edge X coordinate
- `y` - Top edge Y coordinate
- `c` - Character to draw (ASCII 32-126)

**Example:**
```c
DrawChar(screen, COLOR_WHITE, 100, 100, 'A');
```

### DrawText

```c
void DrawText(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y, const char* text);
```

Draw a text string at the specified position. Characters are drawn left-to-right with no spacing between them.

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Text color
- `x` - Left edge X coordinate
- `y` - Top edge Y coordinate
- `text` - Null-terminated string to draw

**Example:**
```c
DrawText(screen, COLOR_GREEN, 10, 10, "Score: 1000");
```

### DrawTextCentered

```c
void DrawTextCentered(GraphicsBuffer* buffer, Pixel color, int32_t centerX, int32_t centerY, const char* text);
```

Draw text centered at a specific point. The text will be centered horizontally and vertically around (centerX, centerY).

**Parameters:**
- `buffer` - Graphics buffer to draw in
- `color` - Text color
- `centerX` - Center point X coordinate
- `centerY` - Center point Y coordinate
- `text` - Null-terminated string to draw

**Example:**
```c
DrawTextCentered(screen, COLOR_RED, 400, 300, "GAME OVER");
```

### GetTextWidth

```c
int GetTextWidth(const char* text);
```

Calculate the width in pixels of a text string. Each character is 8 pixels wide.

**Parameters:**
- `text` - Null-terminated string

**Returns:** Width in pixels (length * 8)

**Example:**
```c
int width = GetTextWidth("Hello");  // Returns 40
DrawText(screen, COLOR_WHITE, (800 - width) / 2, 100, "Hello"); // Centered
```

### GetTextHeight

```c
int GetTextHeight(void);
```

Get the height of text in pixels. All characters are 8 pixels tall.

**Returns:** Height in pixels (always 8)

---

## Blitting

Blitting copies buffer contents from one buffer to another.

### BlitGraphBuffer

```c
void BlitGraphBuffer(
    GraphicsBuffer *srcBuffer,
    GraphicsBuffer *destBuffer,
    int32_t xDestLoc,
    int32_t yDestLoc);
```

Copy source buffer into destination buffer at specified position (opaque). Automatically clips to destination buffer bounds.

This version ignores alpha and overwrites destination pixels.

**Parameters:**
- `srcBuffer` - Source buffer to copy from
- `destBuffer` - Destination buffer to copy into
- `xDestLoc` - Destination X coordinate (top-left of where source appears)
- `yDestLoc` - Destination Y coordinate (top-left of where source appears)

**Example:**
```c
BlitGraphBuffer(sprite, screen, 100, 100); // Draw sprite at (100,100)
```

### BlitGraphBufferComposite

```c
void BlitGraphBufferComposite(
    GraphicsBuffer *srcBuffer,
    GraphicsBuffer *destBuffer,
    int32_t xDestLoc,
    int32_t yDestLoc);
```

Copy source buffer into destination buffer with alpha blending. Automatically clips to destination buffer bounds.

This version uses alpha compositing to blend source over destination.

**Parameters:**
- `srcBuffer` - Source buffer to copy from
- `destBuffer` - Destination buffer to copy into
- `xDestLoc` - Destination X coordinate (top-left of where source appears)
- `yDestLoc` - Destination Y coordinate (top-left of where source appears)

**Example:**
```c
BlitGraphBufferComposite(transparentSprite, screen, 100, 100);
```

### Blit32Bit

```c
void Blit32Bit(Pixel* dst, uint8_t* src, uint32_t width, uint32_t height);
```

Low-level blit function for copying 32-bit pixel data to SDL buffers.

**Parameters:**
- `dst` - Destination pixel buffer
- `src` - Source data buffer (8-bit per component)
- `width` - Width in pixels
- `height` - Height in pixels

### Blit24To32Bit

```c
void Blit24To32Bit(Pixel* dst, uint8_t* src, uint32_t width, uint32_t height);
```

Low-level blit function for converting and copying 24-bit RGB data to 32-bit RGBA buffer.

**Parameters:**
- `dst` - Destination pixel buffer
- `src` - Source RGB data buffer
- `width` - Width in pixels
- `height` - Height in pixels

---

## Utility Functions

### LSMin

```c
inline int LSMin(int a, int b);
```

Returns the minimum of two integers.

**Parameters:**
- `a` - First integer
- `b` - Second integer

**Returns:** Minimum value

### LSMax

```c
inline int LSMax(int a, int b);
```

Returns the maximum of two integers.

**Parameters:**
- `a` - First integer
- `b` - Second integer

**Returns:** Maximum value

### LSPointInRect

```c
int LSPointInRect(int x, int y, const LSRect r);
```

Check if a point is inside a rectangle.

**Parameters:**
- `x` - Point X coordinate
- `y` - Point Y coordinate
- `r` - Rectangle to test

**Returns:** 1 if point is inside rectangle, 0 otherwise

**Example:**
```c
LSRect button = {10, 10, 60, 30};
if (LSPointInRect(mouseX, mouseY, button)) {
    // Mouse is over button
}
```

### IntersectRects

```c
int IntersectRects(const LSRect r1, const LSRect r2, LSRect* sect);
```

Calculate the intersection of two rectangles.

**Parameters:**
- `r1` - First rectangle
- `r2` - Second rectangle
- `sect` - Pointer to store intersection rectangle (output parameter)

**Returns:** 1 if rectangles intersect, 0 if they don't

**Example:**
```c
LSRect playerRect = {10, 10, 30, 30};
LSRect enemyRect = {20, 20, 40, 40};
LSRect intersection;
if (IntersectRects(playerRect, enemyRect, &intersection)) {
    // Collision detected!
}
```

---

## Input Events

### InputEventType

```c
typedef enum {
    kInputEventType_Nothing = 0,
    kInputEventType_MouseDown,
    kInputEventType_MouseUp,
    kInputEventType_MouseMove,
    kInputEventType_KeyDown,
    kInputEventType_KeyUp,
    kInputEventType_Quit
} InputEventType;
```

Enumeration of all possible input event types.

**Values:**
- `kInputEventType_Nothing` - No event
- `kInputEventType_MouseDown` - Mouse button pressed
- `kInputEventType_MouseUp` - Mouse button released
- `kInputEventType_MouseMove` - Mouse moved
- `kInputEventType_KeyDown` - Keyboard key pressed
- `kInputEventType_KeyUp` - Keyboard key released
- `kInputEventType_Quit` - Application quit requested (window close)

### InputEvent

```c
typedef struct InputEvent_t {
    InputEventType eventType;
    // mouse event fields
    uint32_t x, y;
    uint32_t button;
    // keyboard event fields
    uint32_t scanCode;
    uint32_t keyCode;
    // modifier keys
    uint32_t modifiers;
} InputEvent;
```

Input event structure passed to FinchHandleEvent().

**Fields:**
- `eventType` - Type of event (see InputEventType)
- `x, y` - Mouse position (for mouse events)
- `button` - Mouse button number (for mouse button events)
- `scanCode` - Physical key scan code (for keyboard events)
- `keyCode` - Virtual key code / character (for keyboard events)
- `modifiers` - Modifier key flags (Shift, Ctrl, Alt, etc.)

**Example:**
```c
void FinchHandleEvent(InputEvent* event, void* userData) {
    switch (event->eventType) {
        case kInputEventType_MouseDown:
            printf("Mouse clicked at (%d, %d)\n", event->x, event->y);
            break;
        case kInputEventType_KeyDown:
            if (event->keyCode == 'q') {
                gQuitFlag = true;
            }
            break;
        case kInputEventType_Quit:
            gQuitFlag = true;
            break;
    }
}
```

---

## Additional Notes

### Memory Management

- Always use `NewGraphBuffer()` and `DeleteGraphBuffer()` for buffer management
- Never manually free pixel arrays passed to `FinchRenderProc()`
- When wrapping existing memory, only the GraphicsBuffer wrapper is freed
- When auto-allocating (memory=NULL), both wrapper and pixels are freed

### Drawing Notes

- All drawing functions automatically clip to buffer bounds
- Coordinates outside the buffer are safely ignored
- Rectangle functions handle inverted coordinates (right < left, bottom < top)
- Alpha compositing functions blend based on source alpha channel
- Opaque functions ignore alpha and overwrite destination pixels

### Performance Tips

- Use opaque functions (BlitGraphBuffer, FillRectOpaque) when alpha blending is not needed
- Clear the entire screen once with ClearBuffer() rather than multiple FillRectOpaque() calls
- Batch similar drawing operations together
- Pre-allocate buffers for sprites/textures during initialization
