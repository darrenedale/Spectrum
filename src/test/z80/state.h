//
// Created by darren on 27/02/2021.
//

#ifndef SPECTRUM_TEST_Z80_STATE_H
#define SPECTRUM_TEST_Z80_STATE_H

#include <vector>

#include "memoryblock.h"
#include "../../z80/z80.h"

namespace Test::Z80
{
    using UnsignedByte = ::Z80::UnsignedByte;
    using UnsignedWord = ::Z80::UnsignedWord;

    struct State
    {
        UnsignedWord af;
        UnsignedWord bc;
        UnsignedWord de;
        UnsignedWord hl;
        UnsignedWord afShadow;
        UnsignedWord bcShadow;
        UnsignedWord deShadow;
        UnsignedWord hlShadow;
        UnsignedWord ix;
        UnsignedWord iy;
        UnsignedWord sp;
        UnsignedWord pc;
        UnsignedWord memptr;

        UnsignedByte i;
        UnsignedByte r;
        bool iff1;
        bool iff2;
        std::uint8_t im;
        bool halted;

        std::vector<MemoryBlock> memory;
    };
}

#endif //SPECTRUM_TEST_Z80_STATE_H
