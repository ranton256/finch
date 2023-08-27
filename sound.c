#include "SDL.h"
#include "SDL_mixer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static Mix_Music *music = NULL;
static bool audio_open = false;

const int DONT_LOOP = 0;
const int LOOP_FOREVER = -1;
const int FADE_MS = 500;
const int SOUND_VOLUME = MIX_MAX_VOLUME / 4;
const int MUSIC_VOLUME = MIX_MAX_VOLUME / 2;

bool InitSound()
{
    int result = 0;
    int flags = MIX_INIT_MP3;

    if (flags != (result = Mix_Init(flags))) {
        fprintf(stderr, "Could not initialize mixer (result: %d).\n", result);
        fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
        return false;
    }

    int ret = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 640); ///  int chunksize);)
    if(ret < 0) {
        fprintf(stderr, "Error opening audio mixer\n");
        fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    } else {
        audio_open = true;
    }

    ret = Mix_VolumeMusic(MUSIC_VOLUME);
    if(ret < 0) {
        fprintf(stderr, "Error setting volume\n");
        fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    }

    return true;
}

bool PlayMusic(const char* musicPath)
{
    Mix_Music *music = Mix_LoadMUS(musicPath);
    if(!music) {
        fprintf(stderr, "Could not load background music from %s\n", musicPath);
        fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    }
    if(Mix_FadeInMusic(music, LOOP_FOREVER, FADE_MS)) {
        fprintf(stderr, "Could not play background music\n");
        fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    }

    return true;
}

void* LoadSound(const char* soundPath)
{
 	Mix_Chunk* sound = Mix_LoadWAV(soundPath);
	if ( !sound ) {
		fprintf(stderr, "Could not load sound\n");
        fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
	}

    return sound;
}
   
void FreeSound(void* soundPtr)
{
    Mix_Chunk* sound = (Mix_Chunk*) soundPtr;
    if(!soundPtr) return; 
    Mix_FreeChunk( sound );
}

bool PlaySound(void* soundPtr)
{
    Mix_Chunk* sound = (Mix_Chunk*) soundPtr;
    if(!soundPtr) return false;

    int channel = Mix_PlayChannel( -1, sound, DONT_LOOP );
    if(channel < 0) {
        fprintf(stderr, "Could not play sound\n");
        fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    } else {
        (void) Mix_Volume(channel, SOUND_VOLUME);
    }

    return true;
}

void CleanupSound()
{
    if(music) {
        Mix_FreeMusic(music);
        music = NULL;
    }

    if(audio_open) {
        Mix_Quit();
    }
    
}
