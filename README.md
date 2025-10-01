# Finch

Finch is a small C library for graphics and 2D games that uses SDL2. It provides a simple, immediate-mode API for drawing primitives and managing pixel buffers, making it easy to build 2D graphics applications and games without dealing directly with SDL2 complexity.

## Purpose

Finch abstracts SDL2's window management and event handling, providing a clean callback-based interface for developers who want to focus on rendering and game logic rather than platform-specific setup. It's designed for educational purposes, prototyping, and small 2D graphics projects.

## Capabilities

### Graphics Primitives
- **Pixels**: Direct pixel manipulation with `PutPixel()` and `GetPixel()`
- **Lines**: Horizontal, vertical, and arbitrary lines using Bresenham's algorithm
- **Rectangles**: Outlined and filled rectangles with clipping
- **Circles**: Outlined and filled circles
- **Blitting**: Copy graphics buffers with transparency and alpha compositing

### Graphics Buffers
- Flexible `GraphicsBuffer` abstraction for managing pixel data
- Support for both owned and external memory buffers
- 32-bit RGBA pixel format with alpha channel support

### Input Handling
- Mouse events (button presses, movement)
- Keyboard events (key presses with modifiers)
- Quit/window close events

### Audio (via SDL2_mixer)
- Sound playback support through SDL2_mixer integration

## Use Cases

- 2D game prototyping
- Educational graphics programming
- Software rendering experiments
- Pixel art applications
- Simple visualization tools

## Design

### Callback Architecture

Finch uses a callback-based design where your application implements specific functions that the framework calls at appropriate times:

```c
bool FinchMain(int argc, const char* argv[], void** userData);
bool FinchInit(int width, int height, void* userData);
void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData);
void FinchUpdate(void* userData, double elapsedTicks);
void FinchHandleEvent(InputEvent* event, void* userData);
bool FinchDone(void* userData);
void FinchCleanup(void* userData);
```

The main loop is handled internally - you just implement the callbacks and call `FinchStartGraphics()` to begin execution.

### GraphicsBuffer

The core abstraction is `GraphicsBuffer`, which wraps a pixel buffer with metadata:
- Pixel pointer (`ptr`)
- Dimensions (`width`, `height`)
- Row stride (`rowPixels`)
- Memory ownership flag (`ownsPtr`)

This design allows wrapping external buffers (like SDL2's screen buffer) or creating standalone buffers for sprites and textures.

### Coordinate System

Standard screen coordinates: origin (0,0) at top-left, X increases right, Y increases down.

### Color Format

32-bit RGBA pixels: `0xAARRGGBB` format. Helper functions:
- `LSRGBA(r, g, b, a)` - Create pixel from components
- `MakeColor(r, g, b)` - Create opaque pixel
- `MakeColorWithAlpha(r, g, b, a)` - Create pixel with alpha
- `AsPixel(RGBColor24)` - Convert RGB struct to pixel

## Project Structure

```
finch/
├── finch.h, finch.c          # Core library (GraphicsBuffer, drawing primitives)
├── blit.h, blit.c            # Pixel blitting and color utilities
├── input_events.h            # Input event definitions
├── sound.h, sound.c          # Audio system (SDL2_mixer wrapper)
├── sdl2main.c                # SDL2 integration and main loop
├── finch_main.c              # Example application
├── finch_test.c              # Unit tests
├── Makefile                  # Build system
├── Makefile.sdlflags         # Platform-specific SDL2 configuration
└── build/                    # Build artifacts (generated)
```

## Developer Setup

### Prerequisites

You need the following libraries installed:

- **SDL2** - Core graphics library
- **SDL2_mixer** - Audio library
- **libpng** - PNG image support
- **gcc** or compatible C compiler

### Platform-Specific Setup

#### macOS (Homebrew)
```bash
brew install sdl2 sdl2_mixer libpng
```

#### Ubuntu/Debian
```bash
sudo apt-get install libsdl2-dev libsdl2-mixer-dev libpng-dev
```

#### Windows (MSYS2/MinGW)
```bash
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-libpng
```

### Building

```bash
# Build and run tests
make

# Build only (skip tests)
make build

# Run tests
make test

# Clean build artifacts
make clean
```

This produces:
- `finch_test` - Unit test executable
- `finch` - Example application executable
- `libfinch.a` - Static library

### Running the Example

```bash
./finch
```

Press 'q' or close the window to quit. The example displays a circle, diagonal lines, and randomly positioned "stars".

### Creating Your Own Application

1. Implement the required callback functions (see `finch_main.c` for reference)
2. Link against `libfinch.a` and SDL2 libraries
3. Call `FinchStartGraphics()` from your `FinchMain()` to begin

Example minimal application:

```c
#include "finch.h"

bool FinchMain(int argc, const char** argv, void** userData) {
    return FinchStartGraphics(800, 600);
}

bool FinchInit(int width, int height, void* userData) {
    return true;
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void* userData) {
    GraphicsBuffer* screen = NewGraphBuffer(pixels, width, height, width, 0);
    FillRectOpaque(screen, MakeColor(0, 0, 0), 0, 0, width, height);
    DrawCircle(screen, MakeColor(255, 255, 255), width/2, height/2, 100);
    DeleteGraphBuffer(screen);
}

void FinchUpdate(void* userData, double elapsedTicks) {}
void FinchHandleEvent(InputEvent* event, void* userData) {}
bool FinchDone(void* userData) { return false; }
void FinchCleanup(void* userData) {}
```

### Testing

Unit tests use a predicate-based verification approach. Each test:
1. Creates a temporary `GraphicsBuffer`
2. Performs drawing operations
3. Verifies every pixel matches expected values
4. Cleans up resources

Run tests with `make test` or `./finch_test`.

## License

See LICENSE file for details.
