#ifndef DEF_GAME_H
#define DEF_GAME_H

/* Defines related to gameplay initialisation and actions */

#define OBJ_DEFAULT_CAPACITY   16
#define ACTOR_CHUNK_SIZE       16

#define OBJ_FLAG_DEAD          BIT_0

#define MAX_PROJECTILES        64
#define BULLET_STEP            4
#define BULLET_MAX_DISTANCE    150
#define BULLET_HIT_ANGLE       0.5
#define HITSCAN                -1
#define ACTIVE                 1
#define INACTIVE               0

#define RUN_ONCE               0
#define KILL_COMMAND           1
#define STATIC_SPRITE          0
#define MOVING_SPRITE          1
#define SFX_INTERVAL           20

#define PLAYER_ACTOR Game.Actors[Game.ActorsById[Game.player_id]]

#endif
