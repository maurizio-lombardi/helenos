#pragma once
#include "mapper.hpp"


class Mapper1 : public Mapper
{
    u8 tmpReg;
    int writeN;
    u8 regs[4];

    void apply();

  public:
    Mapper1(u8* rom) : Mapper(rom)
    {
        regs[0] = 0x0C;
        writeN = tmpReg = regs[1] = regs[2] = regs[3] = 0;
        apply();
    }

    u8 write(u16 addr, u8 v);
    u8 chr_write(u16 addr, u8 v);

    struct Mapper1State {
	int32_t writeN;
	u8 regs[4];
	u8 super_data[0]; /* For the superclass data */
    };

    void *dump(size_t *size);
    void restore(void *data);
};
