#include "SRC/FILEAXS/Filech.h"
#include "SRC/FILEAXS/LvlLoad.h"
#include "SRC/FILEAXS/Loadgfx.h"
#include "SRC/GAMEPLAY/Game.h"
#include "SRC/GAMEPLAY/Str_gplay.h"
#include "SRC/GAMEPLAY/Str_wpn.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/General.h"
#include "SRC/GFX/Text.h"
#include "SRC/SOUND/Sound.h"
#include "SRC/SOUND/MIDAS/midasdll.h"

#include "Keyb.h"
#include "Menu.h"
#include "Mouse.h"
#include "Patch.h"
#include "Video.h"
#include "Def_ste.h"
#include "Def_vid.h"
#include "Str_menu.h"
#include "Str_sys.h"

extern System_t System;
extern Timer_t Timers;
extern State_t States[];
extern GameData_t Game;
extern Menu_t* current_menu;
extern Menu_t mainmenu;
extern Menu_t ingamemenu;
Weapon_t Weapons[NUM_WEAPONS];

#if DEBUG == 1
char debug[NUM_DEBUG][DEBUG_STR_LEN];
#endif

/*static void interrupt (far *old_Timer_ISR)(void);
static void interrupt (far *midas_Timer_ISR)(void);

unsigned short setTimerBxHookBx;
unsigned char recomputeMidasTickRate = 0;
unsigned int midasTickRate = 1000;*/

void MIDAS_CALL TimerCallback(void)
{
    System.time++;
}

/*void setTimerBxHook()
{
        // compute the expected tick rate (as 8.8 fix) with given bx
    _asm {
        skipRecompute: popf
        pushf
        cmp setTimerBxHookBx, bx
        jz skipRecompute
        mov setTimerBxHookBx, bx
        inc recomputeMidasTickRate
    }
}

void interrupt far Timer(void)
{
    static long last_clock_time = 0;

    asm pushf;
    asm cli;

    ++System.time;

    if (recomputeMidasTickRate)
    {
        midasTickRate = 1000UL / (1193100UL / setTimerBxHookBx);
        Timers.last_midas_time = System.time;
        recomputeMidasTickRate = 0;
    }

    if (Timers.last_midas_time + midasTickRate < System.time)
    {
        Timers.last_midas_time = System.time;
        midas_Timer_ISR();
    }

    // keeps the PC clock ticking in the background
    if (last_clock_time + 55 < System.time)
    {
        last_clock_time = System.time;
        old_Timer_ISR();
    } 
    else
    {
        outportb(PIC2_COMMAND, PIC_EOI);
        outportb(PIC1_COMMAND, PIC_EOI);
    }

    asm popf;
}

void interrupt far stubISR(void) {

}

void setTimer(uint16_t new_count)
{
    outportb(CONTROL_8253, CONTROL_WORD);
    outportb(COUNTER_0, LOW_BYTE(new_count));
    outportb(COUNTER_0, HIGH_BYTE(new_count));
}*/

#if DEBUG == 1
void initDebug()
{
    int i;
    for (i = 0; i > NUM_DEBUG; i++)
        debug[i][0] = '\0';
}
#endif

void initSystem()
{
    System.running    = 1;
    System.paused     = 0;
    System.time       = 0;
    System.seconds    = 0;
    System.ticks      = 0;
    System.frames     = 0;
    System.tick_interval  = 1000/TICK_RATE;
    System.frame_interval = 1000/FRAME_RATE;
    System.tick_rate  = TICK_RATE;
    System.frame_rate = FRAME_RATE;
    System.fps        = 0;
    System.fps_avg    = 0;
    System.debug_mode = 0;
}

void soundInit()
{
    printf("Initializing sounds...\n");
    initSounds();
	printf("Sound system OK\n");
}

void timerInit()
{
    printf("Initializing timer...\n");
    MIDASsetTimerCallbacks(1000000, FALSE, &TimerCallback, NULL, NULL);
    /*midas_Timer_ISR = _dos_getvect(TIME_KEEPER_INT);
    _dos_setvect(TIME_KEEPER_INT, Timer);
    setTimer(TIMER_1000HZ);
    asm sti;*/
	printf("OK\n");
}

void gfxInit()
{
    extern Palette_t NewPalette;

	printf("Initializing graphics...\n");
    loadFontNew();
    printf("Font loaded\n");
    createErrorTextures();
    loadBaseTextures();
    loadAnimsFromList("ANIMS/ERROR.TXT");
    makeSprites();
    makeAnimset();
    printf("Basic textures loaded into memory\n");
    setVideoMode(VGA_256_COLOR_MODE);
    printf("Video mode OK\n");
    loadPalette("Pal.bmp", &NewPalette);
    printf("Palette loaded\n");
    setPalette_VGA(&NewPalette);
    printf("Palette set\n");
    printf("Graphics init OK\n");
    //setVideoMode(TEXT_MODE);
}

void initWeapons()
{
    FILE* weapon_file;
    int c;
    int i = 0;

    char name[20] = {'\0'};
    int range, damage, num_projectiles, projectile_speed, projectile_spread, shot_delay, ammo_type, sound_id, effect_id;

    weapon_file = fopen("WEAPONS.DAT", "r");

    if (weapon_file == NULL)
    {
        fclose(weapon_file);
        quitError("Unable to open file: WEAPONS.DAT\n"
                  "Please check the file actually exists!\n");
        return;
    }
    
    do
    {
        if (c != '\n')
        {
            fscanf(weapon_file, " %s %d %d %d %d %d %d %d %d %d",
            &name, &range, &damage, &num_projectiles, &projectile_speed, &projectile_spread, &shot_delay, &ammo_type, &sound_id, &effect_id);

            Weapons[i].id = i;
            strcpy(Weapons[i].name, name);
            Weapons[i].range = range;
            Weapons[i].damage = damage;
            Weapons[i].num_projectiles = num_projectiles;
            Weapons[i].projectile_speed = projectile_speed;
            Weapons[i].projectile_spread = projectile_spread;
            Weapons[i].shot_delay = shot_delay;
            Weapons[i].ammo_type = ammo_type;
            Weapons[i].sound_id = sound_id;
            Weapons[i].effect_id = effect_id;
            i++;
        }
    } while ((c = fgetc(weapon_file)) != EOF);

    fclose(weapon_file);
}

void otherInit()
{
    initInput();
    printf("Keyboard OK\n");
    if (detectMouse() == TRUE)
    {
        printf("Mouse detected\n");
    }
    initSystem();
    printf("System init OK\n");
    if (!checkDirectoryExists("SAVES"))
    {
        createDirectory("SAVES");
    }
    initSaveList();
    printf("Save system init OK\n");
    initActorTemplates();
    printf("Actor templates OK\n");
    initWeapons();
    printf("Weapon system OK\n");
    printf("System variables OK\n");
    #if DEBUG == 1
    initDebug();
    printf("Debug OK\n");
    #endif
}

void mainInit()
{
    // sound
    soundInit();
    // timer
    timerInit();
    // misc
    otherInit();
    // gfx
    gfxInit();
    printf("Init complete, launching...\n");
}

void titleInit()
{
    // do nothing atm
}

void menuInit()
{
    current_menu = &mainmenu;
}

void gameInit()
{
    if (!checkDirectoryExists("SAVES/CURRENT"))
    {
        createDirectory("SAVES/CURRENT");
    }
    loadAnimsFromList("ANIMS/ANIMS.TXT");
}

void pauseInit()
{
    System.paused = TRUE;
    States[STATE_INGAME].flags &= ~STATE_ENABLE_UPDATE;
}

void ingameMenuInit()
{
    System.paused = TRUE;
    States[STATE_INGAME].flags &= ~STATE_ENABLE_UPDATE;
    current_menu = &ingamemenu;
}

/*void deinitClock()
{
    asm sti;
    setTimer(TIMER_18HZ);
    _dos_setvect(TIME_KEEPER_INT, old_Timer_ISR);
    asm cli;
}*/
