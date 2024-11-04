#ifndef MOVECOLL_H
#define MOVECOLL_H

#include "SRC/FILEAXS/Str_map.h"
#include "SRC/GENERAL/Common.h"
#include "Str_gplay.h"

Vec2_int getGridLocation(Vec2 pos);
void updateGridLoc(Actor_t* actor);
Tile_t* getTileAt(Vec2_int grid_location);
int getEntityTypeAt(Vec2_int grid_location);
Vec2 forceMove(Vec2 pos);
void turnTowards(Actor_t* actor, Vec2 target);
void playerTurnTowards(Vec2 target);
//void moveAllActors();
//void collideAllActors();
void physics();

#endif/* MOVECOLL_H */
