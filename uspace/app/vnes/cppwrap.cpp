#include <cpu.hpp>
#include <Sound_Queue.h>
#include <cartridge.hpp>
#include <apu.hpp>
#include "cppwrap.h"

static Sound_Queue *soundQueue;

void sound_init(void)
{
	APU::init();
	soundQueue = new Sound_Queue;
	soundQueue->init(96000);
}

void sound_new_samples(const int16_t* samples, size_t count)
{
	soundQueue->write(samples, count);
}

void cpu_run_frame(void)
{
	CPU::run_frame();
}

int cartridge_load_file(char *filename)
{
	return Cartridge::load(filename);
}

