#include "SRC/FILEAXS/Saves.h"
#include "SRC/FILEAXS/Def_lev.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/Vectors.h"
#include "SRC/GENERAL/Def_math.h"
#include "SRC/GFX/Draw.h"
#include "SRC/GFX/Def_draw.h"
//#include "SRC/SOUND/Sound.h"
#include "SRC/SYS/Video.h"
#include "SRC/SYS/Def_vid.h"
#include "SRC/SYS/Str_sys.h"

#include "SRC/ALLEGRO/A_Sound.h"

#include "Game.h"
#include "Entities.h"
#include "Movecoll.h"
#include "Def_game.h"
#include "Str_wpn.h"

/* Various actions between the player and other actors, including weapon mechanics */

extern System_t System;
extern Timer_t Timers;
extern GameData_t Game;
extern Weapon_t Weapons[];
WeaponEffect_t Effects[NUM_EFFECTS] = {SOUND_EXPLOSION, 50, 50, 5};

Projectile_t Projectiles[MAX_PROJECTILES] = {0};
int projectile_read = 0;
int projectile_write = 0;
int num_projectiles = 0;

extern Sprite_t Rocket;
extern Sprite_t Explosion;
extern Vec2 camera_offset;

int checkForHit(Vec2 projectile_location, Vec2 target, int radius)
{
    int collision_left, collision_right, collision_top, collision_bottom;

    collision_left = target.x - radius;
    collision_right = target.x + radius;
    collision_top = target.y - radius;
    collision_bottom = target.y + radius;

    if (projectile_location.x >= collision_left && projectile_location.x <= collision_right
    && projectile_location.y >= collision_top && projectile_location.y <= collision_bottom)
        return TRUE;
    else
        return FALSE;
}

int actorHitLoop(id_type source_id, Vec2 pos, int damage)
{
    int i;
    for (i = 0; i < Game.actor_count; i++)
    {
        Actor_t* actor = &Game.Actors[i];
        if (actor->id != source_id && checkForHit(pos, actor->position, actor->radius) == TRUE)
        {
            actor->health -= damage;
            if (actor->target_id_secondary == UINT16_MAX)
                actor->target_id_secondary = actor->target_id_primary; // save previous primary target but only if secondary slot is blank
            actor->target_id_primary = source_id; // infighting mechanic
            if (actor->ai_mode != AI_CHASE) // if not yet fighting, fight!
                actor->ai_mode = AI_CHASE;
            if (Timers.last_sfx + SFX_INTERVAL < System.ticks)
            {
                Timers.last_sfx = System.ticks;
                if (actor->id == Game.player_id)
                    AllegroPlaySFX(SOUND_HURT);
                else
                    AllegroPlaySFX(SOUND_HURT_E);
            }
            #if DEBUG == 1
            if (System.debug_mode == TRUE)
            {
                sprintf(debug[DEBUG_SHOOT], "LAST HIT: %d", i);
                sprintf(debug[DEBUG_ENTITIES], "TARGET HP: %d", actor->health);
            }
            #endif
            return TRUE;
        }
    }
    return FALSE;
}

void actorHit(id_type source_id, int damage, Actor_t* actor)
{
    actor->health -= damage;
    if (actor->target_id_secondary == UINT16_MAX)
        actor->target_id_secondary = actor->target_id_primary; // save previous primary target but only if secondary slot is blank
    actor->target_id_primary = source_id; // infighting mechanic
    if (actor->ai_mode != AI_CHASE) // if not yet fighting, fight!
        actor->ai_mode = AI_CHASE;
    if (Timers.last_sfx + SFX_INTERVAL < System.ticks)
    {
        Timers.last_sfx = System.ticks;
        if (actor->id == Game.player_id)
            AllegroPlaySFX(SOUND_HURT);
        else
            AllegroPlaySFX(SOUND_HURT_E);
    }
    #if DEBUG == 1
    if (System.debug_mode == TRUE)
    {
        sprintf(debug[DEBUG_SHOOT], "LAST HIT: %d", actor->id);
        sprintf(debug[DEBUG_ENTITIES], "TARGET HP: %d", actor->health);
    }
    #endif
}

void splashDamage(id_type source_id, Vec2 pos, uint16_t damage, uint16_t radius)
{
    int splashline, i;

    for (i = 0; i < Game.actor_count; i++)
    {
        Actor_t* actor = &Game.Actors[i];
        for (splashline = 0; splashline <= radius; splashline++)
        {
            Vec2 v = getVec2(pos, actor->position); // vector between impact site and actor
            float distance = normalizeAndGetLength(&v);

            if (distance > radius) // if target is too far
                break;
            if (checkForHit(pos, actor->position, actor->radius) == TRUE)
            {
                actorHit(source_id, damage - splashline, actor);
                #if DEBUG == 1
                if (System.debug_mode == TRUE)
                    sprintf(debug[DEBUG_SHOOT], "S: %d", splashline);
                #endif
                break;
            }
            pos.x += (v.x * 10);
            pos.y += (v.y * 10);
            if (getTileAt(getGridLocation(pos))->block_bullets == TRUE)
            {
                break;
            }
        }
    }
}

void hitScan(id_type weapon_id, id_type source_id, Vec2 pos, Vec2 dir, int max_range, int damage)
{
    int bulletpath;
    uint8_t hit_something = FALSE;

    for (bulletpath = 0; bulletpath < max_range; bulletpath += BULLET_STEP)
    {
        #if DEBUG == 1
        if (System.debug_mode == TRUE && boundaryCheck(((int)(pos.x - camera_offset.x)), ((int)(pos.y - camera_offset.y))) == TRUE)
            SET_PIXEL_VGA(((int)(pos.x - camera_offset.x)), ((int)(pos.y - camera_offset.y)), COLOUR_WHITE);
        #endif
        pos.x += dir.x * BULLET_STEP;
        pos.y += dir.y * BULLET_STEP;

        if (getTileAt(getGridLocation(pos))->block_bullets == TRUE)
        {
            if (weapon_id != WEAPON_FIST)
                particleFx(pos, dir, FX_SPARKS);
            hit_something = TRUE;
            break;
        }
        else if (hit_something == FALSE)
        {
            hit_something = actorHitLoop(source_id, pos, damage);
            if (hit_something == TRUE)
                particleFx(pos, dir, FX_BLOOD);            
        }
    }
    if (hit_something == FALSE && weapon_id != WEAPON_FIST)
        particleFx(pos, dir, FX_DIRT);
}

void increaseProjectileRead()
{
    projectile_read++;

    if (projectile_read == MAX_PROJECTILES)
        projectile_read = 0;
}

void increaseProjectileWrite()
{
    projectile_write++;

    if (projectile_write == MAX_PROJECTILES)
        projectile_write = 0;

    if (projectile_write == projectile_read)
        increaseProjectileRead();
}

void decrementProjectileWrite()
{
    projectile_write--;

    if (projectile_write < 0)
        projectile_write = MAX_PROJECTILES - 1;
}

void deleteProjectile(int index)
{
    int last_projectile = (projectile_write == 0) ? MAX_PROJECTILES - 1 : projectile_write - 1;

    if (last_projectile != projectile_read)
        Projectiles[index] = Projectiles[last_projectile];

    decrementProjectileWrite();
}

void createProjectile(id_type weapon_id, id_type source_id, Vec2 pos, double angle, Vec2 dir, int max_range)
{
    Projectile_t* projectile = &Projectiles[projectile_write];

    projectile->source_id = source_id;
    projectile->origin.x = pos.x;
    projectile->origin.y = pos.y;
    projectile->position.x = pos.x;
    projectile->position.y = pos.y;
    projectile->angle = angle;
    projectile->velocity.x = dir.x * Weapons[weapon_id].projectile_speed;
    projectile->velocity.y = dir.y * Weapons[weapon_id].projectile_speed;
    projectile->max_range = max_range * max_range; // max range is squared to save an expensive sqrt operation later
    projectile->damage = Weapons[weapon_id].damage;
    projectile->effect_id = Weapons[weapon_id].effect_id;
    projectile->state = TRUE;
    projectile->sprite_id = spawnTempSprite(KILL_COMMAND, MOVING_SPRITE, projectile->position, projectile->velocity, projectile->angle, &Rocket);

    increaseProjectileWrite();
    num_projectiles++;
}

void projectileImpact(int index)
{
    Projectile_t* projectile = &Projectiles[index];
    AllegroPlaySFX(Effects[projectile->effect_id].sound_id);
    spawnTempSprite(RUN_ONCE, STATIC_SPRITE, projectile->position, projectile->velocity, projectile->angle, &Explosion);
    deleteTempSprite(projectile->sprite_id);
    splashDamage(projectile->source_id, projectile->position, Effects[projectile->effect_id].damage, Effects[projectile->effect_id].radius);
    deleteProjectile(index);
    num_projectiles--;
}

void updateProjectile(Projectile_t* projectile)
{
    projectile->position.x += projectile->velocity.x;
    projectile->position.y += projectile->velocity.y;
    updateTempSprite(projectile->sprite_id, projectile->position, projectile->velocity, projectile->angle);
}

void handleProjectiles()
{
    int i = 0;
    Vec2 distance;

    while (i != projectile_write)
    {
        Projectile_t* projectile = &Projectiles[i];

        updateProjectile(projectile);
        distance.x = projectile->position.x - projectile->origin.x;
        distance.y = projectile->position.y - projectile->origin.y;

        if (actorHitLoop(projectile->source_id, projectile->position, projectile->damage) == TRUE || // hit someone
            getTileAt(getGridLocation(projectile->position))->block_bullets == TRUE || // hit a solid wall
            getVec2LengthSquared(distance) >= projectile->max_range) // ran out of range
        {
            projectileImpact(i);
            continue;
        }
        i++;
        if (i == MAX_PROJECTILES)
        {
            i = 0;
        }
    }
}

void emptyProjectileArray()
{
    _fmemset(Projectiles, 0, sizeof(Projectile_t) * MAX_PROJECTILES);
    projectile_read = 0;
    projectile_write = 0;
}

void shootWeapon(int weapon_id, Actor_t* source)
{
    Vec2 projectile_loc, direction;
    double angle;
    int i;
    Weapon_t* weapon = &Weapons[weapon_id];

    if (source->last_shot + weapon->shot_delay < System.ticks)
    {
        source->last_shot = System.ticks;
        AllegroPlaySFX(weapon->sound_id);
        //particleFx(source->position, source->direction, FX_WATERGUN);

        projectile_loc.x = source->position.x + source->direction.x * (source->radius * 1.5);
        projectile_loc.y = source->position.y + source->direction.y * (source->radius * 1.5);

        for (i = 0; i < weapon->num_projectiles; i++)
        {
            angle = source->angle + ((rand() % weapon->projectile_spread - (weapon->projectile_spread / 2)) * RAD_1);
            direction = getDirVec2(angle);
            if (weapon->projectile_speed == HITSCAN)
                hitScan(weapon->id, source->id, projectile_loc, direction, weapon->range + (rand() % weapon->projectile_spread - (weapon->projectile_spread / 2)), weapon->damage);
            else
                createProjectile(weapon->id, source->id, projectile_loc, angle, direction, weapon->range + (rand() % weapon->projectile_spread - (weapon->projectile_spread / 2)));
        }
    }
}

void actorDeathLoop()
{
    int i;
    Actor_t* actor;

    for (i = 0; i < Game.actor_count; i++)
    {
        actor = &Game.Actors[i];
        if (actor->health <= 0)
        {
            if (actor->id == Game.player_id)
            { 
                AllegroPlaySFX(SOUND_DEATH);
                loadAfterDeath(Game.current_level_name);
            }       
            else if (actor->trigger_on_death != UINT16_MAX)
            {
                AllegroPlaySFX(SOUND_DEATH_E);
                deathTrigger(i);
                spawnCorpse(actor->position, actor->angle, -1);
                deleteActor(actor->id);
            }
            else
            {
                AllegroPlaySFX(SOUND_DEATH_E);
                spawnCorpse(actor->position, actor->angle, -1);
                deleteActor(actor->id);
            }
        }
    }
}
