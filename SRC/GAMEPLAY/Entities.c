#include "SRC/FILEAXS/Filech.h"
#include "SRC/FILEAXS/LvlLoad.h"
#include "SRC/FILEAXS/Saves.h"
#include "SRC/FILEAXS/Def_lev.h"
#include "SRC/FILEAXS/Str_map.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/Vectors.h"
//#include "SRC/SOUND/Sound.h"
#include "SRC/SYS/Str_sys.h"

#include "Game.h"
#include "Movecoll.h"
#include "Def_ent.h"
#include "Def_game.h"
#include "Str_gplay.h"

/* Entity system */

extern System_t System;
extern Timer_t Timers;
extern Entity_t Entities[];
extern Item_t* Items;
extern GameData_t Game;

uint8_t key_acquired = 0; // replace later with proper inventory system

void checkForItem() // might be replaced with better system later
{
    int tilemap_loc, i, a;

    for (i = 0; i < Game.actor_count; i++)
    {
        Actor_t* actor = &Game.Actors[i];
        tilemap_loc = actor->grid_loc.y * Game.Map.width + actor->grid_loc.x;
        for (a = 0; a < Game.item_count; a++)
        {
            Item_t* item = &Items[a];
            if (tilemap_loc == item->index && item->state == 1)
            {
                if (item->type == ITEM_KEY_RED && actor->id == Game.player_id)
                {
                    key_acquired = TRUE;
                    item->state = 0;
                    //playSFX(SOUND_ITEM);
                }
            }
        }
    }
}

void checkForInteractive() // temporary, will be replaced with better system later
{
    int i;
    static time_type last_env_damage = 0;
    Actor_t* actor;
    Tile_t* tile;

    for (i = 0; i < Game.actor_count; i++)
    {
        actor = &Game.Actors[i];
        tile = getTileAt(actor->grid_loc);

        if (tile->is_entity == 0)
        {
            if (tile->entity_value == TILE_DMG_10)
            {
                if (last_env_damage + HURT_INTERVAL < System.ticks)
                {
                    last_env_damage = System.ticks;

                    /*if (actor->id == Game.player_id)
                        playSFX(SOUND_HURT);
                    else
                        playSFX(SOUND_HURT_E);*/

                    actor->health -= 10;

                    #if DEBUG == 1
                    if (System.debug_mode == TRUE)
                        sprintf(debug[DEBUG_ENTITIES], "TARGET HP: %d", actor->health);
                    #endif
                }
            }
        }
    }
}

Tile_t* getTileAtXY(int x, int y)
{
    Tile_t* tile_location = &Game.Map.tilemap[y * Game.Map.width + x];

    return tile_location;
}

Tile_t* getEntityTile(Entity_t* entity)
{
    return &Game.Map.tilemap[entity->y * Game.Map.width + entity->x];
}

void deleteEntity(Entity_t* entity)
{
    Tile_t* entity_tile = getEntityTile(entity);
    entity_tile->is_entity = 0;
    entity_tile->entity_value = 0;
    Entities[entity->id].type = ENT_NONE;
}

void runSpawner(Entity_t* entity)
{
    Vec2 direction = getDirVec2(entity->data.spawner.angle);

    if (entity->state == 1)
    {
        struct t_spawner* spawner = &entity->data.spawner;

        if (spawner->last_spawn_time + spawner->spawn_time_interval < System.ticks)
        {
            spawner->last_spawn_time = System.ticks;

            if (spawner->num_actors < spawner->max_actors || spawner->max_actors == -1)
            {
                Actor_t actor = {0};
                
                actor.position.x = entity->x * SQUARE_SIZE + direction.x * (rand() % 50);
                actor.position.y = entity->y * SQUARE_SIZE + direction.y * (rand() % 50);
                actor.angle = spawner->angle;
                /* replace variables from here onwards with template system */
                actor.radius = 7;
                actor.control = 0;
                actor.ai_mode = AI_IDLE;
                actor.ai_timer = 100;
                actor.target_id_primary = Game.player_id;
                actor.health = 100;
                actor.trigger_on_death = -1;
                actor.primary_weapon_id = WEAPON_PISTOL;

                createActor(actor, "SPRITES/DUDE2.7UP");
                spawner->num_actors++;
            }

            if (spawner->num_actors >= spawner->max_actors && spawner->max_actors != -1)
            {
                entity->state = 0;

                if (spawner->only_once == 1)
                    deleteEntity(entity);
            }
        }
    }
}

void runTrigger(Entity_t* entity)
{
    int i;
    struct t_trigger* trigger = &entity->data.trigger;

    if (PLAYER_ACTOR.grid_loc.x == entity->x && PLAYER_ACTOR.grid_loc.y == entity->y && entity->state == 0)
    {

        trigger->last_trigger_time = System.ticks;
        //playSFX(SOUND_DOOR_O);
        entity->state = 1;
        for (i = 0; i < 4; i++)
        {
            if (trigger->target_ids[i] != -1 && Entities[trigger->target_ids[i]].type != ENT_COUNTER)
                Entities[trigger->target_ids[i]].state ^= 1;
            else
                Entities[trigger->target_ids[i]].data.counter.value++;
        }
        if (trigger->only_once == 1)
        {
            deleteEntity(entity);
        }
    }
    if (trigger->last_trigger_time + trigger->trigger_interval < System.ticks && entity->state == 1)
        entity->state = 0;
}

void toggleDoor(Entity_t* door)
{
    Tile_t* tile = getTileAtXY(door->x, door->y);

    door->state ^= 1;
    tile->obstacle ^= 1;
    tile->block_bullets ^= 1;
    tile->texture_id = (door->state) == 1 ? tile->texture_id + 1 : tile->texture_id - 1;
}

void useDoor(Entity_t* door, uint8_t use_mode)
{
    if (use_mode == USE_VIA_BUTTON)
    {
        toggleDoor(door);
    }
    else if (door->data.door.locked == TRUE && use_mode == USE_DIRECTLY & key_acquired == TRUE)
    {
        //playSFX(SOUND_DOOR_O);
        door->data.door.locked ^= 1;
        toggleDoor(door);
    }
    /*else if (door->data.door.locked == TRUE && use_mode == USE_DIRECTLY)
        playSFX(SOUND_LOCKED);*/
    else if (door->data.door.locked == FALSE && door->state == 1)
    {
        //playSFX(SOUND_DOOR_C);
        toggleDoor(door);
    }
    else if (door->data.door.locked == FALSE && door->state == 0)
    {
        //playSFX(SOUND_DOOR_O);
        toggleDoor(door);
    }
}

void toggleButton(Entity_t* button)
{
    Tile_t* tile = getTileAtXY(button->x, button->y);

    button->state ^= 1;
    tile->texture_id = (button->state) == 1 ? tile->texture_id + 1 : tile->texture_id - 1;
    if (Entities[button->data.button.target].type == ENT_DOOR)
        useDoor(&Entities[button->data.button.target], USE_VIA_BUTTON);
    else if (Entities[button->data.button.target].type == ENT_SPAWNER && Entities[button->data.button.target].data.spawner.toggleable == TRUE)
    {
        Entities[button->data.button.target].state ^= 1;
    }
}

void toggleCounter(Entity_t* counter)
{
    Entity_t* target = &Entities[counter->data.counter.target_id];
    switch (target->type)
    {
    case ENT_DOOR: toggleDoor(target); break;
    case ENT_BUTTON: toggleButton(target); break;
    }
}

void runCounter(Entity_t* entity)
{
    struct t_counter* counter = &entity->data.counter;

    if (counter->value == counter->max_value)
    {
        toggleCounter(entity);
        if (counter->only_once == 1)
            entity->type = ENT_NONE;
        else
            counter->value = 0;
    }
}

void usePortal(Entity_t* entity)
{
    uint16_t portal_x, portal_y;
    double portal_angle;
    char levelpath[30] = LEVEL_PATH;
    struct t_portal* portal = &entity->data.portal;

    if (PLAYER_ACTOR.grid_loc.x == entity->x && PLAYER_ACTOR.grid_loc.y == entity->y && entity->state == 1)
    {
        //playSFX(SOUND_PORTAL);
        if (portal->level_name != NULL)
        {
            strcat(levelpath, portal->level_name);
            if (checkFileExists(levelpath) == FALSE)
                return;
            else
            {
                portal_x = portal->x;
                portal_y = portal->y;
                portal_angle = portal->angle;
                levelTransition(Game.current_level_name, portal->level_name);
                PLAYER_ACTOR.velocity.x = 0.0;
                PLAYER_ACTOR.velocity.y = 0.0;
                PLAYER_ACTOR.position.x = portal_x;
                PLAYER_ACTOR.position.y = portal_y;
                PLAYER_ACTOR.angle = portal_angle;
                updateGridLoc(&PLAYER_ACTOR);
                saveGameState("CURRENT/");
            }
        }
        else
        {
            PLAYER_ACTOR.position.x = portal->x;
            PLAYER_ACTOR.position.y = portal->y;
            PLAYER_ACTOR.angle = portal->angle;
        }
    }
}

void deathTrigger(int actor_id)
{
    switch (Entities[Game.Actors[actor_id].trigger_on_death].type)
    {
    case ENT_DOOR: toggleDoor(&Entities[Game.Actors[actor_id].trigger_on_death]); break;
    case ENT_BUTTON: toggleButton(&Entities[Game.Actors[actor_id].trigger_on_death]); break;
    }
}

void useTile(Vec2 pos, Vec2 dir)
{
    Vec2_int target;
    Tile_t* tile;
    uint8_t tile_x, tile_y;
    target.x = pos.x + dir.x * 20;
    target.y = pos.y + dir.y * 20;

    tile_x = target.x / SQUARE_SIZE;
    tile_y = target.y / SQUARE_SIZE;

    if ((tile = &Game.Map.tilemap[tile_y * Game.Map.width + tile_x])->is_entity == TRUE)
    {
        Entity_t* ent = &Entities[tile->entity_value];
        switch (ent->type)
        {
        case ENT_DOOR: useDoor(ent, USE_DIRECTLY);
            break;
        case ENT_BUTTON: //playSFX(SOUND_SWITCH), toggleButton(ent);
            break;
        }
    }
}

void entityLoop()
{
    int i;

    for (i = 0; i < MAX_ENTITIES; i++)
    {
        Entity_t* entity = &Entities[i];
        switch (entity->type)
        {
        case ENT_SPAWNER:
            runSpawner(entity);
            break;
        case ENT_TRIGGER:
            runTrigger(entity);
            break;
        case ENT_COUNTER:
            runCounter(entity);
            break;
        case ENT_PORTAL:
            usePortal(entity);
            break;
        default:
            break;
        }
    }
}
