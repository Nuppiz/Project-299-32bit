#ifndef DPMI_H
#define DPMI_H

int dpmi_unlock_region(void *address, unsigned length);
int dpmi_lock_region(void *address, unsigned length);

#endif/* DPMI_H */
