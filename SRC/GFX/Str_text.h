#ifndef STR_TEXT_H
#define STR_TEXT_H

/* Structs related to the text drawing functions */

typedef struct
{
    char* buffer;
    int capacity;
    int length;
    int cursor;
    int start;
} TextInput_t;

#endif /* STR_TEXT_H */
