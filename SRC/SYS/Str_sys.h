#ifndef STR_SYS_H
#define STR_SYS_H

/* Structs related to the core system */

#include "SRC/GENERAL/Common.h"

typedef struct
{
    int    running;
    int    paused;      // stop ticks if in menu/paused
    time_type time;        // global timer
    time_type seconds;     // second timer (time/1000)
    ticks_t ticks;       // total game ticks
    time_type frames;      // total frames drawn
    time_type tick_interval;   // target time interval of logic cycles
    time_type frame_interval;  // target time interval between draws
    int    tick_rate;   // target rate of logic cycles
    int    frame_rate;  // target frame rate
    int    fps;         // actual measured fps
    float  fps_avg;     // average fps (frames/seconds)
    int    ticks_per_frame; // number of ticks per frame, varies based on frame rate
    uint8_t debug_mode; // on/off
} System_t;

typedef struct 
{
    time_type last_midas_time; // used for synchronising the MIDAS Sound System
    time_type last_time; // Used for accumulating seconds & FPS calculation
    ticks_t last_tick; // Tracks time elapsed since last tick started
    time_type last_frame; // Tracks time elapsed since last draw started
    time_type accumulator; // Incremented by frame draw duration, decremented by ticks
    int frame_count; // Counts frames in a second so far; used by debug
    ticks_t last_env_damage; // timer for environmental damage
    ticks_t last_sfx; // timer for sound effects
} Timer_t;

typedef struct {
    fnp init;
    fnp input;
    fnp update;
    fnp draw;
    fnp exit;
    flags_t flags;
} State_t;

#endif /* STR_SYS_H */
