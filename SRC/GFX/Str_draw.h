#ifndef STR_DRAW_H
#define STR_DRAW_H

/* Structs related to draw functions */

#include "SRC/GENERAL/Common.h"
#include "Def_draw.h"

typedef struct
{
    char* filename;
    uint8_t* pixels;
    uint16_t width, height;
    flags_t flags;
    uint8_t material_type;
    int offset_x, offset_y;
} Texture_t;

typedef struct
{
    char* filename;
    uint8_t* pixels;
    uint16_t width, height; // original uncompressed dimensions
    uint16_t size; // compressed size for drawing routines
    flags_t flags;
    uint8_t material_type;
    int offset_x, offset_y;
} RLETexture_t;

typedef struct
{
    uint8_t* pixels;
    uint16_t width, height;
    flags_t flags;
    int offset_x, offset_y;
} RotatedTexture_t;

typedef struct
{
    Texture_t* textures;
    int texture_count;
} Texture_array;

typedef struct
{
    int frame_id;
    RotatedTexture_t* rotations;
} AnimFrame_t;

typedef struct
{
    char* name;
    AnimFrame_t* frames;
    int num_frames;
    time_type frame_interval;
} Anim_t;

typedef struct
{
    Anim_t* anims;
    int anim_count;
} Anim_array;

typedef struct
{
    int anim_ids[NUM_ACTORANIMS];
} AnimSet_t;

typedef struct
{
    uint8_t flags;
    int anim_id;
    int frame;
    time_type last_frame;
    time_type frame_interval;
} Sprite_t;

typedef struct
{
    uint8_t draw_type; // run once or until a "kill" command
    uint8_t move_type; // static or moving
    uint8_t status; // active/inactive
    Vec2 pos;
    Vec2 vel;
    double angle;
    int frame;
    Sprite_t* sprite;
} TempSprite_t;

typedef struct {
    Vec2 pos;
    Vec2 vel;
    int8_t life;
    uint8_t color;
} Particle_t;

typedef struct {
    Vec2 pos;
    int life;
    RotatedTexture_t sprite;
} Corpse_t;

#endif /* STR_DRAW_H */
