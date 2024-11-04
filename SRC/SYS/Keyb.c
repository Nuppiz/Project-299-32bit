#include <dos.h>
#include <conio.h>
#include "SRC/GENERAL/Common.h"
#include "Dpmi.h"
#include "Keys.h"

#include "Def_inp.h"
#include "Str_inp.h"
#include "Str_sys.h"

static volatile KeyScan_t scan_buffer[SCAN_BUFFER_SIZE];
static volatile int scan_head = 0;
static volatile int scan_tail = 0;

Input_t g_Input = {0};// later should be malloc'd when game begins / player joins etc.
extern System_t System;

static inline void pushInputEvent(InputEvent_t event)
{
    if ((uint8_t)(g_Input.queue_tail+1) != g_Input.queue_head)
        g_Input.EventQueue[g_Input.queue_tail++] = event;
}

static void generateKeyEvent(uint8_t keycode, uint8_t keystate, time_type tick)
{
    static InputEvent_t event;
    event.keycode = keycode;
    event.time = tick;

    if (keystate == EV_INP_KEY_REL)
    {
        // Clear key down flag, set key released flag
        g_Input.KeyStates[event.keycode] &= ~KEY_IS_DOWN;
        g_Input.KeyStates[event.keycode] |= KEYCODE_RELEASED;
        event.type = EV_INP_KEY_REL;
        pushInputEvent(event);
    }
    else if ((g_Input.KeyStates[event.keycode] & KEY_IS_DOWN) == 0)
    {
        // Key newly pressed (not a repeat); set key down and key hit flags
        g_Input.KeyStates[event.keycode] |= (keycode == KEY_PAUSE) ?
            EV_INP_KEY_HIT : KEY_IS_DOWN|EV_INP_KEY_HIT;
        event.type = EV_INP_KEY_HIT;
        pushInputEvent(event);
    }
}

static void handleScanCode(uint8_t scan, time_type tick)
{
    uint8_t keycode, keystate;
    static uint8_t extended = 0;
    static uint8_t special = 0;
    const static uint8_t special_sequence[10] =
    {
        SCAN_EXT_PRTSC_START, 0x2A, 0xE0, SCAN_EXT_PRTSC_END,
        SCAN_EXT_PAUSE_START, 0x1D, 0x45, 0xE1, 0x9D, SCAN_EXT_PAUSE_END
    };

    if (special == 0)
    {
        switch (scan)
        {
        case SCAN_EXTENDED: // == SCAN_EXT_PRTSC_START
            extended = KEYCODE_EXTENDED;
            special = SCAN_SEQ_INDEX_PRTSC+1;
            return;
        case SCAN_EXT_PAUSE_START:
            special = SCAN_SEQ_INDEX_PAUSE+1;
            return;
        }
    }
    else if (scan == special_sequence[special])
    {
        switch (scan)
        {
        case SCAN_EXT_PRTSC_END:
            generateKeyEvent(KEY_PRINT_SCRN, EV_INP_KEY_HIT, tick);
            special = 0;
            return;
        case SCAN_EXT_PRTSC_END|KEYCODE_RELEASED:
            generateKeyEvent(KEY_PRINT_SCRN, EV_INP_KEY_REL, tick);
            special = 0;
            return;
        case SCAN_EXT_PAUSE_END:
            generateKeyEvent(KEY_PAUSE, EV_INP_KEY_HIT, tick);
            special = 0;
            return;
        default:
            special++;  
            return;
        }
    }
    else
        special = 0;

    keycode = (scan & ~KEYCODE_RELEASED) | extended;
    keystate = (scan & KEYCODE_RELEASED) ? EV_INP_KEY_REL : EV_INP_KEY_HIT;
    extended = 0;

    generateKeyEvent(keycode, keystate, tick);
}

void drainScanBuffer()
{
    while (scan_head != scan_tail)
    {
       handleScanCode(scan_buffer[scan_head].code, scan_buffer[scan_head].tick);
       incAndWrap(scan_head, SCAN_BUFFER_SIZE);
    }
}

void handleInputEvents()
{
    drainScanBuffer();
}

static void (__interrupt __far *OldKeyHandler_ISR)();

static void __interrupt KeyHandler_ISR()
{
    //uint8_t temp;

    _asm cli;
    
    while ((inp(0x64) & 1))
    {
        time_type tick = System.ticks;
        uint8_t scan = inp(0x60);
        int scan_tail_next = rIncAndWrap(scan_tail, SCAN_BUFFER_SIZE);

        if (scan_tail_next != scan_head)
        {
            scan_buffer[scan_tail].code = scan;
            scan_buffer[scan_tail].tick = tick;
            scan_tail = scan_tail_next;
        }
    }
    /*
	temp = inp(0x61);	// Get current port 61h state
	temp |= 0x80;		// Turn on bit 7 to signal clear keybrd
	outp(0x61, temp);	// Send to port
	temp &= 0x7f;		// Turn off bit 7 to signal break
	outp(0x61, temp);	// Send to port
    */
    outp(0x20, 0x20);

    _asm sti;
}

static void KeyHandler_end() {};

static void initKeyHandler()
{
    uint8_t far *bios_key_state;

    dpmi_lock_region((void*)KeyHandler_ISR, (char*)KeyHandler_end - (char*)KeyHandler_ISR);
    dpmi_lock_region(&g_Input, sizeof(Input_t));
    //dpmi_lock_region(&g_Input.EventQueue, sizeof(KB_QUEUE_LENGTH)*sizeof(InputEvent_t));
    //dpmi_lock_region(&g_Input.KeyStates, sizeof(KB_ARRAY_SIZE));

    _asm cli;
    // save address of current keyhandler interrupt function
    OldKeyHandler_ISR = _dos_getvect(KEYHANDLER_INT);
    // caps lock & num lock off
    bios_key_state = MK_FP(0x040, 0x017);
    *bios_key_state &= (~(32|64));
    //OldKeyHandler_ISR(); 
    // replace old keyhandler with new keyhandler function
    _dos_setvect(KEYHANDLER_INT, KeyHandler_ISR);
    _asm sti;
}

static void quitKeyHandler()
{
    // restore old keyhandler
    //_asm cli;
    _dos_setvect(KEYHANDLER_INT, OldKeyHandler_ISR);
    //_asm sti;
}

void initInput()
{
    initKeyHandler();
}

void quitInput()
{
    quitKeyHandler();
}
