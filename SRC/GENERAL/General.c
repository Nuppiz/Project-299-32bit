#include "SRC/FILEAXS/Str_file.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/SYS/Video.h"
#include "SRC/SYS/Def_vid.h"
#include "SRC/SYS/Str_sys.h"

/* General functions used everywhere */

extern System_t System;

int searchStringArray(char* string, char* string_list[], int num_strings)
{
    int i;
    for (i = 0; i < num_strings; i++)
    {
        if (strcmp(string, string_list[i]) == 0)
            return i;
    }
    
    return RETURN_ERROR;
}

int searchStringList(char* string, StringList_t stringlist)
{
    return searchStringArray(string, stringlist.list, stringlist.count);
}

void freeStringList(StringList_t stringlist)
{
    int i;
    for (i = 0; i < stringlist.count; i++)
        free(stringlist.list[i]);

    free(stringlist.list);
}

void quitError(char* string)
{
    setVideoMode(TEXT_MODE);
    printf("\nFatal error: %s", string);
    System.running = 0;
}
