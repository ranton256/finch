#include "finch.h"
#include "font.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Common color constants (0xAARRGGBB format)
const Pixel COLOR_WHITE       = 0xFFFFFFFF;
const Pixel COLOR_BLACK       = 0xFF000000;
const Pixel COLOR_RED         = 0xFFFF0000;
const Pixel COLOR_GREEN       = 0xFF00FF00;
const Pixel COLOR_BLUE        = 0xFF0000FF;
const Pixel COLOR_YELLOW      = 0xFFFFFF00;
const Pixel COLOR_CYAN        = 0xFF00FFFF;
const Pixel COLOR_MAGENTA     = 0xFFFF00FF;
const Pixel COLOR_GRAY        = 0xFF808080;
const Pixel COLOR_DARK_GRAY   = 0xFF404040;
const Pixel COLOR_LIGHT_GRAY  = 0xFFC0C0C0;

static uint32_t sLastBufferID = 0;

typedef uint32_t CompositePixelsProc(uint32_t sp, uint32_t dp);


extern inline Pixel LSRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	uint8_t pp[4];
	pp[0] = r;
	pp[1] = g;
	pp[2] = b;
	pp[3] = a;
	return *(Pixel *)pp;
}

extern inline Pixel AsPixel(RGBColor24 c)
{
    return LSRGBA(c.red, c.green, c.blue, 255);
}

extern inline Pixel AsPixelWithAlpha(RGBColor24 c, uint8_t alpha)
{
    return LSRGBA(c.red, c.green, c.blue, alpha);
}

extern inline int LSMin(int a, int b);
extern inline int LSMax(int a, int b);

// This creates a graph buffer, if you pass NULL
// for the memory parameter, it allocates it, and frees it with the buffer
// when you call DeleteGraphBuffer
GraphicsBuffer *NewGraphBuffer( 
	Pixel *memory, 
	uint32_t width,
	uint32_t height,
	uint32_t rowPixels,
	uint32_t size )
{	
	uint32_t id = ++sLastBufferID;
	
	assert(sizeof(Pixel) == 4);

	GraphicsBuffer *buffer = (GraphicsBuffer*)malloc(sizeof(GraphicsBuffer));
	if( !buffer )
    {
        fprintf(stderr,"Not enough memory for graphics buffer.\n");
        return NULL;
    }

	// assign a unique id.
	buffer->id = id;
	
	if( !memory  )
	{
		assert(size >= sizeof(Pixel) * width * height);
		buffer->ptr = (Pixel*)malloc(size);
		buffer->ownsPtr = true;
		if( !(buffer->ptr ) ) {
            fprintf(stderr,"Not enough memory for graphics buffer pixels.\n");
            free(buffer);
            return NULL;
        }
		memset(buffer->ptr, 0, size);
	}
	else
	{
		buffer->ptr = memory;
		buffer->ownsPtr = false;
	}
	buffer->width = width;
	buffer->height = height;
	buffer->rowPixels = rowPixels;
	buffer->size = size;
	return buffer;	
}

// This frees a graph buffer, and if the memory for it
// was allocated with the buffer, it frees it.
void DeleteGraphBuffer( GraphicsBuffer *buffer )
{
	// log deletion
	if( buffer )
	{
		if( buffer->ptr && buffer->ownsPtr)
			free(buffer->ptr);

		free(buffer);
	}
}

void PixelComponents(Pixel pixel, uint8_t* rp, uint8_t* gp, uint8_t* bp)
{
	assert(rp); assert(gp); assert(bp);
	*rp = (pixel >> 16) & 0xff;
	*gp = (pixel >> 8) & 0xff;
	*bp = (pixel >> 0) & 0xff;
}

// this composites to 8 bit channel values using an alpha
uint32_t LSCompositeValues(uint32_t a, uint32_t b, uint32_t m)
{
    // >> 8 instead of 255 causes too much rounding error
    // when you mask several images in a row.
	return (m*(a-b) + 255*b) / 255;
}


// arguments are source pixel, current dest pixel.
uint32_t LSCompositePixels(uint32_t sp, uint32_t dp)
{
	uint8_t np[4],*bsrc,*bdst;
	bsrc=(uint8_t*)&sp;
	bdst=(uint8_t*)&dp;
	uint32_t  mask = bsrc[3];
	np[0] = LSCompositeValues( bsrc[0], bdst[0], mask );
	np[1] = LSCompositeValues( bsrc[1], bdst[1], mask );
	np[2] = LSCompositeValues( bsrc[2], bdst[2], mask );
	np[3] = bdst[3];

	return *(uint32_t *)np;
}



inline uint32_t LSCompositePixelsOpaque(uint32_t sp, uint32_t dp)
{
	return sp;
}

void ClearBuffer(GraphicsBuffer* buffer, Pixel color)
{
	FillRectOpaque(buffer, color, 0, 0, buffer->height, buffer->width);
}

void PutPixel(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y)
{
	if( x < 0 || (uint32_t)x >= buffer->width ||
        y < 0 || (uint32_t)y >= buffer->height) {
        return; // out of bounds
    }
    buffer->ptr[y * buffer->rowPixels + x] = color;
}

Pixel GetPixel(GraphicsBuffer* buffer, int32_t x, int32_t y)
{
    if( x < 0 || (uint32_t)x >= buffer->width ||
        y < 0 || (uint32_t)y >= buffer->height) {
        return 0; // out of bounds
    }
    return buffer->ptr[y * buffer->rowPixels + x];
}


// horizontal line drawing:  x1 has to be less or same than x2
void DrawHorzLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t x2, int32_t y)
{
	// check that the line is on the screen
	if( x1 <= x2 && y >= 0 && y < buffer->height && x2 >= 0 && x1 < (int32_t)buffer->width)
	{
		// clip x coordinates to the screen.
		if( x1 < 0 )
			x1 = 0;
		if( x2 >= buffer->width )
			x2 = buffer->width - 1;
		
		// get the buffer.
		uint32_t rowPixels = buffer->rowPixels;
		Pixel *pix = buffer->ptr;
		pix += y * rowPixels;
		pix += x1;
		int32_t count = x2 - x1 + 1;
		while(count--)
		{
			Pixel pval = *pix;
			*pix++ = LSCompositePixels( color, pval );
		}
		// that's it.
	}
}


// vertical line drawing: NOTE y1 has to be less than or same as y2
void DrawVertLine(GraphicsBuffer *buffer, Pixel color, int32_t y1, int32_t y2, int32_t x)
{
	// check that the line is on the screen
	if( y1 <= y2 && x >= 0 && x < buffer->width && y2 >= 0 && y1 < buffer->height )
	{
		if( y1 < 0 )
			y1 = 0;
		if( y2 >= buffer->height )
			y2 = buffer->height - 1;

		uint32_t rowPixels= buffer->rowPixels;
		Pixel *pix = buffer->ptr;//LSGetDoubleBuffer(&rowPixels);
		pix += y1 * rowPixels;
		pix += x;
		int32_t count = y2 - y1 + 1;
		while(count--)
		{
			*pix = LSCompositePixels( color, *pix );
			pix += rowPixels;
		}
	}
}

// Bresenham's line drawing algorithm with alpha compositing
// Handles all 8 octants by classifying lines into 4 cases based on slope and direction
static void LSDrawLineCB(GraphicsBuffer *buffer, Pixel color, CompositePixelsProc compositeFunc, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	// These constants are only used inside this function.
	const unsigned char LineBigSlope = 2;
	const unsigned char LineGoesBackwards = 1;
	const unsigned char LineIsStandardX = 0;
	
	uint32_t rowPixels = buffer->rowPixels;
	Pixel dp, *dest = buffer->ptr;
	
	// starting point , finishing point for the internals....
	// NOTE:: not necessarily the values passed, possible of function of the 
	// passed variables for internals only
	long mStartX;
	long mStartY;
	long mStopX;
	long mStopY;
	
	// internal calculation and optimization variables
	long mAbsDx;
	long mAbsDy;
	long mDy2;
	long mDyMinusDx2;
	long mPreference;
	
	// there are four main cases
	unsigned char mSlopeCase;
	
	// This is the current point.
	long mCurrentX;
	long mCurrentY;

	mStartX = x1;
	mStartY = y1;
	mStopX = x2;
	mStopY = y2;	
	
	mAbsDx =  mStartX - mStopX;
	mAbsDy =  mStartY - mStopY;

	if(mAbsDx < 0)
		mAbsDx = -mAbsDx;
	if(mAbsDy < 0)
		mAbsDy = -mAbsDy;
		
	mSlopeCase = LineIsStandardX;
	
	if(mAbsDy > mAbsDx)
	{ // Go the y route
		long swap;
		mSlopeCase |= LineBigSlope;
		
		// Swap the x and y values of the starting pixel.
		swap = mStartX;
		mStartX = mStartY;
		mStartY = swap;
		
		// Swap the x and y values of the ending pixel.
		swap = mStopX;
		mStopX = mStopY;
		mStopY = swap;
		
		// Swap the slopes.
		swap = mAbsDx;
		mAbsDx = mAbsDy;
		mAbsDy = swap;

	}

	if(mStartX > mStopX)
	{
		mSlopeCase |= LineGoesBackwards;
		
		// Make the x values negative
		mStartX *= -1;
		mStopX *= -1;
	}

	// Set up for the first point
	mCurrentX = mStartX;
	mCurrentY = mStartY;
	
	
	// calculate the pertinent values
	mDy2 = mAbsDy * 2;
	mDyMinusDx2 = 2 * (mAbsDy - mAbsDx);
	mPreference = mDy2 - mAbsDx;
	
	// Done with the setup, now loop.
	// All 4 cases have identical loop logic, only coordinate transformation differs
	long outPointX, outPointY;

	while(1)
	{
		// Transform coordinates based on case
		switch(mSlopeCase)
		{
			case 0: // LineIsStandardX
				outPointX = mCurrentX;
				outPointY = mCurrentY;
				break;
			case 1: // LineGoesBackwards
				outPointX = -mCurrentX;
				outPointY = mCurrentY;
				break;
			case 2: // LineBigSlope
				outPointX = mCurrentY;
				outPointY = mCurrentX;
				break;
			case 3: // LineBigSlope & LineGoesBackwards
				outPointX = mCurrentY;
				outPointY = -mCurrentX;
				break;
		}

		// Check if we're done
		if(mCurrentX >= mStopX)
		{
			return;
		}

		// Put the pixel with clipping
		if( outPointX < buffer->width
			&& outPointY < buffer->height
			&& outPointX >= 0
			&& outPointY >= 0
			)
		{
			dp = dest[ rowPixels*outPointY + outPointX ];
			dest[ rowPixels*outPointY + outPointX ] = compositeFunc( color, dp );
		}

		// Advance to next pixel using Bresenham
		mCurrentX++;
		if(mPreference < 0)
		{
			mPreference += mDy2;
		}
		else
		{
			if(mStartY < mStopY)
				mCurrentY++;
			else
				mCurrentY--;
			mPreference += mDyMinusDx2;
		}
	}
}

// General case line drawing(Bresenheim's)
void DrawLineComposite(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	LSDrawLineCB(buffer, color, LSCompositePixels, x1, y1, x2, y2);
}

void DrawLine(GraphicsBuffer *buffer, Pixel color, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	LSDrawLineCB(buffer, color, LSCompositePixelsOpaque, x1, y1, x2, y2);
}


// rect drawing, does clip.
void DrawRect(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	// Normalize inverted rectangles
	if( left > right ) {
		int32_t temp = left;
		left = right;
		right = temp;
	}
	if( top > bottom ) {
		int32_t temp = top;
		top = bottom;
		bottom = temp;
	}

	// not normal for rects to include
	// the pixels on the right and bottom edge,
	// but the functions we call do, so dec them.
	bottom--;
	right--;

	DrawHorzLine(buffer, color, left, right, top );
	DrawHorzLine(buffer, color, left, right, bottom );

	// don't draw the 4 corner pixels twice.
	top++;
	bottom--;
	if( top <= bottom )
	{
		DrawVertLine(buffer, color,top,bottom,left);
		DrawVertLine(buffer, color,top,bottom,right);
	}

}

// rect filling, does clip.
void FillRectOpaque(GraphicsBuffer *buffer, Pixel color, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
	Pixel *pix;
	int32_t hCount;
	Pixel *row;
	int32_t width, height;
	uint32_t rowPixels = buffer->rowPixels;

	// Normalize inverted rectangles
	if( left > right ) {
		int32_t temp = left;
		left = right;
		right = temp;
	}
	if( top > bottom ) {
		int32_t temp = top;
		top = bottom;
		bottom = temp;
	}

	// check that it is on the screen
	if( bottom >= 0 && top < buffer->height && right >= 0 && left < buffer->width)
	{
		// clip.
		if( left < 0 )
			left = 0;
		if( top < 0 )
			top = 0;
		if( right > buffer->width )
			right = buffer->width; // - 1
		if( bottom > buffer->height )
			bottom = buffer->height; // - 1

		height = bottom - top;
		width = right - left;
		row = buffer->ptr;
		row += top * rowPixels; // go to the top.
		row += left; // go to the left side.
		while( height-- )
		{
			pix = row;
			hCount = width;
			while(hCount--)
				*pix++ = color;
			
			row += rowPixels;
		}
	}
}

// no transparency version
#define LSSetPixelOpaque(buffer,x,y,color) \
	if(x<buffer->width && y<buffer->height && x>=0 && y>=0) \
		{ buffer->ptr[(buffer->rowPixels)*(y)+(x)] = (color); }

// handles transparency
#define LSSetPixel(buffer,x,y,color) \
	if(x<buffer->width && y<buffer->height && x>=0 && y>=0) \
		{ buffer->ptr[(buffer->rowPixels)*(y)+(x)] = LSCompositePixels((color), buffer->ptr[(buffer->rowPixels)*(y)+(x)]); }



// This is a macro used by LSDrawCircle
#define _PlotCirclePoints(x,y)  \
	LSSetPixel(buffer, xCenter + x, yCenter + y, color ); \
	LSSetPixel(buffer, xCenter - x, yCenter + y, color ); \
	LSSetPixel(buffer, xCenter + x, yCenter - y, color ); \
	LSSetPixel(buffer, xCenter - x, yCenter - y, color ); \
	LSSetPixel(buffer, xCenter + y, yCenter + x, color ); \
	LSSetPixel(buffer, xCenter - y, yCenter + x, color ); \
	LSSetPixel(buffer, xCenter + y, yCenter - x, color ); \
	LSSetPixel(buffer, xCenter - y, yCenter - x, color )



// Draw a circle using midpoint circle algorithm (Bresenham-style)
// Calculates one octant and uses 8-way symmetry to plot all 8 octants
void DrawCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius)
{
	// Midpoint circle algorithm:
	// f_circle(x,y) = x*x + y*y - r*r
	// negative if interior, positive if outside, 0 if on boundary.
	
	// we do an increment using 0,0 as origin,
	// for each next pixel in the calc. octant from x = 0, to x= y
	// we determine if the next pixel should be 
	// after xk,yk, the choices are xk+1,yk and xk+1,yk-1
	// We decide based on the circle function above evaluated
	// for the midpoint between these points.
	
	
	// pk(decision parameter) = f_circle(xk + 1, yk - 1/2)
	// = (xk + 1)^2 + (yk-1/2)^2 - r^2
	
	// if pk is negative yk is closer to circle boundary,
	// otherwise the mid-position is outside or on the pixel boundary
	// and we select the pixel on scanline yk-1
	
	// the incremental version of pk is
	// pk+1 = f_circle( xk+1, yk+1 - 1/2)
	// = [ (xk + 1) + 1  ]^2 + (yk+1 - 1/2)^2 - r^2
	// OR
	// pk+1 = pk + 2(xk + 1) + ( (yk+1)^2 - yk^2) - (yk+1 - yk) + 1
	
	// uint32_t rowPixels = buffer->rowPixels;
	// Pixel *dest = buffer->ptr;
	
	
	int32_t p, x, y;

	x = 0;
	y = radius;
	// plot the first points.
	_PlotCirclePoints( x, y );
	p = 1 - radius;
	while( x < y )
	{
		if( p < 0 )
		{
			x++;
			p = p + 2 * x + 1;
		}
		else
		{
			x++;
			y--;
			p = p + 2 *(x-y) + 1;
		}
		_PlotCirclePoints( x, y );
	}
}

void FillCircle(GraphicsBuffer *buffer, Pixel color, int32_t xCenter, int32_t yCenter, int32_t radius)
{
	int32_t p, x, y;
	
	x = 0;
	y = radius;
	// Draw the first line.
	DrawHorzLine(buffer, color, xCenter - x, xCenter + x, yCenter + y );
	DrawHorzLine(buffer, color,  xCenter - x, xCenter + x, yCenter - y );
	DrawHorzLine(buffer, color,  xCenter - y, xCenter + y, yCenter + x );
	DrawHorzLine(buffer, color,  xCenter - y, xCenter + y, yCenter - x );

	p = 1 - radius;
	while( x < y )
	{
		if( p < 0 )
		{
			x++;
			p = p + 2 * x + 1;
		}
		else
		{
			x++;
			y--;
			p = p + 2 *(x-y) + 1;
		}
		
		// 4 lines each time
		DrawHorzLine(buffer, color, xCenter - x, xCenter + x, yCenter + y );
		DrawHorzLine(buffer, color,  xCenter - x, xCenter + x, yCenter - y );
		DrawHorzLine(buffer, color,  xCenter - y, xCenter + y, yCenter + x );
		DrawHorzLine(buffer, color,  xCenter - y, xCenter + y, yCenter - x );
		
	}
}



// This is a version of the blit routine that doesn't use alpha blending
void BlitGraphBuffer( 
	GraphicsBuffer *srcBuffer, 
	GraphicsBuffer *destBuffer,
	int32_t xDestLoc,
	int32_t yDestLoc
	)
{
	Pixel *src = srcBuffer->ptr;
	Pixel *dest = destBuffer->ptr;
	Pixel *srcRow;
	Pixel *destRow;
	
	int32_t rows, cols;
	LSRect clip;
	
	if( xDestLoc >= destBuffer->width ||
		yDestLoc >= destBuffer->height )
	{
		return;
	}

	if( xDestLoc < 0 )
	{
		src += (-xDestLoc);
		clip.left = 0;
	}
	else
		clip.left = xDestLoc;

	if( yDestLoc < 0 )
	{
		src += (-yDestLoc) * srcBuffer->rowPixels;
		clip.top = 0;
	}
	else
		clip.top = yDestLoc;
	
	clip.right = xDestLoc + srcBuffer->width;
		
	if( clip.right > destBuffer->width )
		clip.right = destBuffer->width;
	
	clip.bottom = yDestLoc + srcBuffer->height;
		
	if( clip.bottom > destBuffer->height )
		clip.bottom = destBuffer->height;
	
	if( clip.right <= 0 ||
		clip.bottom <= 0 )
	{
		return;
	}
	
	// clip it if clip is non-null
	
	rows = clip.bottom - clip.top;
	cols = clip.right - clip.left;
	
	// go to where we start.
	dest += clip.top * destBuffer->rowPixels + clip.left;
	
	while(rows--)
	{
		srcRow = src;
		destRow = dest;
		
		// no transparency version
		int32_t c = cols;
		while(c--)
			*destRow++ = *srcRow++;
		
		// next row.
		dest += destBuffer->rowPixels;
		src += srcBuffer->rowPixels;
	}
	
}



void BlitGraphBufferComposite( 
	GraphicsBuffer *srcBuffer, 
	GraphicsBuffer *destBuffer,
	int32_t xDestLoc,
	int32_t yDestLoc
	)
{
	Pixel *src = srcBuffer->ptr;
	Pixel *dest = destBuffer->ptr;
	Pixel *srcRow;
	Pixel *destRow;
	
	int32_t rows, cols;
	LSRect clip;
	
	if( xDestLoc >= (int32_t)destBuffer->width ||
		yDestLoc >= (int32_t)destBuffer->height )
	{
        return;
	}
	if( (int32_t)yDestLoc <= -(int32_t)srcBuffer->height ||
		(int32_t)xDestLoc <= -(int32_t)srcBuffer->width )
	{
		return;
	}

	if( xDestLoc < 0 )
	{
		src += (-xDestLoc);
		clip.left = 0;
	}
	else
		clip.left = xDestLoc;

	if( yDestLoc < 0 )
	{
		src += (-yDestLoc) * srcBuffer->rowPixels;
		clip.top = 0;
	}
	else
		clip.top = yDestLoc;
	
	clip.right = xDestLoc + srcBuffer->width;
		
	if( clip.right > destBuffer->width )
		clip.right = destBuffer->width;
	
	clip.bottom = yDestLoc + srcBuffer->height;
		
	if( clip.bottom > destBuffer->height )
		clip.bottom = destBuffer->height;
	
	if( clip.right <= 0 ||
		clip.bottom <= 0 )
	{
		return;
	}
	
	// clip it if clip is non-null
	
	rows = clip.bottom - clip.top;
	cols = clip.right - clip.left;
	
	// go to where we start.
	dest += clip.top * destBuffer->rowPixels + clip.left;
	
	while(rows--)
	{
		srcRow = src;
		destRow = dest;
		
		// changed to deal with alpha in 32bit color.
		int32_t c = cols;
		while(c--)
		{
			uint32_t sp, dp;
			
			sp = *srcRow++;
			dp = *destRow;
			
			*destRow++ = LSCompositePixels( sp, dp );
		}
		
		// next row.
		dest += destBuffer->rowPixels;
		src += srcBuffer->rowPixels;
	}
	
}



// this checks to see if a point is in a rect.
int LSPointInRect( int x, int y, const LSRect r )
{
	if( 
		x >= r.left &&
		x < r.right &&
		y >= r.top &&
		y < r.bottom )
			return true;
	else
		return false;
}

// this checks for intersection of 2 rectangles.
int IntersectRects( const LSRect r1, const LSRect r2, LSRect* sect )
{
	if( 
		LSPointInRect( r1.left, r1.top, r2 ) ||
		LSPointInRect( r1.left, r1.bottom, r2 ) ||
		LSPointInRect( r1.right, r1.top, r2 ) ||
		LSPointInRect( r1.right, r1.bottom, r2 ) ||
		// this takes care of case where one entirely fits
		// inside another.
		LSPointInRect( r2.left, r2.top, r1 )
		
		)
		{
			sect->left = LSMax( r1.left, r2.left );
			sect->top = LSMax( r1.top, r2.top );
			sect->right = LSMin( r1.right, r2.right );
			sect->bottom = LSMin( r1.bottom, r2.bottom );

			return true;
		}
	else
		return false;
}

// ============================================================================
// TEXT RENDERING
// ============================================================================

void DrawChar(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y, char c)
{
	// Check if character is in supported range
	if (c < FONT_FIRST_CHAR || c > FONT_LAST_CHAR) {
		return; // Unsupported character, just skip it
	}

	// Get the font data for this character
	int charIndex = c - FONT_FIRST_CHAR;
	const uint8_t* charData = font8x8_basic[charIndex];

	// Draw each row of the character
	for (int row = 0; row < FONT_CHAR_HEIGHT; row++) {
		uint8_t rowData = charData[row];

		// Draw each pixel in the row
		for (int col = 0; col < FONT_CHAR_WIDTH; col++) {
			// Check if this pixel should be drawn (MSB is leftmost)
			if (rowData & (1 << (7 - col))) {
				PutPixel(buffer, color, x + col, y + row);
			}
		}
	}
}

void DrawText(GraphicsBuffer* buffer, Pixel color, int32_t x, int32_t y, const char* text)
{
	if (!text) return;

	int currentX = x;
	for (const char* p = text; *p != '\0'; p++) {
		DrawChar(buffer, color, currentX, y, *p);
		currentX += FONT_CHAR_WIDTH;
	}
}

int GetTextWidth(const char* text)
{
	if (!text) return 0;

	int length = 0;
	for (const char* p = text; *p != '\0'; p++) {
		length++;
	}

	return length * FONT_CHAR_WIDTH;
}

int GetTextHeight(void)
{
	return FONT_CHAR_HEIGHT;
}

void DrawTextCentered(GraphicsBuffer* buffer, Pixel color, int32_t centerX, int32_t centerY, const char* text)
{
	int width = GetTextWidth(text);
	int height = GetTextHeight();

	int x = centerX - width / 2;
	int y = centerY - height / 2;

	DrawText(buffer, color, x, y, text);
}
