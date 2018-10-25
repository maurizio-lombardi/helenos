#include <csignal>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "Sound_Queue.h"
#include "apu.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "menu.hpp"
#include "gui.hpp"
#include "config.hpp"

namespace GUI {

// SDL structures:
u8 const* keys;
Sound_Queue* soundQueue;

// Menus:
bool pause = true;

/* Initialize GUI */
void init()
{
    // Initialize graphics system:
    APU::init();
    soundQueue = new Sound_Queue;
    soundQueue->init(96000);
}

/* Get the joypad state from SDL */
u8 get_joypad_state(int n)
{
	int j = 0;
	return j;
}

/* Send the rendered frame to the GUI */
void new_frame(u32* pixels)
{
}

void new_samples(const blip_sample_t* samples, size_t count)
{
    soundQueue->write(samples, count);
}

