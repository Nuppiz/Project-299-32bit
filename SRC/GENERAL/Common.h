#ifndef COMMON_H
#define COMMON_H

#define DEBUG_ASSERT 1

/* Common standard libraries used by multiple C files */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <stdint.h>
#include <math.h>
#include <conio.h>
#include <string.h>
#ifdef __WATCOMC__
#include <MALLOC.H>
#include <i86.h>
#else
#include <ALLOC.H>
#endif

#include "Config.h"
#include "SRC/SYS/Debug.h"
#include "Enums.h"
#include "Def_gen.h"

typedef long time_type;
typedef long ticks_t;
typedef uint16_t id_type;
typedef uint16_t flags_t;

typedef struct
{
    float x, y;
} Vec2;

typedef struct
{
    int x, y;
} Vec2_int;

typedef void (*fnp)(); // function pointer

#if DEBUG == 1
extern char debug[NUM_DEBUG][DEBUG_STR_LEN];
#endif /* COMMON_H */

#endif/* COMMON_H */
