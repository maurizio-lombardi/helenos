#include <cstdlib>
#include "ppu.hpp"
#include "mappers/mapper3.hpp"

/* Based off of https://wiki.nesdev.com/w/index.php/INES_Mapper_003 */

/* Apply the registers state */
void Mapper3::apply()
{
    if (PRG_size_16k)
    {
    /*
     * mirror the bottom on the top
     * 0x8000 - 0xBFFF ==
     * 0xC000 - 0xFFFF
     */
        map_prg<16>(0,0);
        map_prg<16>(1,0);
    }
    else
    {
        /* no mirroring */
        map_prg<16>(0,0);
        map_prg<16>(1,1);
    }

    /* 8k bankswitched CHR */
    map_chr<8>(0, regs[0] & 0b11);

    /* mirroring is based on the header (soldered) */
    set_mirroring(vertical_mirroring?PPU::VERTICAL:PPU::HORIZONTAL);
}

u8 Mapper3::write(u16 addr, u8 v)
{
    /* check for bus contingency? */

    /* chr bank switching */
    if (addr & 0x8000)
    {
      regs[0] = v;
      apply();
    }
    return v;
}

u8 Mapper3::chr_write(u16 addr, u8 v)
{
    return chr[addr] = v;
}

void *Mapper3::dump(size_t *size)
{
	size_t super_size, total_size;
	void *super_data;
	struct Mapper3::Mapper3State *s;

	super_data = Mapper::dump(&super_size);
	total_size = sizeof(struct Mapper3::Mapper3State) + super_size;

	s = (struct Mapper3::Mapper3State *) malloc(total_size);

	s->regs = regs[0];
	s->vertical_mirroring = vertical_mirroring;
	s->PRG_size_16k = PRG_size_16k;

	memcpy(s->super_data, super_data, super_size);
	free(super_data);

	*size = total_size;
	return s;
}

void Mapper3::restore(void *data)
{
	struct Mapper3::Mapper3State *s;

	s = (struct Mapper3::Mapper3State *) data;

	regs[0] = s->regs;
	vertical_mirroring = s->vertical_mirroring;
	PRG_size_16k = s->PRG_size_16k;

	Mapper::restore(s->super_data);
}

