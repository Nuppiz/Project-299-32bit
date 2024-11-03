#ifndef DEF_TIME_H
#define DEF_TIME_H

#define CONTROL_8253        0x43
#define CONTROL_WORD        0x3C
#define COUNTER_0           0x40
#define TIME_KEEPER_INT     0x08 
#define TIMER_18HZ          0
#define TIMER_1000HZ        1193
#define PIC1_COMMAND        0x20
#define PIC2_COMMAND        0xA0
#define PIC_EOI             0x20

#define LOW_BYTE(n)         (n & 0x00ff)
#define HIGH_BYTE(n)        ((n>>8) & 0x00ff)

#endif
