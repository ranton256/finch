#ifndef __FINCH__
#define __FINCH__

#include "input_events.h"

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t Pixel;

typedef struct  {
	uint8_t red, green, blue;
} RGBColor24;

// our rect structure.
typedef struct {
	int32_t left, top, right, bottom;
} LSRect;

// util for min value.
inline int LSMin(int a, int b)
{
    return (a < b) ? a : b;
}

inline int LSMax(int a, int b)
{
    return (a > b) ? a : b;	
}

typedef bool LSBool;

typedef struct {
	uint32_t id; // unique id of this graph buffer.
	Pixel *ptr; // ptr to the pixels.
	uint32_t width, height; // width and height( in use/current )
	uint32_t rowPixels; // pixels per row
	uint32_t size; // size of the buffer in bytes,
		// this doesn't need to be set for everything,
		// set it to 0 if it doesn't matter in case
		// something ever needs to know, as in width, height,
		// and rowPixels won't change during the life of the buffer.
		
	LSBool ownsPtr;// true if ptr should be deleted with the buffer.
} GraphicsBuffer;



Pixel LSRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
Pixel AsPixel(RGBColor24 c);
Pixel AsPixelWithAlpha(RGBColor24 c, uint8_t alpha);


extern inline uint32_t LSCompositeValues(uint32_t a, uint32_t b, uint32_t m);
extern inline uint32_t LSCompositePixels(uint32_t sp, uint32_t dp);
extern inline uint32_t LSCompositePixelsOpaque(uint32_t sp, uint32_t dp);

// These must be defined by the code for the program using Finch.
// They are used as callbacks, but are only called after
// FinchStartGraphics() has been called.
// userData in FinchMain points to a ptr avaiable for the program's use
// which will be passed along in calls to the other functions.
bool FinchMain(int argc, const char* argv[], void** userData);
bool FinchInit(int width, int height, void* userData);
void FinchRenderProc(int width, int height, uint32_t* pixels, void *userData);
void FinchCleanup(void *userData);
void FinchHandleEvent(InputEvent* event, void* userData);
bool FinchDone(void* userData);
void FinchUpdate(void* userData, double elapsedTicks);

// Call this to start the graphics system and show the window,
// but NOTE this will start the main event processing loop,
// which will not return until FinchDone() hook returns true.
bool FinchStartGraphics(int width, int height);

// This creates a graph buffer, if you pass NULL
// for the memory parameter, it allocates it, and frees it with the buffer
// when you call DeleteGraphBuffer
GraphicsBuffer *NewGraphBuffer( 
	Pixel *memory, 
	uint32_t width,
	uint32_t height,
	uint32_t rowPixels,
	uint32_t size );

// This frees a graph buffer, and if the memory for it
// was allocated with the buffer, it frees it.
void DeleteGraphBuffer( GraphicsBuffer *buffer );

// Get byte components of pixel into *rp, *gp, and *bp
void PixelComponents(Pixel pixel, uint8_t* rp, uint8_t* gp, uint8_t* bp);

// Assign color to the pixel at (x,y) in buffer.
void PutPixel(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y);

// horizontal line drawing:  x1 has to be less or same than x2
void DrawHorzLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t x2, int32_t y);

// vertical line drawing: NOTE y1 has to be less than or same as y2
void DrawVertLine(GraphicsBuffer *buffer, Pixel color, int32_t y1, int32_t y2, int32_t x);

// General case line drawing(Bresenheim's)
void DrawLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

// Draw line but composite with source pixel based on alpha values rather than overwrite.
void DrawLineComposite(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

// rect drawing, does clip.
void DrawRect(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom);

// rect filling, does clip.
void FillRectOpaque(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom);

void DrawCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius);

void FillCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius);

// This is the blit routine, it copies the contents of one GraphicsBuffer
// into another, with transparency of course.
void BlitGraphBuffer( 
	GraphicsBuffer *srcBuffer, 
	GraphicsBuffer *destBuffer,
	int32_t xDestLoc,
	int32_t yDestLoc
	);

//  This blits a buffer into another but composites pixels based on alpha values rather than overwrite them.
void BlitGraphBufferComposite( 
	GraphicsBuffer *srcBuffer, 
	GraphicsBuffer *destBuffer,
	int32_t xDestLoc,
	int32_t yDestLoc
	);

// this checks to see if a point is in a rect.
int LSPointInRect( int x, int y, const LSRect r );
// this checks for intersection of 2 rectangles.
int IntersectRects( const LSRect r1, const LSRect r2, LSRect* sect );


#endif // __FINCH__
