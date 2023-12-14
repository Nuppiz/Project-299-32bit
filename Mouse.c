#include "Common.h"
#include "Structs.h"
#include "Mouse.h"
#include "Movecoll.h"
#include "Action.h" // for testing

extern GameData_t Game;
extern flags_t player_control;

union REGS in, out;
Vec2 mouse_location;

int detectMouse()
{
    in.x.eax = INIT_MOUSE;
    int386(MOUSE_INT, &in, &out);
    return out.x.eax; 
}

void showMouse()
{
    in.x.eax = SHOW_MOUSE;
    int386(MOUSE_INT, &in, &out); 
}

void hideMouse()
{
    in.x.eax = HIDE_MOUSE;
    int386(MOUSE_INT, &in, &out); 
}

void getMouse(int* x, int* y, int* left, int* right)
{
    in.x.eax = GET_MOUSE_STATUS;
    int386(MOUSE_INT, &in, &out);
    *x = out.x.ecx;
    *y = out.x.edx; 
    *left = out.x.ebx & 0x1;
    *right = out.x.ebx & 0x2;
}

void playerMouseAim()
{
    playerTurnTowards(mouse_location);
}

void handleMouseInput()
{
    int mouse_x, mouse_y, mouse_lb, mouse_rb;

    getMouse(&mouse_x, &mouse_y, &mouse_lb, &mouse_rb);

    if (mouse_lb)
    {
      shootWeapon(PLAYER_ACTOR.primary_weapon_id, &PLAYER_ACTOR);

    }
    else if (mouse_rb)
    {
      playerMouseAim();
    }
    mouse_location.x = (float)mouse_x;
    mouse_location.y = (float)mouse_y;
}
