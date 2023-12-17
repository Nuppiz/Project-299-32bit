#ifndef KEYB_H
#define KEYB_H

#include "Common.h"
#include "Keys.h"

extern Input_t g_Input;

void handleInputEvents();
void initInput();
void quitInput();

#endif/* KEYB_H */
