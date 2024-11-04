#ifndef GAME_H
#define GAME_H

#include "SRC/GENERAL/Common.h"
#include "Str_gplay.h"

id_type getNewId();
void initActorTemplates();
int loadActorTemplate(char* filename);
id_type createActorFromTemplate(Actor_t actor, ActorTemplate_t* template);
id_type createActor(Actor_t actor, char* texture_name);
void deleteActor(id_type id);
void deleteLastActor();
void initGameData(id_type actor_capacity, id_type id_capacity);
void freeGameData();
void testInitPlayerAnim();

#endif/* GAME_H */
