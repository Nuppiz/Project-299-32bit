#ifndef LVLLOAD_H
#define LVLLOAD_H

#include "SRC/GENERAL/Common.h"

void freeAllEntities();
void levelLoader(char* level_name, uint8_t load_type);
void setEntityTilemap();
void levelTransition(char* prevlevelname, char* newlevelname);

#endif /* LVLLOAD_H */
