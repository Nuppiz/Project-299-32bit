/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      DOS-specific header defines.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#ifndef ALLEGRO_DOS
   #error bad include
#endif

/********************************************/
/************ Platform-specifics ************/
/********************************************/

#define SYSTEM_DOS               AL_ID('D','O','S',' ')

AL_VAR(SYSTEM_DRIVER, system_dos);

AL_VAR(int, i_love_bill);

#define TIMEDRV_FIXED_RATE       AL_ID('F','I','X','T')
#define TIMEDRV_VARIABLE_RATE    AL_ID('V','A','R','T')

AL_VAR(TIMER_DRIVER, timedrv_fixed_rate);
AL_VAR(TIMER_DRIVER, timedrv_variable_rate);

/****************************************/
/************ Sound routines ************/
/****************************************/

#define DIGI_SB10             AL_ID('S','B','1','0')
#define DIGI_SB15             AL_ID('S','B','1','5')
#define DIGI_SB20             AL_ID('S','B','2','0')
#define DIGI_SBPRO            AL_ID('S','B','P',' ')
#define DIGI_SB16             AL_ID('S','B','1','6')
#define DIGI_AUDIODRIVE       AL_ID('E','S','S',' ')
#define DIGI_SOUNDSCAPE       AL_ID('E','S','C',' ')
#define DIGI_WINSOUNDSYS      AL_ID('W','S','S',' ')

#define MIDI_OPL2             AL_ID('O','P','L','2')
#define MIDI_2XOPL2           AL_ID('O','P','L','X')
#define MIDI_OPL3             AL_ID('O','P','L','3')
#define MIDI_SB_OUT           AL_ID('S','B',' ',' ')
#define MIDI_MPU              AL_ID('M','P','U',' ')
#define MIDI_AWE32            AL_ID('A','W','E',' ')


AL_VAR(DIGI_DRIVER, digi_sb10);
AL_VAR(DIGI_DRIVER, digi_sb15);
AL_VAR(DIGI_DRIVER, digi_sb20);
AL_VAR(DIGI_DRIVER, digi_sbpro);
AL_VAR(DIGI_DRIVER, digi_sb16);
AL_VAR(DIGI_DRIVER, digi_audiodrive);
AL_VAR(DIGI_DRIVER, digi_soundscape);
AL_VAR(DIGI_DRIVER, digi_wss);

AL_VAR(MIDI_DRIVER, midi_opl2);
AL_VAR(MIDI_DRIVER, midi_2xopl2);
AL_VAR(MIDI_DRIVER, midi_opl3);
AL_VAR(MIDI_DRIVER, midi_sb_out);
AL_VAR(MIDI_DRIVER, midi_mpu401);
AL_VAR(MIDI_DRIVER, midi_awe32);


#define DIGI_DRIVER_WINSOUNDSYS                                   \
      {  DIGI_WINSOUNDSYS, &digi_wss,           FALSE  },

#define DIGI_DRIVER_AUDIODRIVE                                    \
      {  DIGI_AUDIODRIVE,  &digi_audiodrive,    TRUE   },

#define DIGI_DRIVER_SOUNDSCAPE                                    \
      {  DIGI_SOUNDSCAPE,  &digi_soundscape,    TRUE   },

#define DIGI_DRIVER_SB                                            \
      {  DIGI_SB16,        &digi_sb16,          TRUE   },         \
      {  DIGI_SBPRO,       &digi_sbpro,         TRUE   },         \
      {  DIGI_SB20,        &digi_sb20,          TRUE   },         \
      {  DIGI_SB15,        &digi_sb15,          TRUE   },         \
      {  DIGI_SB10,        &digi_sb10,          TRUE   },

#define MIDI_DRIVER_AWE32                                         \
      {  MIDI_AWE32,       &midi_awe32,         TRUE   },

#define MIDI_DRIVER_ADLIB                                         \
      {  MIDI_OPL3,        &midi_opl3,          TRUE   },         \
      {  MIDI_2XOPL2,      &midi_2xopl2,        TRUE   },         \
      {  MIDI_OPL2,        &midi_opl2,          TRUE   },

#define MIDI_DRIVER_SB_OUT                                        \
      {  MIDI_SB_OUT,      &midi_sb_out,        FALSE  },

#define MIDI_DRIVER_MPU                                           \
      {  MIDI_MPU,         &midi_mpu401,        FALSE  },

#ifdef __cplusplus
extern "C" {
#endif

AL_FUNC(int, load_ibk, (AL_CONST char *filename, int drums));

#ifdef __cplusplus
}
#endif
