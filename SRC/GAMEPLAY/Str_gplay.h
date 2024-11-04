#ifndef STR_GPLAY_H
#define STR_GPLAY_H

/* Structs related to gameplay initialisation and basic elements */

#include "SRC/FILEAXS/Str_map.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/GFX/Str_draw.h"

typedef struct
{
    char* name;
    float walk_speed;
    float run_speed;
    float turn_rate;
    int radius; // collision box radius
    int health;
    id_type primary_weapon_id;
    id_type secondary_weapon_id;
    AnimSet_t animset;
} ActorTemplate_t;

typedef struct
{
    id_type     id;
    flags_t  flags;
    Vec2     position;
    Vec2_int grid_loc;  // location on the grid
    Vec2     direction; // direction
    double   angle;     // current direction in radians
    Vec2     velocity;  // velocity
    float    magnitude; // magnitude of velocity
    int      radius;    // collision box/circle radius
    int health;
    
    flags_t control;
    uint8_t ai_mode;
    ticks_t ai_timer;
    id_type    target_id_primary; // index number of the AI's primary target in the ActorsById array
    id_type    target_id_secondary; // index number of the AI's secondary target in the ActorsById array
    Vec2    move_target;

    id_type texture_id;
    id_type trigger_on_death; // entity ID to trigger on death
    ticks_t last_shot; // last shot taken
    int primary_weapon_id;
    int secondary_weapon_id;
    Sprite_t sprite;
    AnimSet_t* animset;
} Actor_t;

typedef struct
{
    Map_t Map;
    char current_level_name[15];
    Actor_t* Actors;
    id_type* ActorsById;
    id_type actor_count;
    id_type actor_capacity;
    id_type id_capacity;
    uint8_t item_count;
    uint8_t item_capacity;
    id_type player_id;
} GameData_t;

#endif /* STR_GPLAY_H */
