#ifndef DEF_INP_H
#define DEF_INP_H

/* Defines related to the input system */

#include "Def_bit.h"

#define CONTROL_UP          BIT_0
#define CONTROL_DOWN        BIT_1
#define CONTROL_LEFT        BIT_2
#define CONTROL_RIGHT       BIT_3
#define CONTROL_FAST        BIT_4
#define CONTROL_STRAFE_L    BIT_5
#define CONTROL_STRAFE_R    BIT_6
#define CONTROL_STRAFE_MOD  BIT_7
#define CONTROL_MOUSE_AIM   BIT_8

#define KB_ARRAY_LENGTH     256
#define KB_QUEUE_LENGTH     256

#define KEY_HIT_FLAG        1
#define KEY_PRESSED_FLAG    2
#define KEY_RELEASED_FLAG   0x80 // 128, MSB
#define KEY_SPECIAL_CODE    0xE0 // 224
#define KEY_IS_PRESSED(k)   (g_Input.KeyStates[k] & KEY_IS_DOWN)
#define KEY_WAS_HIT(k)      (g_Input.KeyStates[k] & EV_INP_KEY_HIT)
#define KEY_WAS_RELEASED(k) (g_Input.KeyStates[k] & KEYCODE_RELEASED)
#define KEY_WAS_TOGGLED(k)  (g_Input.KeyStates[k] & (KEY_HIT_FLAG|KEY_RELEASED_FLAG))

#define wrapEnd(x,max)    if ((x)>=(max))   (x)=0
#define wrapStart(x,max)  if ((x)<0)        (x)=(max)-1   
#define incAndWrap(x,max) if (++(x)>=(max)) (x)=0
#define decAndWrap(x,max) if (--(x)<0)      (x)=(max)-1
#define rWrapEnd(x,max)    ((x)>=(max)) ? 0:(x)
#define rWrapStart(x,max)  ((x)<0) ? (max)-1:(x)
#define rIncAndWrap(x,max) ((x)+1>=(max)) ? 0:(x)+1
#define rDecAndWrap(x,max) ((x)-1<0) ? (max)-1:(x)-1

#define SCAN_BUFFER_SIZE    32
#define KEYHANDLER_INT      9

#define EV_INP_TYPE_KEYB    (BIT_0|BIT_1|BIT_2)
#define EV_INP_TYPE_MOUSE   BIT_3
#define EV_INP_TYPE_JOY     BIT_4
#define EV_INP_TYPE_HID     BIT_5
#define EV_INP_KEY_HIT      BIT_0
#define EV_INP_KEY_REL      BIT_1
#define KEY_IS_DOWN         BIT_2
  
#define SCAN_EXTENDED         0xE0
#define SCAN_EXT_PRTSC_START  SCAN_EXTENDED
#define SCAN_EXT_PRTSC_END    0x37
#define SCAN_EXT_PAUSE_START  0xE1
#define SCAN_EXT_PAUSE_END    0xC5
#define SCAN_SEQ_INDEX_PRTSC  0
#define SCAN_SEQ_INDEX_PAUSE  4

#define KEYCODE_EXTENDED    0x80
#define KEYCODE_RELEASED    0x80
 
#define INP_FLAG_CAPS_LOCK          BIT_0
#define INP_FLAG_REPEAT_KEYS        BIT_1
#define INP_FLAG_GLOBAL_KEYS        BIT_2
#define INP_FLAG_UI_CONTROL_KEYS    BIT_3
#define INP_FLAG_WRITE_TEXT         BIT_4
#define INP_FLAG_GAME_CONTROL       BIT_5

#define MOUSE_TURN_THRESHOLD   10.0

#define VOLUME_DOWN         1
#define VOLUME_UP           2

#define SFX_VOLUME          1
#define MIDI_VOLUME         2

#endif /* DEF_INP_H */
