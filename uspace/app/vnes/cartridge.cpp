#include <cstdio>
#include "apu.hpp"
#include "cpu.hpp"
#include "mappers/mapper0.hpp"
#include "mappers/mapper1.hpp"
#include "mappers/mapper2.hpp"
#include "mappers/mapper3.hpp"
#include "mappers/mapper4.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"

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
    long r = 0;
    FILE* f = fopen(fileName, "rb");

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

    int mapperNum = (rom[7] & 0xF0) | (rom[6] >> 4);
    /* FIXME: if (loaded()) delete mapper; */
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
            return -3;
    }

    CPU::power();
    PPU::reset();
    //APU::reset();
    return 0;
}

bool loaded()
{
    return mapper != nullptr;
}


}
