#include "SRC/GENERAL/Common.h"
#include "SRC/SOUND/MIDAS/midas.h"
#include "Patch.h"

void patchMidasSetTimer(void far *setTimerBxHook) { 
    // 771 figured out by difference of offsets from .LIB
    void far *timerSetCount = (void far *)((unsigned char *)((void *)&tmrGetScreenRefreshRate) - 771);
    unsigned char far *patch = (void far *)timerSetCount;
    unsigned long ptr = (unsigned long)setTimerBxHook;
    *patch++ = 0x9A; // JMP far
    *patch++ = ptr & 0xFF;
    *patch++ = (ptr >> 8) & 0xFF;
    *patch++ = (ptr >> 16) & 0xFF;
    *patch++ = (ptr >> 24) & 0xFF;
    *patch++ = 0xC3; // RET
}
