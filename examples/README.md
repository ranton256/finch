# Finch Graphics Library Examples

This directory contains example programs that demonstrate various features of the Finch graphics library. These examples are designed to help you learn graphics programming concepts progressively, from simple visualizations to interactive animations.

## Building the Examples

From this directory, run:

```bash
make all
```

This will build all three example programs.

## Running the Examples

Each example can be run directly:

```bash
./text_demo    # Text rendering demonstration
./fern         # Barnsley fern fractal
./bounce       # Bouncing balls physics simulation
```

Or use the Makefile:

```bash
make run EXAMPLE=bounce
```

## The Examples

### 1. text_demo.c - Text Rendering

**Concepts**: Text rendering, FPS counters, mouse tracking, color usage

A comprehensive demonstration of Finch's text rendering capabilities including:
- Drawing text at fixed positions
- Centered text
- FPS counter (updates every second)
- Mouse coordinate display with crosshair
- Full character set display (ASCII 32-126)
- Multiple colors

**Controls**:
- Move mouse to see coordinates update
- Press 'q' or ESC to quit

**Learning Goals**:
- How to display text and debug information
- Calculating text dimensions for positioning
- Creating HUD (Heads-Up Display) elements
- Mouse input handling

---

### 2. fern.c - Barnsley Fern Fractal

**Concepts**: Mathematical visualization, iterated function systems, fractals, gradient colors

Creates a beautiful fractal fern using the Barnsley Fern algorithm. This demonstrates how simple mathematical rules applied repeatedly can create complex, natural-looking patterns.

The algorithm uses four affine transformations:
- **1%**: Stem (moves toward origin)
- **85%**: Main frond (largest part)
- **7%**: Left leaflet
- **7%**: Right leaflet

Each point is plotted with a gradient color that transitions from dark to light as the iteration count increases, creating depth and visual interest.

**Controls**:
- Press 'q' or ESC to quit

**Learning Goals**:
- Plotting individual pixels
- Creating mathematical visualizations
- Using randomness in algorithms
- Coordinate system transformations
- Color gradients for visual effects

---

### 3. bounce.c - Bouncing Balls

**Concepts**: Physics simulation, animation, collision detection, random generation

An interactive physics simulation featuring multiple balls bouncing around the screen. Each ball has:
- Random size (radius 25-60 pixels)
- Random starting position
- Random velocity
- Random color from a palette

The balls bounce realistically off the walls by reversing their velocity when they hit an edge.

**Controls**:
- Press 'r' to restart with new random balls
- Press 'q' or ESC to quit

**Learning Goals**:
- Animation and frame timing
- Physics simulation (velocity, position updates)
- Collision detection (wall bounces)
- Managing multiple objects
- Drawing filled and outlined circles
- Random number generation

---

## Example Complexity

The examples are ordered roughly by complexity:

1. **text_demo.c** - Simplest. Good starting point for understanding the Finch callback system and basic rendering.

2. **fern.c** - Medium complexity. Introduces mathematical visualization and point plotting. No animation or user interaction (besides quit).

3. **bounce.c** - Most complex. Demonstrates animation, physics, multiple objects, and interactive controls.

## Learning Path

We recommend studying the examples in this order:

1. Start with **text_demo.c** to understand:
   - The Finch callback model (FinchMain, FinchInit, FinchRenderProc, etc.)
   - Basic rendering and screen clearing
   - Text output for debugging

2. Move to **fern.c** to learn:
   - Pixel-level drawing
   - Mathematical transformations
   - Creating visual effects with color

3. Finally, tackle **bounce.c** to see:
   - Full game loop structure
   - Managing game state
   - Animation and timing
   - Interactive controls

## Modifying the Examples

Each example is well-commented and designed to be modified. Try:

**text_demo.c**:
- Change colors
- Add more text displays
- Display additional mouse information

**fern.c**:
- Adjust the transformation probabilities
- Change the color gradient formula
- Modify the scale or position
- Increase MAX_ITER for more detail

**bounce.c**:
- Change NUM_BALLS, MAX_SPEED, or ball sizes
- Add ball-to-ball collisions
- Add gravity
- Create trails behind balls
- Add mouse interaction

## Further Reading

- See TUTORIAL.md in the parent directory for a complete Finch programming guide
- See finch.h for full API documentation
- Visit the [project repository](https://github.com/yourusername/finch) for more information

## License

These examples are part of the Finch graphics library and are provided as educational material for learning graphics programming.
