#ifndef DEF_VID_H
#define DEF_VID_H

/* Video system defines */

#define VGA                 0x000A0000/* VGA memory address. */
#define VIDEO_INT           0x10      /* the BIOS video interrupt. */
#define SET_MODE            0x00      /* BIOS func to set the video mode. */
#define VGA_256_COLOR_MODE  0x13      /* use to set 256-color mode. */
#define TEXT_MODE           0x03      /* use to set 80x25 text mode. */
#define MODE_13H            0x13
#define MODE_X              0XA

#define SCREEN_SIZE_13H     64000     /* screen size in mode 0x13 */
#define SCREEN_SIZE_X       76800     /* screen size in mode X */
#define PLANE_SIZE_X        19200     /* size of one video plane in mode X */

#define MISC_OUTPUT         0x03C2    /* VGA misc. output register */
#define SC_INDEX            0x03C4    /* VGA sequence controller */
#define GC_INDEX            0x03CE
#define PALETTE_WRITE       0x03C8
#define PALETTE_DATA        0x03C9
#define CRTC_INDEX          0x03D4    /* VGA CRT controller */

#define MAP_MASK            0x02      /* Sequence controller registers */
#define ALL_PLANES          0xFF02
#define MEMORY_MODE         0x04

#define H_TOTAL             0x00      /* CRT controller registers */
#define H_DISPLAY_END       0x01
#define H_BLANK_START       0x02
#define H_BLANK_END         0x03
#define H_RETRACE_START     0x04
#define H_RETRACE_END       0x05
#define V_TOTAL             0x06
#define CRTC_OVERFLOW       0x07
#define MAX_SCAN_LINE       0x09
#define V_RETRACE_START     0x10
#define V_RETRACE_END       0x11
#define V_DISPLAY_END       0x12
#define OFFSET              0x13
#define UNDERLINE_LOCATION  0x14
#define V_BLANK_START       0x15
#define V_BLANK_END         0x16
#define MODE_CONTROL        0x17

#endif
