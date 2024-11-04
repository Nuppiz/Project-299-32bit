#ifndef STR_VID_H
#define STR_VID_H

/* Structs related to the video system */

#include "SRC/GENERAL/Common.h"

typedef struct
{
    uint8_t r,g,b;
} Color_t;

typedef struct
{
    Color_t colors[256];
} Palette_t;

#endif /* STR_VID_H */
