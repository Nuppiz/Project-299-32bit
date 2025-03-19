#ifndef VIDEO_H
#define VIDEO_H

#include "Str_vid.h"

extern uint8_t* game_screen;
extern uint8_t far* screen_buf;
extern uint8_t far* ModeX_buf[];

int checkForVGA();
void setVideoMode(uint8_t mode);
void initMode13h();
void setModeX();
void loadPalette(char* filename, Palette_t* pal);
void setPalette_VGA(Palette_t* pal);
void render13h();
void renderWithoutClear();
void renderModeX();

#endif /* VIDEO_H */
