#ifndef KEYB_H
#define KEYB_H

#include "SRC/GENERAL/Common.h"
#include "Keys.h"
#include "Str_inp.h"

extern Input_t g_Input;

void handleInputEvents();
void initInput();
void quitInput();

#endif/* KEYB_H */
