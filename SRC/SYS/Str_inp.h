#ifndef STR_INP_H
#define STR_INP_H

/* Structs related to the input system*/

#include "SRC/GENERAL/Common.h"
#include "Def_inp.h"

typedef struct
{
    uint8_t keycode;
    uint8_t type;
    time_type time;
} KeyEvent_t;

typedef struct
{
    uint8_t code;
    time_type tick;
} KeyScan_t;

typedef struct
{
    KeyEvent_t queue[KB_QUEUE_LENGTH];
    uint8_t keystates[KB_ARRAY_LENGTH];
    uint8_t queue_head;
    uint8_t queue_tail;
} Keyboard_t;

typedef struct
{
    uint8_t keycode;
    uint8_t state;
    ticks_t time;
    uint8_t type;
} InputEvent_t;

typedef struct
{
    InputEvent_t EventQueue[KB_QUEUE_LENGTH];
    uint8_t KeyStates[KB_ARRAY_LENGTH]; // TO-DO: split into multiple arrays
    uint8_t queue_head;
    uint8_t queue_tail;
} Input_t;

#endif /* STR_INP_H */
