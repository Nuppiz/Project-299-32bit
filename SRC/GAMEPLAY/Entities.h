#ifndef ENTITIES_H
#define ENTITIES_H

#include "SRC/FILEAXS/Str_map.h"

void checkForInteractive();
void checkForItem();
Tile_t* getEntityTile(Entity_t* entity);
void deathTrigger(int actor_id);
void entityLoop();

#endif /* ENTITIES_H */
