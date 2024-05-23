#include "Common.h"
#include "Structs.h"
#include "MIDAS/MIDASDLL.H"
#include "MIDAS/MIDAS.H"
#include "MIDAS/mconfig.h"
#include "Exit.h"
#include "General.h"

struct SFX_file SFX_filenames[NUM_SFX];
MIDASsample* SFX_array;
uint8_t music_on = FALSE;
uint8_t SFX_on = TRUE;
extern System_t System;
id_type num_sfx = 0;

/*
 * Simplified MIDAS Sound System API
 *
 * $Id: midas.c,v 1.14 1997/03/05 19:35:03 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of the MIDAS Sound System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Sound System license, LICENSE.TXT. By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/

//#define NOTIMER

#define puts(x)

/* number of sound effect channels: */
#define FXCHANNELS 8

/* maximum number of channels in music: */
#define MAXMUSCHANNELS 8

#define NUMSTREAMCHANNELS 2
#define NUMSAMPLECHANNELS 4
#define NUMMUSICCHANNELS 16

/* sound effect playing rate: */
#define FXRATE 11025

/****************************************************************************\
*      Global variables:
\****************************************************************************/

unsigned    defAmplify;             /* default amplification */
unsigned    amplification;          /* current, amplification */
unsigned    musicVolume = 64;       /* music master volume */
unsigned    SFXVolume = 64;         /* SFX master volume */
int         error;

SoundDevice     *midasSD;               /* current Sound Device */
MIDASmodule module;

#ifdef __WIN32__
#ifdef NODSOUND
SoundDevice     *midasSoundDevices[NUMSDEVICES] =
    { &WinWave,
      &MixNoSound };
#else
SoundDevice     *midasSoundDevices[NUMSDEVICES] =
    { &DSound,
      &WinWave,
      &MixNoSound };
#endif
#else
#ifdef __LINUX__
SoundDevice     *midasSoundDevices[NUMSDEVICES] =
    { &OSS };
#else
SoundDevice     *midasSoundDevices[NUMSDEVICES] =
    {
      &GUS,
      &mGusMix,
      &PAS,
      &WSS,
      &SB,
      &NoSound };
#endif
#endif

gmpPlayHandle   midasPlayHandle;        /* Generic Module Player playing
                                           handle */

int             midasDisableEMS;        /* 1 if EMS usage is disabled
                                           (default 0) */
int             midasSDNumber;          /* Sound Device number (-1 for
                                           autodetect, default -1) */
int             midasSDPort;            /* Sound Device I/O port number
                                           (-1 for autodetect or SD default,
                                           default -1) */
int             midasSDIRQ;             /* Sound Device IRQ number (-1 for
                                           autodetect or SD default,
                                           default -1) */
int             midasSDDMA;             /* Sound Device DMA channel number
                                           (-1 for autodetect or SD default,
                                           default -1) */
int             midasSDCard;            /* Sound Device sound card type
                                           (-1 for autodetect or SD default,
                                           default -1) */
unsigned        midasMixRate;           /* Sound Device mixing rate */
unsigned        midasOutputMode;        /* Sound Device output mode force
                                           bits, default 0 (SD default) */
int             midasAmplification;     /* Forced amplification level or -1
                                           for SD default (default -1) */
int             midasChannels;          /* number of channels open or 0 if no
                                           channels have been opened using
                                           midasOpenChannels() */
int             midasPlayerNum;         /* timer music player number */

int             midasEMSInit;           /* is EMS heap manager initialized? */
int             midasTMRInit;           /* is TempoTimer initialized? */
int             midasTMRPlay;           /* is sound being played with timer?*/
int             midasSDInit;            /* is Sound Device initialized? */
int             midasSDChans;           /* are Sound Device channels open? */
int             midasGMPInit;           /* is GMP initialized? */
int             midasGMPPlay;           /* is GMP playing? */
int             midasTMRMusic;          /* is music being player with timer?*/

void (*midasErrorExit)(char *msg);      /* error exit function */


    /* Channel numbers used with gmpPlaySong(): */
static unsigned midasSDChannels[32] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31 };

static char     errmsg[90];
static int      dResult;

void loadBaseSFX()
{
    // loads a premade list of SFX files into memory
    int i;

    for (i = 0; i < num_sfx; i++)
    {
        SFX_array[i] = MIDASloadRawSample(SFX_filenames[i].filename, MIDAS_SAMPLE_8BIT_MONO, SFX_filenames[i].looping);
    }
}

id_type loadSFXFromFile(char* filename)
{
    // loads an individual SFX file into memory and returns its ID in the array
    id_type sfx_id = num_sfx;

    SFX_array = realloc(SFX_array, (num_sfx + 1) * sizeof(MIDASsample));
    SFX_array[sfx_id] = MIDASloadRawSample(filename, MIDAS_SAMPLE_8BIT_MONO, 0);
    num_sfx++;

    return sfx_id;
}

// effect_id comes from the enum table SoundEffects
void playSFX(int effect_id)
{
    if (SFX_on == TRUE)
        MIDASplaySample(SFX_array[effect_id], MIDAS_CHANNEL_AUTO, 0, FXRATE, SFXVolume, panMiddle);
}

void changeSFXVolume(int modifier)
{
    if (modifier == VOLUME_UP && SFXVolume < 64)
        SFXVolume += 4;
    else if (modifier == VOLUME_DOWN && SFXVolume > 0)
        SFXVolume -= 4;
}

void stopSFX()
{
    // deallocate memory for sound effects
    int i;
    for (i = 0; i < num_sfx; i++)
    {
        MIDASfreeSample(SFX_array[i]);
    }
}

void stopMusic()
{
    music_on = FALSE;
    MIDASstopModule(module);
}

void playMusic(char *music)
{
    music_on = TRUE;
    /* Load module and start playing: */
    module = MIDASloadModule(music);
    MIDASplayModule(module, 0);
}

void changeMusicVolume(int modifier)
{
    if (modifier == VOLUME_UP)
    {
        if (musicVolume < 64 )
        {
            musicVolume += 4;
            if ( (error = midasSD->SetMasterVolume(musicVolume))
                != OK )
                midasError(error);
        }
    }

    else if (modifier == VOLUME_DOWN)
    {
        if (musicVolume > 0 )
            {
                musicVolume -= 4;
                if ( (error = midasSD->SetMasterVolume(musicVolume))
                    != OK )
                    midasError(error);
            }
    }
}

void generateSFXFileTable()
{
    FILE* SFX_list_file;
    int i = 0;
    int looping;
    char filename[20];

    SFX_list_file = fopen("SFX/SFXLIST.txt", "r");

    if (SFX_list_file == NULL)
    {
        fclose(SFX_list_file);
        quitError("Unable to open the basic SFX list file!\n"
                  "Please check the file actually exists!\n");
        return;
    }

    do
    {
        fscanf(SFX_list_file, "%s %d", filename, &looping);
        SFX_filenames[i].filename = malloc(strlen(filename) + 1);
        strcpy(SFX_filenames[i].filename, filename);
        SFX_filenames[i].looping = looping;
        i++;
        num_sfx++;
    } while (fgetc(SFX_list_file) != EOF);
    
    fclose(SFX_list_file);
}

void initSounds()
{
    MIDASstartup();

    midasSetDefaults();                 /* set MIDAS defaults */

    MIDASinit();

    /* Open all channels: */
    if ( !MIDASopenChannels(NUMSTREAMCHANNELS + NUMSAMPLECHANNELS +
        NUMMUSICCHANNELS) )
        midasError(error);

    /* The first NUMSTREAMCHANNELS channels are used for streams, the next
       NUMSAMPLECHANNELS for samples and the rest for music */

    /* Set automatic sample channel range: */
    if ( !MIDASsetAutoEffectChannels(NUMSTREAMCHANNELS, NUMSAMPLECHANNELS) )
        midasError(error);

    /* Get Sound Device default amplification value: */
    if ( (error = midasSD->GetAmplification(&defAmplify)) != OK )
        midasError(error);
    amplification = defAmplify;

    /* Initialize array for sound effect filenames */
    printf("Generating base SFX file name table...\n");
    generateSFXFileTable();
    printf("OK!\n");
    printf("Loading base SFX files into memory...\n");
    SFX_array = malloc(num_sfx * sizeof(MIDASsample));
    loadBaseSFX();
    printf("OK!\n");
    free(SFX_filenames);
}