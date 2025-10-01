#ifndef __FINCH__
#define __FINCH__

/**
 * Finch Graphics Library
 *
 * A simple C library for 2D graphics and game development built on SDL2.
 * Provides an immediate-mode API for drawing primitives and managing pixel buffers.
 *
 * COORDINATE SYSTEM:
 *   Origin (0,0) is at top-left corner
 *   X axis increases to the right
 *   Y axis increases downward
 *
 *   (0,0) -------> X increases
 *     |
 *     |
 *     v
 *   Y increases
 *
 * PIXEL FORMAT (32-bit RGBA):
 *   0xAARRGGBB hexadecimal format
 *   AA = Alpha channel (transparency: 0=transparent, 255=opaque)
 *   RR = Red channel (0-255)
 *   GG = Green channel (0-255)
 *   BB = Blue channel (0-255)
 *
 * CALLBACK EXECUTION ORDER:
 *   1. FinchMain() - Entry point, call FinchStartGraphics() to begin
 *   2. FinchInit() - Called once after window creation
 *   3. Main loop begins (repeats until FinchDone() returns true):
 *      a. FinchHandleEvent() - Called for each input event
 *      b. FinchUpdate() - Called once per frame for game logic
 *      c. FinchRenderProc() - Called once per frame to draw
 *      d. FinchDone() - Checked to see if app should quit
 *   4. FinchCleanup() - Called once before exit
 */

#include "input_events.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * Pixel type: 32-bit RGBA value in 0xAARRGGBB format.
 * Use MakeColor() or color constants to create pixels.
 */
typedef uint32_t Pixel;

/**
 * Common color constants for convenience.
 * All colors are fully opaque (alpha = 255).
 *
 * Example:
 *   DrawCircle(buffer, COLOR_RED, 100, 100, 50);
 */
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

/**
 * RGB color structure (24-bit, no alpha).
 * Used as intermediate representation for color conversion.
 */
typedef struct  {
	uint8_t red, green, blue;
} RGBColor24;

/**
 * Rectangle structure defining a rectangular region.
 * Coordinates are inclusive: pixels from [left,top] to [right,bottom] are inside.
 *
 * Note: Functions automatically handle inverted rectangles (right < left).
 */
typedef struct {
	int32_t left, top, right, bottom;
} LSRect;

/**
 * Returns the minimum of two integers.
 */
inline int LSMin(int a, int b)
{
    return (a < b) ? a : b;
}

/**
 * Returns the maximum of two integers.
 */
inline int LSMax(int a, int b)
{
    return (a > b) ? a : b;
}

typedef bool LSBool;

/**
 * Graphics buffer structure: wraps a pixel array with metadata.
 *
 * Fields:
 *   id        - Unique identifier for this buffer
 *   ptr       - Pointer to pixel data (array of Pixel values)
 *   width     - Width in pixels
 *   height    - Height in pixels
 *   rowPixels - Number of pixels per row (may be >= width for stride/padding)
 *   size      - Size of buffer in bytes (may be 0 if not relevant)
 *   ownsPtr   - If true, ptr will be freed when DeleteGraphBuffer() is called
 *
 * Important: Always use NewGraphBuffer() and DeleteGraphBuffer() to manage buffers.
 */
typedef struct {
	uint32_t id;
	Pixel *ptr;
	uint32_t width, height;
	uint32_t rowPixels;
	uint32_t size;
	LSBool ownsPtr;
} GraphicsBuffer;


// ============================================================================
// COLOR FUNCTIONS
// ============================================================================

/**
 * Create a Pixel from RGBA components.
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0=transparent, 255=opaque)
 * @return Pixel value in 0xAARRGGBB format
 *
 * Example:
 *   Pixel semiTransparentRed = LSRGBA(255, 0, 0, 128); // 50% transparent red
 */
Pixel LSRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Convert RGBColor24 structure to opaque Pixel (alpha=255).
 *
 * @param c RGB color structure
 * @return Fully opaque pixel
 *
 * Example:
 *   RGBColor24 color = {255, 0, 0}; // red
 *   Pixel pixel = AsPixel(color);
 */
Pixel AsPixel(RGBColor24 c);

/**
 * Convert RGBColor24 structure to Pixel with specified alpha.
 *
 * @param c RGB color structure
 * @param alpha Transparency (0=transparent, 255=opaque)
 * @return Pixel with specified alpha
 */
Pixel AsPixelWithAlpha(RGBColor24 c, uint8_t alpha);

/**
 * Composite two 8-bit color values using alpha blending.
 * Formula: result = (a * m + b * (255 - m)) / 255
 *
 * @param a Source value
 * @param b Destination value
 * @param m Mask/alpha value (0-255)
 * @return Blended value
 */
extern inline uint32_t LSCompositeValues(uint32_t a, uint32_t b, uint32_t m);

/**
 * Composite two pixels using alpha blending.
 * Blends source pixel over destination pixel based on source alpha.
 *
 * @param sp Source pixel
 * @param dp Destination pixel
 * @return Composited pixel
 */
extern inline uint32_t LSCompositePixels(uint32_t sp, uint32_t dp);

/**
 * Opaque pixel composite (no blending).
 * Simply returns the source pixel, ignoring destination.
 *
 * @param sp Source pixel
 * @param dp Destination pixel (ignored)
 * @return Source pixel
 */
extern inline uint32_t LSCompositePixelsOpaque(uint32_t sp, uint32_t dp);


// ============================================================================
// APPLICATION CALLBACKS (must be implemented by your program)
// ============================================================================

/**
 * Application entry point.
 * Called first when the program starts. Should call FinchStartGraphics()
 * to create the window and begin the main loop.
 *
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @param userData Pointer to store application-specific data (optional)
 * @return true on success, false on error
 *
 * Example:
 *   bool FinchMain(int argc, const char* argv[], void** userData) {
 *       return FinchStartGraphics(800, 600);
 *   }
 */
bool FinchMain(int argc, const char* argv[], void** userData);

/**
 * Initialization callback.
 * Called once after the window is created, before the main loop begins.
 * Use this to initialize game state, load resources, etc.
 *
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @param userData Application data pointer from FinchMain
 * @return true on success, false to abort startup
 */
bool FinchInit(int width, int height, void* userData);

/**
 * Rendering callback.
 * Called once per frame to draw graphics. The pixels array is the screen buffer.
 *
 * IMPORTANT: Create a GraphicsBuffer wrapper using NewGraphBuffer() and remember
 * to call DeleteGraphBuffer() when done (the wrapper, not the pixel array).
 *
 * @param width Screen width in pixels
 * @param height Screen height in pixels
 * @param pixels Pointer to screen pixel buffer (do not free this!)
 * @param userData Application data pointer
 *
 * Example:
 *   void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
 *       GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);
 *       ClearBuffer(screen, COLOR_BLACK);
 *       DrawCircle(screen, COLOR_WHITE, width/2, height/2, 100);
 *       DeleteGraphBuffer(screen); // Only delete wrapper, not pixels!
 *   }
 */
void FinchRenderProc(int width, int height, uint32_t* pixels, void *userData);

/**
 * Cleanup callback.
 * Called once before the program exits. Use this to free resources,
 * save state, etc.
 *
 * @param userData Application data pointer
 */
void FinchCleanup(void *userData);

/**
 * Event handling callback.
 * Called for each input event (keyboard, mouse, window close, etc.).
 * May be called multiple times per frame if multiple events occurred.
 *
 * @param event Pointer to event data (see input_events.h)
 * @param userData Application data pointer
 *
 * Example:
 *   void FinchHandleEvent(InputEvent* event, void* userData) {
 *       if (event->eventType == kInputEventType_KeyDown && event->keyCode == 'q') {
 *           gQuitFlag = true;
 *       }
 *   }
 */
void FinchHandleEvent(InputEvent* event, void* userData);

/**
 * Quit check callback.
 * Called once per frame to check if the application should exit.
 *
 * @param userData Application data pointer
 * @return true to quit the application, false to continue
 */
bool FinchDone(void* userData);

/**
 * Update callback.
 * Called once per frame for game logic updates (physics, AI, etc.).
 * Called before FinchRenderProc() each frame.
 *
 * @param userData Application data pointer
 * @param elapsedTicks Time elapsed since last update (in SDL ticks, ~milliseconds)
 *
 * Example:
 *   void FinchUpdate(void* userData, double elapsedTicks) {
 *       playerX += playerVelocityX * (elapsedTicks / 1000.0);
 *   }
 */
void FinchUpdate(void* userData, double elapsedTicks);


// ============================================================================
// CORE FUNCTIONS
// ============================================================================

/**
 * Start the graphics system and create the window.
 * This begins the main event loop and will not return until FinchDone() returns true.
 *
 * Call this from FinchMain() to start your application.
 *
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @return true on success, false on error
 *
 * Example:
 *   bool FinchMain(int argc, const char* argv[], void** userData) {
 *       return FinchStartGraphics(800, 600);
 *   }
 */
bool FinchStartGraphics(int width, int height);


// ============================================================================
// BUFFER MANAGEMENT
// ============================================================================

/**
 * Create a new graphics buffer.
 *
 * If memory is NULL, allocates pixel array automatically (will be freed in DeleteGraphBuffer).
 * If memory is provided, wraps existing pixel array (you must manage its lifetime).
 *
 * @param memory Pointer to existing pixel array, or NULL to auto-allocate
 * @param width Width in pixels
 * @param height Height in pixels
 * @param rowPixels Pixels per row (usually equals width, may be larger for stride/padding)
 * @param size Size in bytes (use width*height*sizeof(Pixel) if unsure, or 0)
 * @return Pointer to new GraphicsBuffer, or NULL on error
 *
 * Example (auto-allocate):
 *   GraphicsBuffer* sprite = NewGraphBuffer(NULL, 64, 64, 64, 64*64*sizeof(Pixel));
 *   // ... use sprite ...
 *   DeleteGraphBuffer(sprite); // Frees pixel array and wrapper
 *
 * Example (wrap existing array):
 *   GraphicsBuffer* screen = NewGraphBuffer(pixels, 800, 600, 800, 0);
 *   // ... use screen ...
 *   DeleteGraphBuffer(screen); // Only frees wrapper, not pixels!
 */
GraphicsBuffer *NewGraphBuffer(
	Pixel *memory,
	uint32_t width,
	uint32_t height,
	uint32_t rowPixels,
	uint32_t size );

/**
 * Delete a graphics buffer.
 * Frees the GraphicsBuffer structure and pixel array (if owned by buffer).
 *
 * @param buffer Buffer to delete
 *
 * IMPORTANT: If buffer was created with memory=NULL, the pixel array is freed.
 *            If buffer was created wrapping existing memory, only the wrapper is freed.
 */
void DeleteGraphBuffer( GraphicsBuffer *buffer );

/**
 * Extract RGB components from a pixel.
 * Does not extract alpha channel.
 *
 * @param pixel Pixel value
 * @param rp Pointer to store red component (0-255)
 * @param gp Pointer to store green component (0-255)
 * @param bp Pointer to store blue component (0-255)
 *
 * Example:
 *   uint8_t r, g, b;
 *   PixelComponents(COLOR_YELLOW, &r, &g, &b);
 *   // r=255, g=255, b=0
 */
void PixelComponents(Pixel pixel, uint8_t* rp, uint8_t* gp, uint8_t* bp);


// ============================================================================
// BASIC DRAWING
// ============================================================================

/**
 * Clear or fill entire buffer with a solid color.
 * This is a convenience function equivalent to FillRectOpaque() for the entire buffer.
 *
 * @param buffer Graphics buffer to clear
 * @param color Fill color
 *
 * Example:
 *   ClearBuffer(screen, COLOR_BLACK); // Clear screen to black
 */
void ClearBuffer(GraphicsBuffer* buffer, Pixel color);

/**
 * Set a single pixel at (x,y) to the specified color.
 * Does nothing if coordinates are out of bounds (automatic clipping).
 *
 * @param buffer Graphics buffer to draw in
 * @param color Pixel color
 * @param x X coordinate (0 = left edge)
 * @param y Y coordinate (0 = top edge)
 *
 * Example:
 *   PutPixel(screen, COLOR_RED, 100, 100);
 */
void PutPixel(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y);

/**
 * Get the color of the pixel at (x,y).
 * Returns 0 (transparent black) if coordinates are out of bounds.
 *
 * @param buffer Graphics buffer to read from
 * @param x X coordinate
 * @param y Y coordinate
 * @return Pixel color at (x,y), or 0 if out of bounds
 *
 * Example:
 *   Pixel color = GetPixel(screen, 100, 100);
 */
Pixel GetPixel(GraphicsBuffer* buffer, int32_t x, int32_t y);


// ============================================================================
// LINE DRAWING
// ============================================================================

/**
 * Draw a horizontal line from (x1,y) to (x2,y).
 * Automatically clips to buffer bounds.
 *
 * Note: This function uses alpha compositing, not opaque drawing.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Line color
 * @param x1 Starting X coordinate
 * @param x2 Ending X coordinate (can be less than x1)
 * @param y Y coordinate of the line
 *
 * Example:
 *   DrawHorzLine(screen, COLOR_RED, 10, 100, 50); // Horizontal red line
 */
void DrawHorzLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t x2, int32_t y);

/**
 * Draw a vertical line from (x,y1) to (x,y2).
 * Automatically clips to buffer bounds.
 *
 * Note: This function uses alpha compositing, not opaque drawing.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Line color
 * @param y1 Starting Y coordinate
 * @param y2 Ending Y coordinate (can be less than y1)
 * @param x X coordinate of the line
 *
 * Example:
 *   DrawVertLine(screen, COLOR_GREEN, 10, 100, 50); // Vertical green line
 */
void DrawVertLine(GraphicsBuffer *buffer, Pixel color, int32_t y1, int32_t y2, int32_t x);

/**
 * Draw a line from (x1,y1) to (x2,y2) using Bresenham's algorithm.
 * Handles all angles and directions. Automatically clips to buffer bounds.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Line color
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 *
 * Example:
 *   DrawLine(screen, COLOR_BLUE, 10, 10, 100, 100); // Diagonal blue line
 */
void DrawLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**
 * Draw a line with alpha compositing (blends with existing pixels).
 * Same as DrawLine() but uses alpha blending instead of overwriting pixels.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Line color (alpha channel is used for transparency)
 * @param x1 Starting X coordinate
 * @param y1 Starting Y coordinate
 * @param x2 Ending X coordinate
 * @param y2 Ending Y coordinate
 *
 * Example:
 *   Pixel semiTransparent = LSRGBA(255, 0, 0, 128); // 50% red
 *   DrawLineComposite(screen, semiTransparent, 10, 10, 100, 100);
 */
void DrawLineComposite(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);


// ============================================================================
// RECTANGLE DRAWING
// ============================================================================

/**
 * Draw a rectangle outline.
 * Coordinates define the rectangle bounds (inclusive).
 * Automatically handles inverted coordinates and clips to buffer bounds.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Rectangle color
 * @param left Left edge X coordinate
 * @param top Top edge Y coordinate
 * @param right Right edge X coordinate
 * @param bottom Bottom edge Y coordinate
 *
 * Example:
 *   DrawRect(screen, COLOR_YELLOW, 50, 50, 150, 100); // Yellow rectangle outline
 */
void DrawRect(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom);

/**
 * Fill a rectangle with solid color (no alpha blending).
 * Coordinates define the rectangle bounds (inclusive).
 * Automatically handles inverted coordinates and clips to buffer bounds.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Fill color
 * @param left Left edge X coordinate
 * @param top Top edge Y coordinate
 * @param right Right edge X coordinate
 * @param bottom Bottom edge Y coordinate
 *
 * Example:
 *   FillRectOpaque(screen, COLOR_RED, 50, 50, 150, 100); // Solid red rectangle
 */
void FillRectOpaque(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom);


// ============================================================================
// CIRCLE DRAWING
// ============================================================================

/**
 * Draw a circle outline using midpoint circle algorithm.
 * Automatically clips to buffer bounds.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Circle color
 * @param xCenter Center X coordinate
 * @param yCenter Center Y coordinate
 * @param radius Radius in pixels
 *
 * Example:
 *   DrawCircle(screen, COLOR_CYAN, 200, 200, 50); // Cyan circle outline
 */
void DrawCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius);

/**
 * Draw a filled circle using scanline fill algorithm.
 * Automatically clips to buffer bounds.
 *
 * @param buffer Graphics buffer to draw in
 * @param color Circle color
 * @param xCenter Center X coordinate
 * @param yCenter Center Y coordinate
 * @param radius Radius in pixels
 *
 * Example:
 *   FillCircle(screen, COLOR_MAGENTA, 200, 200, 50); // Solid magenta circle
 */
void FillCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius);


// ============================================================================
// BLITTING (copying buffer contents)
// ============================================================================

/**
 * Copy source buffer into destination buffer at specified position (opaque).
 * Automatically clips to destination buffer bounds.
 *
 * This version ignores alpha and overwrites destination pixels.
 *
 * @param srcBuffer Source buffer to copy from
 * @param destBuffer Destination buffer to copy into
 * @param xDestLoc Destination X coordinate (top-left of where source appears)
 * @param yDestLoc Destination Y coordinate (top-left of where source appears)
 *
 * Example:
 *   BlitGraphBuffer(sprite, screen, 100, 100); // Draw sprite at (100,100)
 */
void BlitGraphBuffer(
	GraphicsBuffer *srcBuffer,
	GraphicsBuffer *destBuffer,
	int32_t xDestLoc,
	int32_t yDestLoc
	);

/**
 * Copy source buffer into destination buffer with alpha blending.
 * Automatically clips to destination buffer bounds.
 *
 * This version uses alpha compositing to blend source over destination.
 *
 * @param srcBuffer Source buffer to copy from
 * @param destBuffer Destination buffer to copy into
 * @param xDestLoc Destination X coordinate (top-left of where source appears)
 * @param yDestLoc Destination Y coordinate (top-left of where source appears)
 *
 * Example:
 *   BlitGraphBufferComposite(transparentSprite, screen, 100, 100);
 */
void BlitGraphBufferComposite(
	GraphicsBuffer *srcBuffer,
	GraphicsBuffer *destBuffer,
	int32_t xDestLoc,
	int32_t yDestLoc
	);


// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if a point is inside a rectangle.
 *
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param r Rectangle to test
 * @return 1 if point is inside rectangle, 0 otherwise
 *
 * Example:
 *   LSRect button = {10, 10, 60, 30};
 *   if (LSPointInRect(mouseX, mouseY, button)) {
 *       // Mouse is over button
 *   }
 */
int LSPointInRect( int x, int y, const LSRect r );

/**
 * Calculate the intersection of two rectangles.
 *
 * @param r1 First rectangle
 * @param r2 Second rectangle
 * @param sect Pointer to store intersection rectangle (output parameter)
 * @return 1 if rectangles intersect, 0 if they don't
 *
 * Example:
 *   LSRect playerRect = {10, 10, 30, 30};
 *   LSRect enemyRect = {20, 20, 40, 40};
 *   LSRect intersection;
 *   if (IntersectRects(playerRect, enemyRect, &intersection)) {
 *       // Collision detected!
 *   }
 */
int IntersectRects( const LSRect r1, const LSRect r2, LSRect* sect );


#endif // __FINCH__
