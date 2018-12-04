#include <cstdio>
#include <cstdlib>
#include "apu.hpp"
#include "cpu.hpp"
#include "mappers/mapper0.hpp"
#include "mappers/mapper1.hpp"
#include "mappers/mapper2.hpp"
#include "mappers/mapper3.hpp"
#include "mappers/mapper4.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"
#include "cwrap.h"

namespace Cartridge {


Mapper* mapper = nullptr;  // Mapper chip.

/* PRG-ROM access */
template <bool wr> u8 access(u16 addr, u8 v)
{
    if (!wr) return mapper->read(addr);
    else     return mapper->write(addr, v);
}
template u8 access<0>(u16, u8); template u8 access<1>(u16, u8);

/* CHR-ROM/RAM access */
template <bool wr> u8 chr_access(u16 addr, u8 v)
{
    if (!wr) return mapper->chr_read(addr);
    else     return mapper->chr_write(addr, v);
}
template u8 chr_access<0>(u16, u8); template u8 chr_access<1>(u16, u8);

void signal_scanline()
{
    mapper->signal_scanline();
}

/* Load the ROM from a file. */
int load(const char* fileName)
{
    int i;
    long r = 0;
    FILE* f = fopen(fileName, "rb");
    uint8_t *sha1;

    if (!f)
        return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    printf("Image size = %ld bytes\n", size);

    u8* rom = new u8[size];
    if (!rom)
       return -2;
    while (r < size) {
	if (size - r > 128)
           r += fread(&rom[r], 1, 128, f);
	else
           r += fread(&rom[r], 1, size - r, f);
    } 
    fclose(f);

    sha1 = (uint8_t *)malloc(32);
    if (!sha1)
	return -3;

    sha1_chksum(rom, 8192, sha1);
    for (i = 0; i < 20; ++i)
	printf("%x", sha1[i]);
    printf("\n");

    int mapperNum = (rom[7] & 0xF0) | (rom[6] >> 4);
    if (loaded()) delete mapper;
    printf("mapper %d\n", mapperNum);
    switch (mapperNum)
    {
        case 0:  mapper = new Mapper0(rom); break;
        case 1:  mapper = new Mapper1(rom); break;
        case 2:  mapper = new Mapper2(rom); break;
        case 3:  mapper = new Mapper3(rom); break;
        case 4:  mapper = new Mapper4(rom); break;
	default:
            /*FIXME: delete mapper*/
            delete rom;
            return -4;
    }

    CPU::power();
    PPU::reset();
    APU::reset();
    free(sha1);
    return 0;
}

bool loaded()
{
    return mapper != nullptr;
}

void *dump(size_t *size)
{
	return mapper->dump(size);
}

void restore(void *data)
{
	mapper->restore(data);
}


}
