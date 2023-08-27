#ifndef __SUPPORT__
#define __SUPPORT__

#include <assert.h>

#ifdef DEBUG

#define DbgAssert(x) assert(x)
#else
#define DbgAssert(x)
#endif

#endif
