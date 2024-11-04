#ifndef DEF_WAT_H
#define DEF_WAT_H

/* Watcom-specific defines to make the code portable between Borland and Watcom */

#ifdef __WATCOMC__
/* __TURBOC__       __WATCOMC__ */  
#define strupr      _strupr   
#define disable     _disable
#define enable      _enable
#define getvect     _dos_getvect
#define setvect     _dos_setvect
#define inportb     inp
#define inport      inpw
#define outportb    outp
#define outport     outpw
#define farfree     _ffree
#define asm         _asm
#define NULL        ((void *)0)
#define M_PI        3.14159265358979323846264338327950288
#endif /* DEF_WAT_H */

#endif/* DEF_WAT_H */
