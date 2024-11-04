#ifndef INPUT_H
#define INPUT_H

#include "SRC/GENERAL/Common.h"
#include "SRC/GFX/Str_text.h"

void processKeyEvents(uint8_t text_input, TextInput_t* destination);
void clearKeys();
void gameInput();
void titleInput();
void menuInput();
void pauseInput();
void ingameMenuInput();

#endif/* INPUT_H */
