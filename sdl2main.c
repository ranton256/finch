// #define SDL_MAIN_HANDLED
#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "input_events.h"
#include "sound.h"

// may not work on windows...
#ifndef __WINDOWS__
#include <unistd.h>
#else

#include <direct.h>
#include <errno.h>


#define getcwd _getcwd
#define chdir _chdir
#endif

extern int WIN_WIDTH;
extern int WIN_HEIGHT;

// These are defined by the portable code for each app/game.
bool FinchInit(int width, int height, void** userData);
void FinchRenderProc(int width, int height, uint32_t* pixels, void *userData);
void FinchCleanup(void *userData);
void FinchHandleEvent(InputEvent* event, void* userData);
bool FinchDone(void* userData);
void FinchUpdate(void* userData, double elapsedTicks);

typedef struct GameState {
    SDL_Rect windowRect;
    SDL_Window* window;
    SDL_Renderer* renderer; 

    Uint32 * pixels;
    SDL_Texture * pixelsTexture;
	void* userData;
} GameState;

bool Setup(GameState* state);
void Cleanup(GameState* state);
bool InitSDL();
SDL_Window* CreateWindow(const SDL_Rect windowRect);
SDL_Renderer* CreateRenderer(SDL_Window* window);

void Render(GameState* state);
void MainLoop(GameState* state);



bool WriteEntireTextFile(const char* path, const void* contents, long size)
{
    FILE* fp = fopen(path, "w");
    if(!fp) {
        fprintf(stderr, "Error opening file for writing %s\n", path);
        return false;
    }
    int result = fwrite(contents, size, 1, fp);
    if(result < 0) {
        fprintf(stderr, "Error writing file %s\n", path);
        fclose(fp);
        return false;
    }
    if(fclose(fp)) {
        fprintf(stderr, "Error closing file %s\n", path);
        return false;
    }
    return true;
}

#ifdef __WINDOWS__
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif

bool find_app_dir(const char* argv0, char* result, int bufSize)
{
	int slen = strlen(argv0);
   	const char *endPtr = argv0 + slen;
	result[0] = 0;

	while(endPtr > argv0) {
		if(*endPtr == PATH_SEP) {
			int dirLen = (int)( endPtr - argv0 );
			if(dirLen >= bufSize) {
				return false;
			}
			strncpy(result, argv0, dirLen);
			result[dirLen] = 0;
			return true;
		}
		endPtr--;
	}

	return false;
}

int main( int argc, char* args[] )
{
	GameState state = {0};
    
	
	char appDir[256];
	printf("starting up\n");

	find_app_dir(args[0], appDir, sizeof(appDir));

	if(true) {
		// this is for debugging path startup issues in installers.
		char wdBuf[256];
		char *cwd = (char *)getcwd(wdBuf, sizeof(wdBuf));

		char contents[512];
		snprintf(contents, sizeof(contents), "argv[0]: %s\ncwd: %s\nappdir: %s\n", args[0], cwd, appDir);
		WriteEntireTextFile("/tmp/fm_my_cwd.txt", contents, strlen(contents));
	}	

	const char* wkDir = appDir;
	if (argc > 1) {
		wkDir = args[1];
	}

	// change to same directory as binary.
	(void)chdir(wkDir);

	// If we are on mac, in an application bundle, we need to change to "../Contents"
	// This should just fail to do anything if we don't have a sibling "Resources directory"
	(void)chdir("../Resources");

    // This sets size and position of window.
    state.windowRect.x = 550;
    state.windowRect.y = 250;
    state.windowRect.w = WIN_WIDTH;
    state.windowRect.h = WIN_HEIGHT;
    
	printf("Calling Setup\n");
	if ( !Setup(&state) )
    {
        fprintf(stderr, "Setup failed.\n");
		return -1;
    }
    
	MainLoop(&state);

    Cleanup(&state);

    return 0;
}

// Poll for events, and handle the ones we care about.
bool PollEvent( InputEvent* outEvent )
{
	SDL_Event event;
	int result = SDL_PollEvent(&event);
	if(result != 0)
	{
		outEvent->modifiers = 0;
		outEvent->x = 0;
		outEvent->y = 0;

		switch (event.type) 
		{			
			case SDL_MOUSEBUTTONDOWN:
				outEvent->eventType = kInputEventType_MouseDown;
				outEvent->modifiers = (Uint32)SDL_GetModState();
				outEvent->x = event.button.x;
				outEvent->y = event.button.y;
				outEvent->button = event.button.button;
				break;
			case SDL_MOUSEBUTTONUP:
				outEvent->eventType = kInputEventType_MouseUp;
				outEvent->modifiers = (Uint32)SDL_GetModState();
				outEvent->x = event.button.x;
				outEvent->y = event.button.y;
				outEvent->button = event.button.button;
				break;
			case SDL_MOUSEMOTION:
				outEvent->eventType = kInputEventType_MouseMove;
				outEvent->modifiers = (Uint32)SDL_GetModState();
				outEvent->x = event.motion.x;
				outEvent->y = event.motion.y;
				outEvent->button = 0;
				break;
			case SDL_KEYDOWN:
				outEvent->eventType = kInputEventType_KeyDown;
				outEvent->scanCode = event.key.keysym.scancode;
				outEvent->keyCode = event.key.keysym.sym;
				outEvent->modifiers = (Uint32)event.key.keysym.mod;
				break;
			case SDL_KEYUP:
				outEvent->eventType = kInputEventType_KeyUp;
				outEvent->scanCode = event.key.keysym.scancode;
				outEvent->keyCode = event.key.keysym.sym;
				outEvent->modifiers = (Uint32)event.key.keysym.mod;
				break;
			case SDL_QUIT:
				outEvent->eventType = kInputEventType_Quit;
				break;
			default:
				// do nothing
				outEvent->eventType = kInputEventType_Nothing;
				result = false;
		}
	}
	
	return result != 0;
}

const int MIN_TICKS_PER_FRAME = 1;

void MainLoop(GameState* state)
{
	Render(state);

	double elapsed, ticks, lastTicks = SDL_GetTicks();
    do
    {
        InputEvent evt;
        while( PollEvent( &evt ) ) {
			FinchHandleEvent(&evt, state->userData);
 		}

		SDL_Delay(1); // save CPU
		
		Render(state);

		ticks = SDL_GetTicks();
		elapsed = ticks - lastTicks;
		if(elapsed > MIN_TICKS_PER_FRAME) {
			FinchUpdate(state->userData, elapsed);
			lastTicks = ticks;
		}
    }
    while(!FinchDone(state->userData));
}

void Cleanup(GameState* state)
{
	FinchCleanup(state->userData);
	CleanupSound();

    if(state->pixels)
        free(state->pixels);
    if(state->pixelsTexture)
        SDL_DestroyTexture(state->pixelsTexture);
}


void Render(GameState* state)
{
	// Clear the window
	SDL_RenderClear( state->renderer );
	
    const int width = state->windowRect.w;
    const int height = state->windowRect.h;
    uint32_t* pixels = state->pixels;
	
	FinchRenderProc(width, height, pixels, state->userData);

    SDL_UpdateTexture(state->pixelsTexture, NULL, pixels, width * sizeof(Uint32));
	SDL_RenderCopy(state->renderer, state->pixelsTexture, NULL, NULL);
	SDL_RenderPresent( state->renderer);
}

bool Setup(GameState* state)
{
	if ( !InitSDL(SDL_INIT_AUDIO) )
		return false;

	state->window =CreateWindow(state->windowRect);
	if ( !state->window )
		return false;

    state->renderer = CreateRenderer(state->window);
	if ( !state->renderer )
		return false;
    
    SDL_RenderSetLogicalSize( state->renderer, state->windowRect.w, state->windowRect.h );
    SDL_SetRenderDrawColor( state->renderer, 255, 0, 0, 255 );
    
    const int width = state->windowRect.w;
    const int height = state->windowRect.h;

    state->pixels = (Uint32*)malloc(sizeof(Uint32) * width * height);
	if(!state->pixels) {
		fprintf(stderr, "not enough memory for pixel buffer\n");
		return false;
	}

    state->pixelsTexture = SDL_CreateTexture(state->renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
	
	return FinchInit(width, height, &state->userData);
}

bool InitSDL()
{
	if ( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
	{
		fprintf(stderr,"Error initializing SDL: %s\n", SDL_GetError());
		return false;
	}

	InitSound();
	return true;
}

SDL_Window* CreateWindow(const SDL_Rect windowRect)
{
	SDL_Window* window = SDL_CreateWindow( "Server", windowRect.x, windowRect.y, windowRect.w, windowRect.h, 0 );
	if ( window == NULL )
	{
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
	}

	return window;
}

SDL_Renderer* CreateRenderer(SDL_Window* window)
{
	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if ( renderer == NULL )
	{
		fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
	}

	return renderer;
}
	
