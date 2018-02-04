#pragma once
#define $(...)
#define REFLECTED
#define VaradicArgs ...
#define VariadicArgs ...

//wtf vs2017????
#ifndef NULL
#define NULL ((void*)0)
#endif 

$(exclude)
#ifndef PREPROCESSOR
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>

#ifdef WB_DEBUG
#define wb_assert(condition, msg, ...) do { \
	if(!(condition)) { \
		log_error(msg, __VA_ARGS__); \
		__debugbreak(); \
	} \
} while(0)
#else 
#define wb_assert(condition, msg, ...)
#endif

#define log_error(fmt, ...) do { \
	char buf[4096]; \
	snprintf(buf, 4096, fmt, __VA_ARGS__); \
	fprintf(stderr, "%s \n", buf); \
} while(0)

typedef int bool;
#define true 1
#define false 0

#define MouseLeft SDL_BUTTON_LEFT
#define MouseRight SDL_BUTTON_RIGHT

#include "thirdparty.h"
#endif


#include "wb_platform.h"

#ifdef WB_WINDOWS
#include "wb_win32.c"
#endif

#ifdef WB_LINUX
#include "wb_linux.c"
#endif

#include "vmf_generated.h"

$(end)

typedef const char* string;

#define Max(x, y) ((x)>(y)?(x):(y))
#define Min(x, y) ((x)>(y)?(y):(x))

#define Math_Sqrt2 1.414213f
#define Math_InvSqrt2 0.7071067f
#define Math_Pi 3.141592f
#define Math_Tau 6.283185f
#define Math_RadToDeg 57.29577f
#define Math_DegToRad 0.01745329f

#define isizeof(x) (isize)sizeof(x)

