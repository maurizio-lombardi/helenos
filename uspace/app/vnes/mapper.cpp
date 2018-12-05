#include <cstdlib>
#include "ppu.hpp"
#include "mapper.hpp"


Mapper::Mapper(u8* rom) : rom(rom)
{
    // Read infos from header:
    prgSize      = rom[4] * 0x4000;
    chrSize      = rom[5] * 0x2000;
    prgRamSize   = rom[8] ? rom[8] * 0x2000 : 0x2000;
    set_mirroring((rom[6] & 1) ? PPU::VERTICAL : PPU::HORIZONTAL);

    this->prg    = rom + 16;
    this->prgRam = new u8[prgRamSize];

    // CHR ROM:
    if (chrSize)
        this->chr = rom + 16 + prgSize;
    // CHR RAM:
    else
    {
        chrRam = true;
        chrSize = 0x2000;
        this->chr = new u8[chrSize];
    }
}

Mapper::~Mapper()
{
    delete rom;
    delete prgRam;
    if (chrRam)
        delete chr;
}

/* Access to memory */
u8 Mapper::read(u16 addr)
{
    if (addr >= 0x8000)
        return prg[prgMap[(addr - 0x8000) / 0x2000] + ((addr - 0x8000) % 0x2000)];
    else
        return prgRam[addr - 0x6000];
}

u8 Mapper::chr_read(u16 addr)
{
    return chr[chrMap[addr / 0x400] + (addr % 0x400)];
}

void *Mapper::dump(size_t *size)
{
	struct Mapper::MapperState *s;
	size_t total_size = sizeof(Mapper::MapperState) + prgRamSize + chrSize;

	s = (struct Mapper::MapperState *) malloc(total_size);
	memcpy(s->prgMap, prgMap, sizeof(u32) * 4);
	memcpy(s->chrMap, chrMap, sizeof(u32) * 8);
	s->prgRamSize = prgRamSize;
	s->chrSize = chrSize;
	s->prgSize = prgSize;
	s->chrRam = chrRam;

	memcpy(s->dynamic_data, prgRam, prgRamSize);
	memcpy(&s->dynamic_data[prgRamSize], chr, chrSize);

	*size = total_size;

	return s;
}

void Mapper::restore(void *data)
{
	struct Mapper::MapperState *s;

	delete prgRam;
	if (chrRam)
		delete chr;

	s = (struct Mapper::MapperState *) data;
	chrRam = s->chrRam;
	chrSize = s->chrSize;
	prgSize = s->prgSize;

	if (chrRam)
		chr = new u8[chrSize];
	else
		chr = rom + 16 + prgSize;

	memcpy(prgMap, s->prgMap, sizeof(u32) * 4);
	memcpy(chrMap, s->chrMap, sizeof(u32) * 8);
	prgRamSize = s->prgRamSize;

	prgRam = new u8[prgRamSize];

	memcpy(prgRam, s->dynamic_data, prgRamSize);
	memcpy(chr, &s->dynamic_data[prgRamSize], chrSize);
}

/* PRG mapping functions */
template <int pageKBs> void Mapper::map_prg(int slot, int bank)
{
    if (bank < 0)
        bank = (prgSize / (0x400*pageKBs)) + bank;

    for (int i = 0; i < (pageKBs/8); i++)
        prgMap[(pageKBs/8) * slot + i] = (pageKBs*0x400*bank + 0x2000*i) % prgSize;
}
template void Mapper::map_prg<32>(int, int);
template void Mapper::map_prg<16>(int, int);
template void Mapper::map_prg<8> (int, int);

/* CHR mapping functions */
template <int pageKBs> void Mapper::map_chr(int slot, int bank)
{
    for (int i = 0; i < pageKBs; i++)
        chrMap[pageKBs*slot + i] = (pageKBs*0x400*bank + 0x400*i) % chrSize;
}
template void Mapper::map_chr<8>(int, int);
template void Mapper::map_chr<4>(int, int);
template void Mapper::map_chr<2>(int, int);
template void Mapper::map_chr<1>(int, int);
