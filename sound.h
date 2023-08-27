#ifndef __SOUND__
#define __SOUND__

bool InitSound();

bool PlayMusic(const char* musicPath);


void* LoadSound(const char* soundPath);
void FreeSound(void* soundPtr);
bool PlaySound(void* soundPtr);

void CleanupSound();

#endif