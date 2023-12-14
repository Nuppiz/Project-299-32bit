#ifndef KEYB_H
#define KEYB_H

#include "Common.h"
#include "Keys.h"

extern uint8_t* g_Keyboard;

void handleInputEvents();
void initInput();
void quitInput();

#endif/* KEYB_H */
