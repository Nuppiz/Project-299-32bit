#ifndef VIDEO_H
#define VIDEO_H

#include "Str_vid.h"

extern uint8_t* screen;
extern uint8_t far* screen_buf;

int checkForVGA();
void setVideoMode(uint8_t mode);
void loadPalette(char* filename, Palette_t* pal);
void setPalette_VGA(Palette_t* pal);
void render();
void renderWithoutClear();

#endif /* VIDEO_H */
