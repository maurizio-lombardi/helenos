#ifndef _JOYPAD_H_
#define _JOYPAD_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void joypad_init(void);
unsigned char joypad_read_state(int n);
void joypad_write_strobe(int v);
unsigned char joypad_state_get(int n);
void joypad_keyboard_event(wchar_t c, int press);

#ifdef __cplusplus
}
#endif

#endif

