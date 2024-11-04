#include "SRC/FILEAXS/Filech.h"
#include "SRC/FILEAXS/Lvlload.h"
#include "SRC/FILEAXS/Loadgfx.h"
#include "SRC/GAMEPLAY/Game.h"
#include "SRC/GFX/Draw.h"
#include "SRC/SOUND/Sound.h"

#include "Def_ste.h"
#include "Str_sys.h"

/* State exit functions */

extern State_t States[];
extern System_t System;
extern uint8_t music_on;

void titleExit()
{
    // do nothing atm
}

void menuExit()
{
    
}

void gameExit()
{
    if (music_on == TRUE)
        stopMusic();
    freeAllEntities();
    freeAllTextures();
    emptyCorpseArray();
}

void pauseExit()
{
    System.paused = FALSE;
    States[STATE_INGAME].flags |= STATE_ENABLE_UPDATE;
}

void ingameMenuExit()
{
    System.paused = FALSE;
    States[STATE_INGAME].flags |= STATE_ENABLE_UPDATE;
}
