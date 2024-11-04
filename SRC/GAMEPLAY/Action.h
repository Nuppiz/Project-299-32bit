#ifndef ACTION_H
#define ACTION_H

#include "SRC/GENERAL/Common.h"
#include "Str_gplay.h"

int checkForHit(Vec2 projectile, Vec2 target, int radius);
void shootWeapon(int weapon_id, Actor_t* source);
void useTile(Vec2 pos, Vec2 dir);
void handleProjectiles();
void actorDeathLoop();

#endif /* ACTION_H */
