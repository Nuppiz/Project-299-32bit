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
 *      Main header file for the entire Allegro library.
 *      (separate modules can be included from the allegro/ directory)
 *
 *      By Shawn Hargreaves.
 *
 *      Vincent Penquerc'h split the original allegro.h into separate headers.
 *
 *      See readme.txt for copyright information.
 */


#ifndef ALLEGRO_H
#define ALLEGRO_H


#include "allegro/base.h"
#include "allegro/gfx.h"
#include "allegro/system.h"
#include "allegro/timer.h"
#include "allegro/file.h"
#include "allegro/datafile.h"
#include "allegro/sound.h"

#include "allegro/gui.h"
#include "allegro/3d.h"
#include "allegro/fixed.h"
#include "allegro/lzss.h"
#include "allegro/palette.h"
#include "allegro/font.h"
#include "allegro/color.h"
#include "allegro/keyboard.h"
#include "allegro/mouse.h"
#include "allegro/joystick.h"
#include "allegro/draw.h"
#include "allegro/text.h"

#include "allegro/platform/aldos.h"

#ifndef ALLEGRO_NO_FIX_CLASS
   #ifdef __cplusplus
      #include "allegro/fix.h"
   #endif
#endif /* ALLEGRO_H */

#endif          /* ifndef ALLEGRO_H */


