#pragma once
#include "mapper.hpp"


class Mapper2 : public Mapper
{
    u8 regs[1];
    bool vertical_mirroring;

    void apply();

  public:
    Mapper2(u8* rom) : Mapper(rom)
    {
        regs[0] = 0;
        vertical_mirroring = rom[6] & 0x01;
        apply();
    }

    u8 write(u16 addr, u8 v);
    u8 chr_write(u16 addr, u8 v);

    struct Mapper2State {
	u8 regs;
	u8 vertical_mirroring;
	u8 super_data[0];
    };

    void *dump(size_t *size);
    void restore(void *data);
};
