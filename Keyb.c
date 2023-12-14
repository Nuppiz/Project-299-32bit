#include "Common.h"
#include "Structs.h"
#include "_dpmi.h"

/* Keyboard driver and control inputs */

extern System_t System;
Keyboard_t Keyboard = {0};
Input_t Input = {0};

static volatile KeyScan_t scan_buffer[SCAN_BUFFER_SIZE];
static volatile int scan_head = 0;
static volatile int scan_tail = 0;

static void interrupt (far *old_Keyhandler_ISR)(void);

void pushKeyEvent(KeyEvent_t event)
{
    if ((uint8_t)(Keyboard.queue_tail + 1) != Keyboard.queue_head)
        Keyboard.queue[Keyboard.queue_tail++] = event;
}

void pushInputEvent(InputEvent_t event)
{
    if ((uint8_t)(Input.queue_tail + 1) != Input.queue_head)
        Input.EventQueue[Input.queue_tail++] = event;
}

void handleInputEvents() // keymap should be an array of commands
{
    while (Input.queue_head != Input.queue_tail) 
    {
        InputEvent_t event = Input.EventQueue[Input.queue_head];
        Input.queue_head++;

        /* chain of responsibility */

        /* if ((g_Input.flags & INP_FLAG_GLOBAL_KEYS)
            && handleGlobalKeys(event) == HANDLED)
            continue;

        if ((g_Input.flags & INP_FLAG_WRITE_TEXT)
            && handleTextInput(event) == HANDLED)
            continue;

        // handle UI keys here (menu movement, selection, etc. also in-game inventory and such)
        //if ((g_Input.flags & INP_FLAG_UI_CONTROL)
        //    && handleUIControl(event))
        //    continue;

        if ((g_Input.flags & INP_FLAG_GAME_CONTROL)
            && handleGameControl(event) == HANDLED)
            continue;*/
    }
}

static void handleScanCode(uint8_t scan)
{
    KeyEvent_t event;
    InputEvent_t inputevent;
    static uint8_t status = 0;

    if (scan == KEY_SPECIAL_CODE)
    {
        status = 0x80;
        return;
    }

    event.keycode = (scan & ~KEY_RELEASED_FLAG) | status;
    event.time    = System.ticks;

    inputevent.keycode = scan;
    inputevent.time = System.ticks;

    if (scan & KEY_RELEASED_FLAG)
    {
        // Clear key down flag, set key released flag
        Keyboard.keystates[event.keycode] &= ~KEY_PRESSED_FLAG;
        Keyboard.keystates[event.keycode] |= KEY_RELEASED_FLAG;
        event.type = KEY_RELEASED_FLAG;
        inputevent.state = 0;
        pushKeyEvent(event);
        pushInputEvent(inputevent);
    }
    else if (!(Keyboard.keystates[event.keycode] == KEY_PRESSED_FLAG))
    {
        // Key newly pressed (not a repeat); set key down and key struck flags
        Keyboard.keystates[event.keycode] |= KEY_PRESSED_FLAG|KEY_HIT_FLAG;
        event.type = KEY_HIT_FLAG;
        inputevent.state = 1;
        pushKeyEvent(event);
        pushInputEvent(inputevent);
    }
    status = 0;
}

void interrupt far Keyhandler()
{
    while (inportb(0x64) & 1)
        handleScanCode(inportb(0x60));
    outportb(0x20, 0x20);
}

static void __interrupt KeyHandler_ISR()
{
    uint8_t temp;

    _asm cli;
    
    while ((inp(0x64) & 1))
    {
        uint8_t scan = inp(0x60);
        int scan_tail_next = rIncAndWrap(scan_tail, SCAN_BUFFER_SIZE);

        if (scan_tail_next != scan_head)
        {
            scan_buffer[scan_tail].code = scan;
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

void initKeyboard()
{
    uint8_t far *bios_key_state;

    dpmi_lock_region((void*)KeyHandler_ISR, (char*)KeyHandler_end - (char*)KeyHandler_ISR);
    dpmi_lock_region(&Input, sizeof(Input_t));
    //dpmi_lock_region(&g_Input.EventQueue, sizeof(KB_QUEUE_LENGTH)*sizeof(InputEvent_t));
    //dpmi_lock_region(&g_Input.KeyStates, sizeof(KB_ARRAY_SIZE));

    _asm cli;
    // save address of current keyhandler interrupt function
    old_Keyhandler_ISR = _dos_getvect(KEYHANDLER_INT);
    // caps lock & num lock off
    bios_key_state = MK_FP(0x040, 0x017);
    *bios_key_state &= (~(32|64));
    //OldKeyHandler_ISR(); 
    // replace old keyhandler with new keyhandler function
    _dos_setvect(KEYHANDLER_INT, KeyHandler_ISR);
    _asm sti;
}

void deinitKeyboard()
{
    // restore old keyhandler
    asm cli
    _dos_setvect(KEYHANDLER_INT, old_Keyhandler_ISR);
    asm sti
}
