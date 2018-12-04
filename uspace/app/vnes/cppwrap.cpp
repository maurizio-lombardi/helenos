#include <cpu.hpp>
#include <ppu.hpp>
#include <Sound_Queue.h>
#include <cartridge.hpp>
#include <apu.hpp>
#include "cppwrap.h"

static Sound_Queue *soundQueue;

void sound_init(void)
{
	APU::init();
	soundQueue = new Sound_Queue;
	soundQueue->init(44100);
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

void *cartridge_dump(size_t *size)
{
	return Cartridge::dump(size);
}

void *cpu_dump(size_t *size)
{
	return CPU::dump(size);
}

void *ppu_dump(size_t *size)
{
	return PPU::dump(size);
}

