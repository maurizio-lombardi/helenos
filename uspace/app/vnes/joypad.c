#include <io/keycode.h>
#include "joypad.h"

static unsigned char joypad_bits[2];  // Joypad shift registers.
static int strobe;        // Joypad strobe latch.
static int A, B, SEL, START, UP, DOWN, LEFT, RIGHT;

void joypad_init(void)
{
	A = B = SEL = START = 0;
	UP = DOWN = LEFT = RIGHT = 0;
}

unsigned char joypad_state_get(int n)
{
	unsigned char r = 0;

	if ( n == 1 ) /* Second controller is not supported (yet?) */
		return 0;

	r |= A << 0;
	r |= B << 1;
	r |= SEL << 2;
	r |= START << 3;
	r |= UP << 4;
	r |= DOWN << 5;
	r |= LEFT << 6;
	r |= RIGHT << 7;

	return r;
}

/* Read joypad state (NES register format) */
unsigned char joypad_read_state(int n)
{
    // When strobe is high, it keeps reading A:
    if (strobe)
        return 0x40 | (joypad_state_get(n) & 1);

    // Get the status of a button and shift the register:
    unsigned char j = 0x40 | (joypad_bits[n] & 1);
    joypad_bits[n] = 0x80 | (joypad_bits[n] >> 1);
    return j;
}

void joypad_write_strobe(int v)
{
    // Read the joypad data on strobe's transition 1 -> 0.
    if (strobe && !v)
        for (int i = 0; i < 2; i++)
            joypad_bits[i] = joypad_state_get(i);

    strobe = v;
}

void joypad_keyboard_event(wchar_t c, int press)
{
	switch (c) {
	case 'a':
		A = press;
		break;
	case 's':
		B = press;
		break;
	case 'b':
		SEL = press;
		break;
	case KC_UP:
		UP = press;
		break;
	case KC_DOWN:
		DOWN = press;
		break;
	case KC_LEFT:
		LEFT = press;
		break;
	case KC_RIGHT:
		RIGHT = press;
		break;
	}
}


