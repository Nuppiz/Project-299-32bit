#include "SRC/Allegro/Allegro.h"
#include "SRC/GENERAL/General.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/General.h"
//#include "SRC/SOUND/MIDAS/midasdll.h"
#include "Init.h"
#include "Input.h"
#include "Keyb.h"
#include "State.h"
#include "Video.h"

#include "Def_ste.h"
#include "Def_vid.h"
#include "Str_sys.h"

System_t System = {0};
Timer_t Timers = {0};
extern State_t* Stack[NUM_STATES];
extern int state_count;
extern int stack_top;

void quit()
{
    //deinitClock();
    quitInput();
    allegro_exit();
    setVideoMode(TEXT_MODE);
    printf("Thanks for playing!\n");
}

void quitSetup()
{
    // exit from Allegro setup
    allegro_exit();
    setVideoMode(TEXT_MODE);
    printf("Setup complete, run the game again to play!\n");
}

void updateStats()
{
    #if DEBUG == 1
    if (System.debug_mode == TRUE)
        sprintf(debug[DEBUG_FPS], "TIME: %ld MINS, %ld SECS\nTICKS: %ld, FRAMES: %ld\nFPS: %d, AVERAGE: %.2f",
            System.seconds/60, System.seconds%60, System.ticks, System.frames, System.fps, System.fps_avg);
    #endif
}

void updateStates()
{
    int i;
    State_t* state;
    
    for (i = 0; i < state_count; i++)
    {
        state = Stack[i];
        if ((state->flags & STATE_ENABLE_UPDATE))
            state->update();
    }
}

void handleInput()
{
    // only handle input from the state at the top of the stack
    handleInputEvents();
    Stack[stack_top]->input();
    clearKeys();
}

void processTick()
{
    handleInput();
    updateStates();

    if (System.paused == FALSE)
    {
        System.ticks++;
        System.ticks_per_frame++;
    }
}

void handleTicks()
{
    if (Timers.last_tick + System.tick_interval < System.time)
    {
        do
        {
            Timers.last_tick = System.time;
            Timers.accumulator -= System.tick_interval;

            processTick();
        }
        while (Timers.accumulator >= System.tick_interval);
    }
}

void drawStates()
{
    int i;
    State_t* state;

    for (i = 0; i < state_count; i++)
    {
        state = Stack[i];
        if ((state->flags & STATE_ENABLE_DRAW))
            state->draw();
    }
}

void renderFrame()
{
    drawStates();
    if (System.screen_height == SCREEN_HEIGHT_13H)
        render13h();
    else
        renderModeX();

    System.frames++;
    Timers.frame_count++;
}

void handleFrame()
{
    if (Timers.last_frame + System.frame_interval < System.time)
    {
        Timers.last_frame = System.time;
        Timers.accumulator += System.time - Timers.last_frame;

        renderFrame();
        
        System.ticks_per_frame = 0;

        #if DEBUG == 1
        if (System.debug_mode == TRUE)
            updateStats();
        #endif
    }
}

void fpsCount()
{
    if (Timers.last_time + 1000 < System.time) // FPS calculation; optional for debugging
    {
        Timers.last_time += 1000;
        System.seconds++;
        System.fps_avg = (float)System.frames/System.seconds;
        System.fps = Timers.frame_count;
        Timers.frame_count = 0;
    }
}

void loop()
{
    while (System.running == 1)
    {
        handleTicks();
        handleFrame();

        #if DEBUG == 1
        if (System.debug_mode == TRUE)
            fpsCount();
        #endif
    }
}

void main()
{
    // throw out immediately if no VGA adapter is detected
    if ((checkForVGA() == FALSE))
    {
        printf("No VGA adapter detected!\n"
            "This game requires a VGA adapter.\n");
    }
    else
    {
        mainInit();
        pushState(STATE_TITLE);
        loop();
        if (System.running == 0)
            quit();
        else if (System.running == 2)
            quitSetup();
    }
}
