#include "SRC/GAMEPLAY/Action.h"
#include "SRC/GAMEPLAY/AI.h"
#include "SRC/GAMEPLAY/Entities.h"
#include "SRC/GAMEPLAY/Movecoll.h"

/* State logic, status etc. update functions */

extern GameData_t Game;

void titleUpdate()
{
    // do nothing atm
}

void menuUpdate()
{
    // do nothing atm
}

void gameUpdate()
{
    //AILoop();
    physics();
    checkForItem();
    checkForInteractive();
    handleProjectiles();
    actorDeathLoop();
    entityLoop();
}

void pauseUpdate()
{
    // do nothing atm
}

void ingameMenuUpdate()
{
    // do nothing atm
}
