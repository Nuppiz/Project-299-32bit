#ifndef STR_MENU_H
#define STR_MENU_H

/* Structs related to the menu system */

#include "SRC/GENERAL/Common.h"

typedef struct
{
    char* text;
    fnp action;
} Option_t;

typedef struct
{
    //char* bgfilename;
    int num_selections;
    int cursor_loc;
    int start_y;
    int cursor_y;
    int cursor_x;
    int cursor_spacing;
    Option_t* options;
} Menu_t;

#endif /* STR_MENU_H */
