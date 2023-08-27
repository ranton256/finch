#include "finch.h"
#include "blit.h"
#include "input_events.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


static int WIN_WIDTH = 1024;
static int WIN_HEIGHT = 768;
static bool sDone = false;

bool FinchMain(int argc, const char** argv, void** userData)
{
    if(userData) {
        *userData = NULL;
        // we don't have any userdata at the moment.
    }
    
    printf("Main\n");
    return FinchStartGraphics(WIN_WIDTH, WIN_HEIGHT);
}

bool FinchInit(int width, int height, void* userData)
{
    printf("Init\n");
    return true;
}

void FinchCleanup()
{
    printf("Cleanup\n");
}

bool FinchDone(void* userData)
{
    return sDone;
}

void FinchRenderProc(int width, int height, uint32_t* pixels, void *userData)
{
    GraphicsBuffer *screen = NewGraphBuffer(pixels, width, height, width, 0);
    if(!screen) { // yikes!
        fprintf(stderr, "Error setting up screen settings for render.");
        return;
    }
    
    FillRectOpaque(screen, 0, 0, 0, width, height);
    DrawCircle(screen, MakeColor(255,128,0), width/2, height/2, (width + height)/8);
    DrawLine(screen, MakeColor(0, 30, 220), 10,       10,  width-10, height-10);
    DrawLine(screen, MakeColor(0, 30, 220), width-10, 10, 10,        height-10);

    // draw some "stars"
    #define NUM_STARS 80
    #define STAR_SEED 797

    srand(STAR_SEED);
    for(int i = 0; i < NUM_STARS; i++) {
        int x = rand() % width;
        int y = rand() % height;
        PutPixel(screen, MakeColor(220,230,250), x, y);
    }

    DeleteGraphBuffer(screen);
}

void FinchUpdate(void* userData, double elapsedTicks)
{
    // ....
}

void FinchHandleEvent(InputEvent* event, void* userData)
{
    if (event->eventType == kInputEventType_Quit ||
        (event->eventType == kInputEventType_KeyDown && event->keyCode == 'q') )
    {
        printf("Quitting...\n");
        sDone = true;
        return;
    }

    if(event->eventType == kInputEventType_MouseDown) {
        printf("click!\n");
    }

    if(event->eventType == kInputEventType_KeyDown)
    {
        if(isprint(event->keyCode)) {
            printf("you pressed %c\n", (char)event->keyCode);
        } else {
            printf("you pressed: code  %d\n", (int)event->keyCode);
        }
    }
}

