#ifndef STR_MAP_H
#define STR_MAP_H

/* Structs related to level/map elements */

#include "SRC/GENERAL/Common.h"

typedef struct {
    uint8_t texture_id;
    uint8_t obstacle : 1;
    uint8_t block_bullets : 1;
    uint8_t is_entity : 1;
    uint8_t entity_value : 5; // id in Entities array
} Tile_t;

typedef struct
{
    int level_num; // current level number
    int width; // width in tiles
    int height; // height in tiles
    Tile_t* tilemap;
} Map_t;

typedef struct {
    uint8_t x, y, id;
    uint8_t state : 1;
    uint8_t type : 7;
    union u_data {
        struct t_door {uint8_t locked : 1; uint8_t key : 7;} door;
        struct t_button {uint8_t target;} button;
        struct t_spawner {double angle; ticks_t last_spawn_time; ticks_t spawn_time_interval;
        int8_t max_actors; uint8_t num_actors; uint16_t spawn_type: 5;
        id_type trigger_on_death: 5; uint16_t toggleable : 1; uint16_t only_once : 1;} spawner;
        struct t_trigger {ticks_t last_trigger_time; ticks_t trigger_interval; int8_t target_ids[4]; uint8_t only_once;} trigger;
        struct t_counter {uint16_t value : 5; uint16_t max_value : 5; uint16_t target_id : 5; uint16_t only_once : 1;} counter;
        struct t_portal {char level_name[30]; int x; int y; double angle;} portal;
    } data;
} Entity_t;

typedef struct {
    int index; // tilemap index
    uint8_t state : 1; // enabled/disabled
    uint8_t type : 7; // type of item from Items enum
} Item_t;

#endif /* STR_MAP_H */
