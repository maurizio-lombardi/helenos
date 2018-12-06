#include <cpu.hpp>
#include <ppu.hpp>
#include <Sound_Queue.h>
#include <cartridge.hpp>
#include <apu.hpp>
#include "cppwrap.h"

static Sound_Queue *soundQueue;
static int sound_mute;

void sound_init(int mute)
{
	APU::init();
	sound_mute = mute;

	if (!sound_mute) {
		soundQueue = new Sound_Queue;
		soundQueue->init(44100);
	}
}

void sound_new_samples(const int16_t* samples, size_t count)
{
	if (!sound_mute)
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

void cartridge_restore(void *data)
{
	return Cartridge::restore(data);
}

uint8_t *cartridge_sha1_get(void)
{
	return Cartridge::rom_sha1_get();
}

void *cpu_dump(size_t *size)
{
	return CPU::dump(size);
}

void *ppu_dump(size_t *size)
{
	return PPU::dump(size);
}

void cpu_restore(void *data)
{
	CPU::restore(data);
}

void ppu_restore(void *data)
{
	PPU::restore(data);
}

void *apu_dump(size_t *size)
{
	return APU::dump(size);
}

void apu_restore(void *data)
{
	APU::restore(data);
}
