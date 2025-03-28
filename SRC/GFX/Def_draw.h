#ifndef DEF_DRAW_H
#define DEF_DRAW_H

#include "SRC/SYS/Def_bit.h"

/* Defines related to drawing functions */

#if SCREEN_WIDTH == 320
#define SET_PIXEL(x,y,color) screen_buf[(((y)<<8)+((y)<<6)) + (x)] = color
#else
#define SET_PIXEL(x,y,color) screen_buf[(y)*SCREEN_WIDTH + (x)] = color
#endif

#if SCREEN_WIDTH == 320
#define SET_PIXEL_VGA(x,y,color) game_screen[(((y)<<8)+((y)<<6)) + (x)] = color
#else
#define SET_PIXEL_VGA(x,y,color) game_screen[(y)*SCREEN_WIDTH + (x)] = color
#endif

#define DOT_DISTANCE        30
#define LOOK_DISTANCE       30

#define MAX_PARTICLES       128
#define MAX_CORPSES         16
#define CORPSE_DIAMETER     16
#define MAX_TEMPSPRITES     32
#define NUM_ROTATIONS       12 // number of pre-rotated sprites per animation
#define ROTATION_ANGLE_STEP     (RAD_360/NUM_ROTATIONS)

#define SPRITE_IS_STATIC    0
#define SPRITE_IS_ANIM      1

#define FULLY_OUT           0
#define PARTIAL             1
#define FULLY_IN            2

#define COLOUR_WHITE        4
#define COLOUR_YELLOW       160
#define COLOUR_BLUE         200
#define TRANSPARENT_COLOR   251

#define MAX_TILE_RUNS       256
#define TILE_SIZE           20

#define TEXTURE_FLAGS_TRANSPARENCY  BIT_0
#define TEXTURE_FLAGS_LARGE_RLE     BIT_1
#define TEXTURE_FLAGS_ROWED_RLE     BIT_2

#endif
