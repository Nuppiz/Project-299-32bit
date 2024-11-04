#ifndef DEF_VID_H
#define DEF_VID_H

/* Video system defines */

#define VGA                 0x000A0000/* VGA memory address. */
#define VIDEO_INT           0x10      /* the BIOS video interrupt. */
#define SET_MODE            0x00      /* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE  0x13      /* use to set 256-color mode. */
#define TEXT_MODE           0x03      /* use to set 80x25 text mode. */

#define SCREEN_SIZE         64000

#define PALETTE_WRITE       0x03C8
#define PALETTE_DATA        0x03C9

#endif
